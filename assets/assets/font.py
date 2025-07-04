from pathlib import Path
from PIL import ImageFont
import re
from textwrap import dedent, fill
import argparse

from .base import AssetBase


CHARSET = ''.join(chr(i) for i in range(32, 127))

GLYPH_DATA_TYPE_BITS = 16


class Glyph:
    ch: str
    offset_x: int
    offset_y: int
    width: int
    height: int
    advance: int
    data_values: bytes

    def __init__(self, font: ImageFont.FreeTypeFont, ch: str, bpp: int):
        self.ch = ch
        mask, (self.offset_x, self.offset_y) = font.getmask2(self.ch, anchor='ls')
        self.width, self.height = mask.size
        self.advance = int(font.getlength(ch))

        self.stride = (self.width * bpp + GLYPH_DATA_TYPE_BITS - 1) // GLYPH_DATA_TYPE_BITS
        self.data = []
        for y in range(self.height):
            value = 0
            bits_remaining = GLYPH_DATA_TYPE_BITS
            bits_offset = 0
            for x in range(self.width):
                if bits_remaining < bpp:
                    self.data.append(value)
                    value = 0
                    bits_remaining = GLYPH_DATA_TYPE_BITS
                    bits_offset = 0
                pixel_byte = mask.getpixel((x, y))
                pixel_bits = round((pixel_byte / 255) * ((1 << bpp) - 1))
                value |= pixel_bits << bits_offset
                bits_remaining -= bpp
                bits_offset += bpp
            self.data.append(value)


def export_ttf(ttf_path: Path, size: int, name: str, bpp: int):
    assert re.fullmatch(r'[a-z][a-z0-9_]+', name, re.IGNORECASE)

    font = ImageFont.FreeTypeFont(ttf_path, size)
    family, style = font.getname()

    ascent, descent = font.getmetrics()
    glyphs = [Glyph(font, ch, bpp) for ch in CHARSET]
    combined_data = []
    offsets = {}
    for glyph in glyphs:
        offsets[glyph] = len(combined_data)
        combined_data += glyph.data

    assert all(ord(g.ch) == ord(glyphs[0].ch) + i for i, g in enumerate(glyphs))

    def glyph_def(glyph_: Glyph) -> str:
        return (
            '{ '
            f'{glyph_.width:2}, '
            f'{glyph_.height:2}, '
            f'{glyph_.offset_x:4}, '
            f'{glyph_.offset_y:4}, '
            f'{glyph_.advance:4}, '
            f'{glyph_.stride:4}, '
            f'{{ &{name}_data[{offsets[glyph_]:4}], {len(glyph_.data):4} }}'
            ' },'
            f'  // "{glyph_.ch}"'
        )

    hpp_content = dedent(f'''\
        namespace font::data
        {{
            /**
             * - Name:  {family}
             * - Style: {style}
             * - File:  {ttf_path.name}
             * - Size:  {size}
             */
            extern const Font {name};
        }}
    ''')

    cpp_content = dedent(f'''\
        namespace font::data
        {{
        
            namespace
            {{
                constexpr GlyphDataType {name}_data[] = {{ {
    '\n' + fill(', '.join(map(lambda v: f'0x{v:x}', combined_data)),
                initial_indent=' '*20,
                subsequent_indent=' '*20)
    } }};
            
                constexpr Glyph {name}_glyphs[] = {{
                    {('\n' + ' ' * 20).join(map(glyph_def, glyphs))}
                }}; 
            }}
            
            constexpr Font {name} = {{
                "{family}",
                {ascent},
                {descent},
                {bpp},
                {ord(glyphs[0].ch)},
                {len(glyphs)},
                {name}_glyphs,
            }};
        }}
    ''')

    return hpp_content, cpp_content


class FontAsset(AssetBase):
    def __init__(self, name: str, *, ttf: str, size: int, bpp: int = 1):
        super().__init__()

        self.name = name
        self.ttf = Path(ttf).resolve()
        self.size = size
        self.bpp = bpp

        self.dependencies.append(self.ttf)

    def get_output(self):
        print(f'- Font asset {self.name}, size={self.size}, bpp={self.bpp} ttf={self.ttf.as_posix()}')
        hpp, cpp = export_ttf(self.ttf, self.size, self.name, self.bpp)
        return hpp.splitlines(), cpp.splitlines()

