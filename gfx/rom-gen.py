from PIL import Image

image = Image.open('./promo-image.png')
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

with open('./image.cpp', 'w') as f:
    f.write('#include "image.hpp"\n')
    f.write('const unsigned char IMAGE_DATA[] = {')
    for idx, byte in enumerate(lcd_data):
        if idx % 32 == 0:
            f.write('\n    ')
        f.write(f'0x{byte:02x}, ')
    f.write('\n};\n')
