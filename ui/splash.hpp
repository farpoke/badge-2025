#pragma once

#include "state.hpp"

#include <badge/image.hpp>

namespace ui
{

    class SplashScreen final : public State {
    public:
        static constexpr auto DURATION_MS = 2'000;
        static constexpr auto TICK_DIVIDER = 2;
        static constexpr auto DELAY_1 = 100;
        static constexpr auto DELAY_2 = 500;

        SplashScreen();
        ~SplashScreen() override;

        void update(int delta_ms) override;
        void draw() override;

    protected:
        uint8_t* mask;
        image::Image bg_image;
    };

}
