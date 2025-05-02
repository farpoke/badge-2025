#pragma once

#include "lcd.hpp"

namespace image
{
    using lcd::Pixel;

    struct Image {
        int width;
        int height;
        const Pixel* color_data;
        const uint8_t* alpha_data;
    };

}
