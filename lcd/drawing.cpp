#include "drawing.hpp"

namespace drawing
{

    template<typename T = uint8_t>
    bool validate_rect(int &left, int &top, int &width, int &height, T **data = nullptr, int stride = 0) {
        if (width < 0) {
            left += width;
            width = -width;
        }
        if (height < 0) {
            top += height;
            height = -height;
        }
        if (left < 0) {
            width += left;
            if (data != nullptr)
                *data = &(*data)[-left];
            left = 0;
        }
        if (top < 0) {
            height += top;
            if (data != nullptr)
                *data = &(*data)[-top * stride];
            top = 0;
        }
        if (left + width > WIDTH) {
            width = WIDTH - left;
        }
        if (top + height > HEIGHT) {
            height = HEIGHT - top;
        }
        if (left >= WIDTH || top >= HEIGHT || width <= 0 || height <= 0 || left + width <= 0 || top + height <= 0) {
            return false;
        }
        return true;
    }

    void fill_rect(int left, int top, int width, int height, Pixel color) {
        const bool in_frame = validate_rect(left, top, width, height);
        if (!in_frame)
            return;
        auto *frame_ptr = lcd::get_offscreen_ptr_unsafe();
        for (int y = top; y < top + height; y++) {
            auto *ptr = &frame_ptr[y * WIDTH];
            for (int x = left; x < left + width; x++) {
                ptr[x] = color;
            }
        }
    }

    void copy(int left, int top, int width, int height, const Pixel *pixels) {
        const int  stride   = width;
        const bool in_frame = validate_rect(left, top, width, height, &pixels, stride);
        if (!in_frame)
            return;
        auto *frame_ptr = lcd::get_offscreen_ptr_unsafe();
        for (int y = 0; y < height; y++) {
            const auto *src_ptr = &pixels[y * stride];
            auto       *dst_ptr = &frame_ptr[(top + y) * WIDTH];
            for (int x = 0; x < width; x++) {
                dst_ptr[left + x] = src_ptr[x];
            }
        }
    }

    void draw_image(int left, int top, const Image &image) {
        copy(left, top, image.width, image.height, image.color_data);
    }

    void fill_masked(int left, int top, Pixel color, const Image &mask) {
        int       width  = mask.width;
        int       height = mask.height;
        const int stride = mask.width;
        auto      data   = mask.alpha_data;
        if (data == nullptr)
            return;
        const bool in_frame = validate_rect(left, top, width, height, &data, stride);
        if (!in_frame)
            return;
        auto *frame_ptr = lcd::get_offscreen_ptr_unsafe();
        for (int y = top; y < top + height; y++) {
            const auto *src_ptr = &data[y * stride];
            auto       *dst_ptr = &frame_ptr[y * WIDTH];
            for (int x = left; x < left + width; x++) {
                const auto alpha      = src_ptr[x];
                const auto rgb        = dst_ptr[x];
                const auto pixel_mask = from_grayscale(alpha);
                const auto result     = (rgb & ~pixel_mask) | (color & pixel_mask);
                dst_ptr[x]            = result;
            }
        }
    }

} // namespace drawing
