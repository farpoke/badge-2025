#pragma once

#include "pixel.hpp"

namespace image
{
    struct Image {
        int width = 0;
        int height = 0;
        const Pixel* color_data = nullptr;
        const uint8_t* alpha_data = nullptr;
    };

}
