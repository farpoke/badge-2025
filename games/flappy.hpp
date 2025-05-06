#pragma once

#include <ui/state.hpp>

namespace flappy
{

    class FlappyGame final : public ui::State {
    public:

        void update(int delta_ms) override;
        void draw() override;

        void pause() override;
        void resume() override;

    };

}
