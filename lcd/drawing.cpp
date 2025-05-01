#include "drawing.hpp"

namespace drawing
{

    void fill_rect(int left, int right, int top, int bottom, Pixel color) {
        if (left > right) {
            const auto tmp = left;
            left = right;
            right = tmp;
        }
        if (top > bottom) {
            const auto tmp = top;
            top = bottom;
            bottom = tmp;
        }
        if (left < 0) left = 0;
        if (right >= WIDTH) right = WIDTH - 1;
        if (top < 0) top = 0;
        if (bottom >= HEIGHT) bottom = HEIGHT - 1;
        auto* frame_ptr = lcd::get_offscreen_ptr_unsafe();
        for (int y = top; y <= bottom; y++) {
            auto* ptr = &frame_ptr[y * WIDTH];
            for (int x = left; x <= right; x++) {
                ptr[x] = color;
            }
        }
    }

    void copy(int left, int right, int top, int bottom, const Pixel* pixels) {
        if (left > right || top > bottom) return;
        const auto stride = right - left + 1;
        if (left < 0) {
            pixels = &pixels[-left];
            left = 0;
        }
        if (right >= WIDTH) {
            right = WIDTH - 1;
        }
        if (top < 0) {
            pixels = &pixels[-top * stride];
            top = 0;
        }
        if (bottom >= HEIGHT) {
            bottom = HEIGHT - 1;
        }
        const int width = right - left + 1;
        const int height = bottom - top + 1;
        auto* frame_ptr = lcd::get_offscreen_ptr_unsafe();
        for (int y = 0; y < height; y++) {
            const auto* src_ptr = &pixels[y * stride];
            auto* dst_ptr = &frame_ptr[(top + y) * WIDTH];
            for (int x = 0; x < width; x++) {
                dst_ptr[left + x] = src_ptr[x];
            }
        }
    }

}
