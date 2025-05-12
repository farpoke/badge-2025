#pragma once

#include "state.hpp"

namespace ui
{

    class AnimationGallery final : public State {
    public:

        void update(int delta_ms) override;
        void draw() override;

    private:

    };

}
