#pragma once

#include <cstdint>

#include <span>

namespace font::data
{

    using GlyphDataType = uint16_t;

    struct Glyph {
        uint8_t width = 0;
        uint8_t height = 0;
        int8_t offset_x = 0;
        int8_t offset_y = 0;
        int8_t advance = 0;
        uint8_t stride = 0;
        std::span<const GlyphDataType> data = {};
    };

    constexpr Glyph NUL_GLYPH = {};

    struct Font {
        const char* name;
        int8_t ascent;
        int8_t descent;
        uint8_t bpp;
        char glyph_base;
        uint8_t glyph_count;
        std::span<const Glyph> glyphs;

        constexpr const Glyph& get(char ch) const {
            if (ch < glyph_base || ch >= glyph_base + glyph_count)
                return NUL_GLYPH;
            return glyphs[ch - glyph_base];
        }
    };

}
