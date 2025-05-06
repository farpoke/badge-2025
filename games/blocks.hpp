#pragma once

#include <ui/state.hpp>

namespace blocks
{

    constexpr auto GRID_WIDTH = 8;
    constexpr auto GRID_HEIGHT = 8;

    class BlocksGame final : public ui::State {
    public:

        void update(int delta_ms) override;
        void draw() override;

        void pause() override;
        void resume() override;

    };

}
