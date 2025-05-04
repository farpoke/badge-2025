#pragma once

#include "state.hpp"

#include <badge/image.hpp>

namespace ui
{

    class SplashScreen final : public State {
    public:
        static constexpr auto DURATION_MS = 2'000;
        static constexpr auto STRIPE_SPACING = 100;

        SplashScreen();
        ~SplashScreen() override;

        void update(int delta_ms) override;
        void draw() override;

    protected:
        uint8_t* mask;
        image::Image bg_image;
    };

}
