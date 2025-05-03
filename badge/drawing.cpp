#include "drawing.hpp"

#include <math.h>

namespace drawing
{

    void clear(Pixel color) {
        auto* ptr = lcd::get_offscreen_ptr_unsafe();
        for (int i = 0; i < WIDTH * HEIGHT; i++)
            ptr[i] = color;
    }

    void draw_pixel(int x, int y, Pixel color) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
            return;
        auto* ptr = lcd::get_offscreen_ptr_unsafe();
        ptr[y * WIDTH + x] = color;
    }

    void draw_line(int x0, int y0, int x1, int y1, Pixel color) {
        // See http://members.chello.at/~easyfilter/bresenham.html
        const int dx = abs(x1 - x0);
        const int sx = x0 < x1 ? 1 : -1;
        const int dy = -abs(y1 - y0);
        const int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        int x = x0;
        int y = y0;
        while (true) {
            draw_pixel(x, y, color);
            if (x == x1 && y == y1) break;
            const auto e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y += sy;
            }
        }
    }

    void draw_line_aa(int x0, int y0, int x1, int y1, Pixel color) {
        // See http://members.chello.at/~easyfilter/bresenham.html
        const int dx = abs(x1 - x0);
        const int sx = x0 < x1 ? 1 : -1;
        const int dy = -abs(y1 - y0);
        const int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        int x = x0;
        int y = y0;
        while (true) {
            draw_pixel(x, y, color);
            if (x == x1 && y == y1) break;
            const auto e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y += sy;
            }
        }
    }

    void draw_rect(int left, int top, int width, int height, Pixel color) {
        const auto x0 = left;
        const auto x1 = left + width - 1;
        const auto y0 = top;
        const auto y1 = top + height - 1;
        draw_line(x0, y0, x1, y0, color);
        draw_line(x0, y1, x1, y1, color);
        draw_line(x0, y0, x0, y1, color);
        draw_line(x1, y0, x1, y1, color);
    }

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
