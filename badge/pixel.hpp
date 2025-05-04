#pragma once

#include <cstdint>

using Pixel = uint16_t;

constexpr Pixel rgb24(uint32_t rgb) {
    // 8-8-8 : RRRR Rrrr GGGG GGgg BBBB Bbbb
    // 5-6-5 :           RRRR RGGG GGGB BBBB
    return ((rgb & 0xF80000) >> 8) | ((rgb & 0x00F800) >> 5) | ((rgb & 0x0000F8) >> 3);
}

constexpr Pixel rgb888(uint8_t r, uint8_t g, uint8_t b) { return rgb24(r << 16 | g << 8 | b); }

constexpr Pixel l8(uint8_t gray) { return rgb888(gray, gray, gray); }


constexpr auto COLOR_BLACK = rgb888(0, 0, 0);
constexpr auto COLOR_WHITE = rgb888(255, 255, 255);
