#include "animation.hpp"

#include <assets.hpp>

#include <badge/buttons.hpp>
#include <ui/ui.hpp>

namespace ui
{

    void Animation::update(int delta_ms) {
        if (buttons::b())
            pop_state();
        else
            anim->update(delta_ms);
    }

    void Animation::draw() {
        anim->draw();
    }

    void Animation::pause() {
        anim->reset();
    }

    void Animation::resume() {
        anim->initialize();
    }

}
