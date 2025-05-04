#pragma once

#include "font.hpp"
#include "image.hpp"
#include "lcd.hpp"

namespace drawing
{
    using lcd::WIDTH;
    using lcd::HEIGHT;

    using lcd::Pixel;
    using lcd::to_pixel;
    using lcd::from_grayscale;

    using image::Image;

    void clear(Pixel color);

    void draw_pixel(int x, int y, Pixel color);

    void draw_line(int x0, int y0, int x1, int y1, Pixel color);
    void draw_line_aa(int x0, int y0, int x1, int y1, Pixel color);

    void draw_rect(int left, int top, int width, int height, Pixel color);
    void fill_rect(int left, int top, int width, int height, Pixel color);

    void draw_circle(int x, int y, int radius, Pixel color);
    void fill_circle(int x, int y, int radius, Pixel color);

    void copy(int left, int top, int width, int height, const Pixel* pixels);
    void copy_masked(int left, int top, int width, int height, const Pixel* pixels, const uint8_t* mask);

    void draw_image(int left, int top, const Image& image);
    void fill_masked(int left, int top, Pixel color, const Image& mask);

}
