#pragma once

#include <cstdint>

#include "badge-2025.h"

namespace buttons
{

    void init();
    void update();

    uint32_t get(uint32_t mask);
    uint32_t get_current(uint32_t mask);
    uint32_t get_changed(uint32_t mask);

#define MAKE_BUTTON_FUNCS(name, NAME)                                                                                  \
    inline bool name() { return get(1 << NAME); }                                                                      \
    inline bool name##_current() { return get_current(1 << NAME); }                                                    \
    inline bool name##_changed() { return get_changed(1 << NAME); }

    MAKE_BUTTON_FUNCS(up, BTN_UP)
    MAKE_BUTTON_FUNCS(down, BTN_DOWN)
    MAKE_BUTTON_FUNCS(left, BTN_LEFT)
    MAKE_BUTTON_FUNCS(right, BTN_RIGHT)
    MAKE_BUTTON_FUNCS(push, BTN_PUSH)
    MAKE_BUTTON_FUNCS(a, BTN_A)
    MAKE_BUTTON_FUNCS(b, BTN_B)
    MAKE_BUTTON_FUNCS(c, BTN_B)
    MAKE_BUTTON_FUNCS(d, BTN_C)

#undef MAKE_BUTTON_FUNCS

} // namespace buttons
