#pragma once

#include "state.hpp"

namespace ui
{

    class SplashScreen final : public State {
    public:
        static constexpr auto DURATION_MS = 5'000;

        void update(int delta_ms) override;
        void draw() override;
    };

}
