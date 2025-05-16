#pragma once

#include "font.hpp"
#include "image.hpp"
#include "lcd.hpp"
#include "pixel.hpp"

namespace drawing
{
    using lcd::HEIGHT;
    using lcd::WIDTH;

    using font::TextDraw;

    using image::Image;

    void clear(Pixel color);

    void draw_pixel(int x, int y, Pixel color);

    void draw_line(int x0, int y0, int x1, int y1, Pixel color);

    void draw_rect(int left, int top, int width, int height, Pixel color);

    void fill_rect(int left, int top, int width, int height, Pixel color);
    void fill_rect(int left, int top, int width, int height, Pixel color, uint8_t alpha);
    void fill_rect(int left, int top, int width, int height, Pixel color, const uint8_t *alpha);

    void draw_ellipse(int left, int top, int width, int height, Pixel color);
    void fill_ellipse(int left, int top, int width, int height, Pixel color);

    void copy(int left, int top, int width, int height, int stride, const Pixel *pixels);
    void copy_alpha(int left, int top, int width, int height, int stride, const Pixel *pixels, const uint8_t *alpha);

    void draw_image(int dst_left, int dst_top, int src_left, int src_top, int width, int height, const Image &image);

    inline void draw_image(int left, int top, const Image &image) {
        draw_image(left, top, 0, 0, image.width, image.height, image);
    }

    void fill_alpha(int left, int top, Pixel color, const Image &mask);

    void draw_text(int x, int y, Pixel bg, uint8_t bg_alpha, Pixel fg, const TextDraw &render);
    void draw_text(int x, int y, std::string_view text, Pixel fg, const font::Font& font);
    void draw_text_centered(int x, int y, std::string_view text, Pixel fg, const font::Font &font);

} // namespace drawing
