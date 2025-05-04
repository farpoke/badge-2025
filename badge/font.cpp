#include "font.hpp"

#include <cstring>
#include <memory>

#include <assets.hpp>
#include <badge/drawing.hpp>

namespace font
{
    using namespace drawing;

    constexpr Font lucida(data::lucida);
    constexpr Font m5x7(data::m5x7);
    constexpr Font m6x11(data::m6x11);
    constexpr Font noto_sans(data::noto_sans);
    constexpr Font noto_sans_cm(data::noto_sans_cm);

    TextDraw Font::render(std::string_view text) const {
        if (text.empty())
            return {};

        TextDraw result = {};

        result.dx = data.get(text[0]).offset_x - 1;
        int right = result.dx;
        result.dy = -data.ascent - 1;
        int bottom = data.descent + 1;

        data::Glyph glyph;
        for (char ch : text) {
            glyph = data.get(ch);
            right += glyph.advance;
            result.dy = std::min(result.dy, glyph.offset_y - 1);
            bottom = std::max(bottom, glyph.height + glyph.offset_y + 1);
        }
        right += glyph.width - glyph.advance - glyph.offset_x + 1;

        result.width = right - result.dx + 1;
        result.height = bottom - result.dy + 1;

        result.alpha = std::unique_ptr<uint8_t[]>(new uint8_t[result.width * result.height]);
        memset(result.alpha.get(), 0, result.width * result.height);

        if (data.bpp == 1) {
            constexpr auto pixels_per_value = sizeof(data::GlyphDataType) * 8;
            int x0 = 1;
            int y0 = -result.dy + 1;
            for (char ch : text) {
                glyph = data.get(ch);

                for (int row = 0; row < glyph.height; row++) {
                    auto ptr = &glyph.data[row * glyph.stride];
                    for (int col = 0; col < glyph.width; col++) {
                        const auto value = ptr[col / pixels_per_value];
                        const auto bit = (value >> (col % pixels_per_value)) & 1;
                        if (!bit) continue;
                        const auto px = x0 + glyph.offset_x + col;
                        const auto py = y0 + glyph.offset_y + row;
                        if (px >= 0 && px < result.width && py >= 0 && py < result.height) {
                            result.alpha[py * result.width + px] = 255;
                        }
                    }
                }

                x0 += glyph.advance;
            }
        }
        else {
            const auto n_colors = 1 << data.bpp;
            auto alpha_lut = std::unique_ptr<uint8_t[]>(new uint8_t[n_colors]);
            alpha_lut[0] = 0;
            alpha_lut[n_colors - 1] = 255;
            for (int i = 1; i < n_colors - 1; i++) {
                alpha_lut[i] = i << (8 - data.bpp);
            }

            int x0 = 1;
            int y0 = -result.dy + 1;
            for (char ch : text) {
                glyph = data.get(ch);

                for (int row = 0; row < glyph.height; row++) {
                    auto ptr = &glyph.data[row * glyph.stride];
                    auto data_value = *ptr;
                    auto bits_remaining = sizeof(glyph.data[0]) * 8;
                    for (int col = 0; col < glyph.width; col++) {
                        if (bits_remaining < data.bpp) {
                            data_value = *(++ptr);
                            bits_remaining = sizeof(glyph.data[0]) * 8;
                        }
                        const auto bits = data_value & ((1 << data.bpp) - 1);
                        data_value >>= data.bpp;
                        bits_remaining -= data.bpp;
                        if (bits == 0) continue;
                        const auto px = x0 + glyph.offset_x + col;
                        const auto py = y0 + glyph.offset_y + row;
                        if (px >= 0 && px < result.width && py >= 0 && py < result.height) {
                            result.alpha[py * result.width + px] |= alpha_lut[bits];
                        }
                    }
                }

                x0 += glyph.advance;
            }
        }

        return result;
    }

}
