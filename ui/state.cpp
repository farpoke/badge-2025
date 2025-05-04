#include "state.hpp"

namespace ui
{

    void State::update(int delta_ms) {
        time_ms += delta_ms;
    }

    void State::pause() {
        active = false;
    }

    void State::resume() {
        active = true;
    }

}
