from pathlib import Path
from PIL import Image

from .base import AssetBase


def rgb_to_pixel(r, g, b):
    # RGB 8-8-8: RRRR Rrrr  GGGG GGgg  BBBB Bbbb
    #            \_    \_  |      |  _/    _/
    #              \     \ /      | /     /
    # RGB 5-6-5:    RRRR R GGG  GGG B BBBB
    #              /         /  \         \
    #       High (first) byte    Low (second) byte

    high_byte = (r & 0xF8) | (g >> 5)
    low_byte = ((g & 0x1C) << 3) | (b >> 3)

    assert 0 <= high_byte <= 255
    assert 0 <= low_byte <= 255

    # The ARM core is little-endian.
    return low_byte, high_byte


def extract_data(path: Path):
    image = Image.open(path)
    w, h = image.size
    bands = ''.join(image.getbands())

    pixel_data = bytearray()
    alpha_data = None

    if bands == 'RGB':
        for y in range(h):
            for x in range(w):
                pixel = image.getpixel((x, y))
                pixel_data.extend(rgb_to_pixel(*pixel))

    elif bands == 'RGBA':
        alpha_data = bytearray()
        for y in range(h):
            for x in range(w):
                r, g, b, a = image.getpixel((x, y))
                pixel_data.extend(rgb_to_pixel(r, g, b))
                alpha_data.append(a)

    return w, h, pixel_data, alpha_data


class ImageAsset(AssetBase):
    def __init__(self, name, *, image: str, color: bool = True, alpha: bool = True):
        super().__init__()

        self.name = name
        self.image_path = Path(image).resolve()
        self.color = color
        self.alpha = alpha

        self.dependencies.append(self.image_path)

    def get_output(self):
        path_str = self.image_path.as_posix()
        print(f'- Image asset {self.name}, {path_str}')

        w, h, rgb, alpha = extract_data(self.image_path)
        if self.alpha:
            assert alpha is not None, f'Expected alpha data in image {self.name}, {path_str}'

        header_lines = [
            'namespace image {',
            '    namespace data {',
        ]
        source_lines = [
            'namespace image {',
            '    namespace data {',
        ]

        if self.color:
            rgb_name = self.name + '_RGB'
            header_lines.append(f'        extern const uint8_t {rgb_name}[];')
            source_lines.extend(self.format_data_array(name=rgb_name, data=rgb, indent=8))
            rgb_expr = f'reinterpret_cast<const Pixel*>(data::{rgb_name})'
        else:
            rgb_expr = 'nullptr'

        if self.alpha:
            alpha_name = self.name + '_ALPHA'
            header_lines.append(f'        extern const uint8_t {alpha_name}[];')
            source_lines.extend(self.format_data_array(name=alpha_name, data=alpha, indent=8))
            alpha_expr = 'data::' + alpha_name
        else:
            alpha_expr = 'nullptr'

        header_lines += [
            '    }',
            f'    extern const Image {self.name};',
            '}',
        ]
        source_lines += [
            '    }',
            f'    const Image {self.name} {{ {w}, {h}, {rgb_expr}, {alpha_expr} }};',
            '}',
        ]

        return header_lines, source_lines
