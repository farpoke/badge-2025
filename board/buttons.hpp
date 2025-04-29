#pragma once

#include <cstdint>

#include "badge-2025.h"

namespace buttons
{

    void init();
    void update();

    bool get(uint32_t mask);
    bool get_current(uint32_t mask);
    bool get_changed(uint32_t mask);

#define MAKE_BUTTON_FUNCS(name, NAME)                               \
    inline bool name() { return get(1 << NAME); }                   \
    inline bool name##_current() { return get_current(1 << NAME); } \
    inline bool name##_changed() { return get_changed(1 << NAME); }

    MAKE_BUTTON_FUNCS(lh_up,    BTN_LEFT_UP)
    MAKE_BUTTON_FUNCS(lh_down,  BTN_LEFT_DOWN)
    MAKE_BUTTON_FUNCS(lh_left,  BTN_LEFT_LEFT)
    MAKE_BUTTON_FUNCS(lh_right, BTN_LEFT_RIGHT)
    MAKE_BUTTON_FUNCS(lh_push,  BTN_LEFT_PUSH)
    MAKE_BUTTON_FUNCS(rh_up,    BTN_RIGHT_UP)
    MAKE_BUTTON_FUNCS(rh_down,  BTN_RIGHT_DOWN)
    MAKE_BUTTON_FUNCS(rh_left,  BTN_RIGHT_LEFT)
    MAKE_BUTTON_FUNCS(rh_right, BTN_RIGHT_RIGHT)
    MAKE_BUTTON_FUNCS(rh_push,  BTN_RIGHT_PUSH)

#undef MAKE_BUTTON_FUNCS

}
