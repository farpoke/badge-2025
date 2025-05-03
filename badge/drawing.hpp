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

    void fill_rect(int left, int top, int width, int height, Pixel color);
    void copy(int left, int top, int width, int height, const Pixel* pixels);
    void draw_image(int left, int top, const Image& image);
    void fill_masked(int left, int top, Pixel color, const Image& mask);

}
