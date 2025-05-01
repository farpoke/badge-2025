#pragma once

#include <../lcd/lcd.hpp>

namespace drawing
{
    using lcd::WIDTH;
    using lcd::HEIGHT;

    using lcd::Pixel;
    using lcd::to_pixel;
    using lcd::from_grayscale;

    void fill_rect(int left, int right, int top, int bottom, Pixel color);
    void copy(int left, int right, int top, int bottom, const Pixel* pixels);

}
