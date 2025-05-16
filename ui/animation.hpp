#pragma once

#include "state.hpp"

#include <badge/animation.hpp>

namespace ui
{

    class Animation final : public State {
    public:
        explicit Animation(anim::Animation* anim) : anim(anim) {}

        void update(int delta_ms) override;
        void draw() override;

        void pause() override;
        void resume() override;

    private:
        anim::Animation* anim;

    };

}
