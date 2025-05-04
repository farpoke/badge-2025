#pragma once

#include <cstdint>

#include "state.hpp"

namespace ui
{

    constexpr int STATE_STACK_SIZE = 8;

    void update(int delta_ms);
    void draw();

    uint32_t get_ui_time_ms();

    void push_state(const StatePtr& state);
    StatePtr pop_state();

    template<typename T, typename ...Args>
    StatePtr push_new_state(Args&& ... args) {
        auto state = make_state<T>(std::forward<Args>(args)...);
        push_state(state);
        return state;
    }

}
