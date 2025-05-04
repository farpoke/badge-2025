#include "ui.hpp"

#include <badge/drawing.hpp>

namespace ui
{

    static uint32_t _ui_time_ms = 0;
    StatePtr _current_state = nullptr;
    StatePtr _state_stack[STATE_STACK_SIZE];
    int _state_stack_size = 0;

    void update(int delta_ms) {
        _ui_time_ms += delta_ms;
        if (_current_state)
            _current_state->update(delta_ms);
    }

    void draw() {
        if (_current_state)
            _current_state->draw();
        else
            drawing::clear(0);
    }

    uint32_t get_time_ms() {
        return _ui_time_ms;
    }

    void push_state(const StatePtr& state) {
        if (_current_state) {
            _state_stack[_state_stack_size++] = _current_state;
            _current_state->pause();
        }
        _current_state = state;
        if (_current_state) {
            _current_state->resume();
        }
    }

    StatePtr pop_state() {
        auto prev_state = _current_state;
        if (_state_stack_size == 0)
            _current_state = nullptr;
        else {
            _current_state = _state_stack[--_state_stack_size];
            _state_stack[_state_stack_size] = nullptr;
        }
        if (prev_state)
            prev_state->pause();
        if (_current_state)
            _current_state->resume();
        return prev_state;
    }

}
