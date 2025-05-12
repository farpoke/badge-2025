#include "animations.hpp"

#include <assets.hpp>

#include <ui/ui.hpp>

namespace ui
{

    void AnimationGallery::update(int delta_ms) {
        anim::dramatic.update(delta_ms);
    }

    void AnimationGallery::draw() {
        anim::dramatic.draw();
    }

    void AnimationGallery::pause() {
        anim::dramatic.reset();
    }

    void AnimationGallery::resume() {
        anim::dramatic.initialize();
    }

}
