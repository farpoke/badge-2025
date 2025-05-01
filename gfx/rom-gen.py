import sys
from pathlib import Path
from textwrap import wrap
from PIL import Image

assert len(sys.argv) == 3
_, in_path, out_path = sys.argv

image = Image.open(in_path)
w, h = image.size

lcd_data = bytes()
for y in range(h):
    for x in range(w):
        r, g, b = image.getpixel((x, y))

        # RGB 8-8-8: RRRR Rrrr  GGGG GGgg  BBBB Bbbb
        #            \_    \_  |      |  _/    _/
        #              \     \ /      | /     /
        # RGB 5-6-5:    RRRR R GGG  GGG B BBBB
        #              /         /  \         \
        #       High (first) byte    Low (second) byte

        high_byte = (r & 0xF8) | (g >> 5)
        low_byte = ((g & 0x1C) << 3) | (b >> 3)

        lcd_data += bytes([high_byte, low_byte])

hex_data = ', '.join(f'0x{b:02x}' for b in lcd_data)
hex_lines = wrap(hex_data, 120, initial_indent='    ', subsequent_indent='    ')

cpp_lines = [
    '#include <gfx/image.hpp>',
    '',
    'const unsigned char IMAGE_DATA[] = {',
    *hex_lines,
    '};',
    '',
]

out_path = Path(out_path)
out_path.parent.mkdir(parents=True, exist_ok=True)
out_path.write_text('\n'.join(cpp_lines))
