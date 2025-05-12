#pragma once

#include <cstdint>
#include <memory>
#include <span>

#include <badge/pixel.hpp>

namespace anim
{

    class Animation {
    public:
        explicit Animation(std::span<const uint8_t> data) : data(data) {};

        void initialize();
        void update(int delta_ms);
        void draw() const;
        void reset();

    private:
        std::span<const uint8_t> data;

        int n_frames = 0;
        int interval = 0;
        int bpp = 0;
        int current_frame = 0;
        int countdown = 0;
        const uint8_t* frame0_ptr = nullptr;
        const uint8_t* current_ptr = nullptr;
        std::unique_ptr<Pixel[]> palette = {};
        std::unique_ptr<uint8_t[]> frame = {};

        void read_frame();

    };

}
