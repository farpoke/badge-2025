#include "drawing.hpp"

#include <cmath>

#include <hardware/interp.h>

namespace drawing
{

    /*
    constexpr Pixel blend_mask(Pixel dst, Pixel src, uint8_t mask) {
        const auto pixel_mask = from_grayscale(mask);
        return (dst & ~pixel_mask) | (src & pixel_mask);
    }
    */

    void init_interp_blend() {
        auto cfg = interp_default_config();
        interp_config_set_blend(&cfg, true);
        interp_set_config(interp0, 0, &cfg);

        cfg = interp_default_config();
        interp_set_config(interp0, 1, &cfg);
    }

    Pixel blend_alpha(Pixel dst, Pixel src, uint8_t alpha) {
        // Use interpolator 0 in blend mode to interpolate between source and destination color.
        interp0->accum[1] = alpha;
        // Interpolate the red channel (upper 5 bits of 16-bit pixel):
        interp0->base[0]  = dst & 0xF800;
        interp0->base[1]  = src & 0xF800;
        Pixel result      = interp0->peek[1] & 0xF800;
        // Interpolate the green channel (middle 6 bits of 16-bit pixel):
        interp0->base[0]  = dst & 0x07E0;
        interp0->base[1]  = src & 0x07E0;
        result |= interp0->peek[1] & 0x07E0;
        // Interpolate the blue channel (lower 5 bits of 16-bit pixel):
        interp0->base[0]  = dst & 0x001F;
        interp0->base[1]  = src & 0x001F;
        result |= interp0->peek[1] & 0x001F;
        //
        return result;
    }

    void clear(Pixel color) {
        auto *ptr = lcd::get_offscreen_ptr_unsafe();
        for (int i = 0; i < WIDTH * HEIGHT; i++)
            ptr[i] = color;
    }

    void draw_pixel(int x, int y, Pixel color) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
            return;
        auto *ptr          = lcd::get_offscreen_ptr_unsafe();
        ptr[y * WIDTH + x] = color;
    }

    void draw_line(int x0, int y0, int x1, int y1, Pixel color) {
        // See http://members.chello.at/~easyfilter/bresenham.html
        const int dx  = abs(x1 - x0);
        const int sx  = x0 < x1 ? 1 : -1;
        const int dy  = -abs(y1 - y0);
        const int sy  = y0 < y1 ? 1 : -1;
        int       err = dx + dy;
        int       x   = x0;
        int       y   = y0;
        while (true) {
            draw_pixel(x, y, color);
            if (x == x1 && y == y1)
                break;
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
        const int dx  = abs(x1 - x0);
        const int sx  = x0 < x1 ? 1 : -1;
        const int dy  = -abs(y1 - y0);
        const int sy  = y0 < y1 ? 1 : -1;
        int       err = dx + dy;
        int       x   = x0;
        int       y   = y0;
        while (true) {
            draw_pixel(x, y, color);
            if (x == x1 && y == y1)
                break;
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

    int validate_rect(int &left, int &top, int &width, int &height, int stride = 0) {
        int offset = 0;
        if (stride == 0)
            stride = width;
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
            offset += -left;
            left = 0;
        }
        if (top < 0) {
            height += top;
            offset += -top * stride;
            top = 0;
        }
        if (left + width > WIDTH) {
            width = WIDTH - left;
        }
        if (top + height > HEIGHT) {
            height = HEIGHT - top;
        }
        if (left >= WIDTH || top >= HEIGHT || width <= 0 || height <= 0 || left + width <= 0 || top + height <= 0) {
            return -1;
        }
        return offset;
    }

    void fill_rect(int left, int top, int width, int height, Pixel color) {
        const auto offset = validate_rect(left, top, width, height);
        if (offset < 0)
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
        const int  stride = width;
        const auto offset = validate_rect(left, top, width, height, stride);
        if (offset < 0)
            return;
        auto *frame_ptr = lcd::get_offscreen_ptr_unsafe();
        for (int y = 0; y < height; y++) {
            const auto *src_ptr = &pixels[y * stride + offset];
            auto       *dst_ptr = &frame_ptr[(top + y) * WIDTH];
            for (int x = 0; x < width; x++) {
                dst_ptr[left + x] = src_ptr[x];
            }
        }
    }

    void copy_alpha(int left, int top, int width, int height, const Pixel *pixels, const uint8_t *alpha) {
        const int  stride = width;
        const auto offset = validate_rect(left, top, width, height, stride);
        if (offset < 0)
            return;
        auto *frame_ptr = lcd::get_offscreen_ptr_unsafe();
        init_interp_blend();
        for (int y = 0; y < height; y++) {
            const auto *src_ptr  = &pixels[y * stride + offset];
            const auto *alpha_ptr = &alpha[y * stride + offset];
            auto       *dst_ptr  = &frame_ptr[(top + y) * WIDTH];
            for (int x = 0; x < width; x++) {
                dst_ptr[left + x] = blend_alpha(dst_ptr[left + x], src_ptr[x], alpha_ptr[x]);
            }
        }
    }

    void draw_image(int left, int top, const Image &image) {
        if (image.alpha_data == nullptr)
            copy(left, top, image.width, image.height, image.color_data);
        else
            copy_alpha(left, top, image.width, image.height, image.color_data, image.alpha_data);
    }

    void fill_alpha(int left, int top, Pixel color, const Image &mask) {
        int        width  = mask.width;
        int        height = mask.height;
        const int  stride = mask.width;
        const auto data   = mask.alpha_data;
        if (data == nullptr)
            return;
        const auto offset = validate_rect(left, top, width, height, stride);
        if (offset < 0)
            return;
        auto *frame_ptr = lcd::get_offscreen_ptr_unsafe();
        init_interp_blend();
        for (int y = top; y < top + height; y++) {
            const auto *src_ptr = &data[y * stride + offset];
            auto       *dst_ptr = &frame_ptr[y * WIDTH];
            for (int x = left; x < left + width; x++) {
                dst_ptr[x] = blend_alpha(dst_ptr[x], color, src_ptr[x]);
            }
        }
    }

} // namespace drawing
