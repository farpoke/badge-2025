#include "font.hpp"

#include <cstring>
#include <memory>

#include "../fonts/font_data.hpp"

#include "../fonts/cpp/lucida.hpp"
#include "../fonts/cpp/m5x7.hpp"
#include "../fonts/cpp/m6x11.hpp"
#include "../fonts/cpp/noto_sans.hpp"
#include "../fonts/cpp/noto_sans_cm.hpp"

#include "../board/lcd.hpp"

namespace font
{

    using lcd::Pixel;

    constexpr Font lucida(data::lucida);
    constexpr Font m5x7(data::m5x7);
    constexpr Font m6x11(data::m6x11);
    constexpr Font noto_sans(data::noto_sans);
    constexpr Font noto_sans_cm(data::noto_sans_cm);

    void Font::draw(int x, int y, std::string_view text) const {
        if (text.empty())
            return;

        const int left = x + data.get(text[0]).offset_x - 1;
        int right = left;
        int top = y - data.ascent - 1;
        int bottom = y + data.descent + 1;

        data::Glyph glyph;
        for (char ch : text) {
            glyph = data.get(ch);
            right += glyph.advance;
            top = std::min(top, y + glyph.offset_y - 1);
            bottom = std::max(bottom, y + glyph.height + glyph.offset_y + 1);
        }
        right += glyph.width - glyph.advance - glyph.offset_x + 1;

        const int text_width = right - left + 1;
        const int text_height = bottom - top + 1;

        auto text_pixels = std::unique_ptr<Pixel[]>(new Pixel[text_width * text_height]);
        memset(text_pixels.get(), 0, text_width * text_height * sizeof(Pixel));

        if (data.bpp == 1) {
            constexpr auto pixels_per_value = sizeof(data::GlyphDataType) * 8;
            int x0 = 1;
            int y0 = y - top + 1;
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
                        if (px >= 0 && px < text_width && py >= 0 && py < text_height) {
                            text_pixels[py * text_width + px] = ~0;
                        }
                    }
                }

                x0 += glyph.advance;
            }
        }
        else {
            const auto n_colors = 1 << data.bpp;
            Pixel colors[n_colors];
            colors[0] = 0;
            colors[n_colors - 1] = ~0;
            for (int i = 1; i < n_colors - 1; i++) {
                const auto value = (255 / n_colors) * i;
                colors[i] = lcd::from_grayscale(value);
            }

            int x0 = 1;
            int y0 = y - top + 1;
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
                        if (px >= 0 && px < text_width && py >= 0 && py < text_height) {
                            text_pixels[py * text_width + px] |= colors[bits];
                        }
                    }
                }

                x0 += glyph.advance;
            }
        }

        lcd::copy(left, right, top, bottom, text_pixels.get());
    }

}
