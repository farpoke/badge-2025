#include "animation.hpp"

#include <cassert>

#include <badge/lcd.hpp>

namespace anim
{

    void Animation::initialize() {
        current_ptr = data.data();

        n_frames = *current_ptr++;
        interval = *current_ptr++;
        bpp = *current_ptr++;

        const auto n_colors = 1 << bpp;
        palette.reset(new Pixel[n_colors]);
        for (int i = 0; i < n_colors; i++) {
            const auto r = *current_ptr++;
            const auto g = *current_ptr++;
            const auto b = *current_ptr++;
            palette[i] = rgb888(r, g, b);
        }

        frame.reset(new uint8_t[lcd::WIDTH * lcd::HEIGHT]);

        current_frame = 0;
        countdown = interval;
        frame0_ptr = current_ptr;
        read_frame();
    }

    void Animation::update(int delta_ms) {
        countdown -= delta_ms;
        while (countdown < 0) {
            countdown += interval;
            current_frame = (current_frame + 1) % n_frames;
            read_frame();
        }
    }

    void Animation::draw() const {
        auto* ptr = lcd::get_offscreen_ptr_unsafe();
        for (int y = 0; y < lcd::HEIGHT; y++) {
            for (int x = 0; x < lcd::WIDTH; x++) {
                *ptr++ = palette[frame[y * lcd::WIDTH + x]];
            }
        }
    }

    void Animation::reset() {
        palette.reset();
        frame.reset();
    }

    void Animation::read_frame() {
        uint16_t buffer = 0;
        int buffer_bits = 0;
        const auto get_bits = [&](int n_bits) {
            if (buffer_bits < n_bits) {
                assert(current_ptr < data.data() + data.size());
                buffer |= *current_ptr++ << buffer_bits;
                buffer_bits += 8;
            }
            const auto value = buffer & ((1 << n_bits) - 1);
            buffer >>= n_bits;
            buffer_bits -= n_bits;
            return value;
        };

        if (current_frame == 0) {
            current_ptr = frame0_ptr;
            for (int i = 0; i < lcd::WIDTH * lcd::HEIGHT; i++) {
                frame[i] = get_bits(bpp);
            }
        }
        else {
            int idx = 0;
            while (idx < lcd::WIDTH * lcd::HEIGHT) {
                const bool diff = get_bits(1);
                int run = get_bits(7);
                if (run == 0)
                    run = 1 << 7;
                if (diff) {
                    for (int i = 0; i < run; i++)
                        frame[idx + i] = get_bits(bpp);
                }
                idx += run;
            }
            assert(idx == lcd::WIDTH * lcd::HEIGHT);
        }
    }

}
