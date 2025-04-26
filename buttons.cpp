#include "buttons.hpp"

#include <hardware/gpio.h>

namespace buttons
{
    constexpr uint32_t MASK = 0
        | (1 << BTN_LEFT_UP)
        | (1 << BTN_LEFT_DOWN)
        | (1 << BTN_LEFT_LEFT)
        | (1 << BTN_LEFT_RIGHT)
        | (1 << BTN_LEFT_PUSH)
        | (1 << BTN_RIGHT_UP)
        | (1 << BTN_RIGHT_DOWN)
        | (1 << BTN_RIGHT_LEFT)
        | (1 << BTN_RIGHT_RIGHT)
        | (1 << BTN_RIGHT_PUSH);

    uint32_t current_state = 0;
    uint32_t previous_state = 0;

    void init_common(int pin) {
        gpio_set_function(pin, GPIO_FUNC_SIO);
        gpio_set_dir(pin, true);
        gpio_put(pin, 0);
    }

    void init_input(int pin) {
        gpio_set_function(pin, GPIO_FUNC_SIO);
        gpio_set_dir(pin, false);
        gpio_set_input_enabled(pin, true);
        gpio_pull_up(pin);
    }

    void init() {
        init_common(BTN_LEFT_COMMON);
        init_common(BTN_RIGHT_COMMON);
        for (int i = 0; i < 32; i++)
            if (MASK & (1 << i))
                init_input(i);
    }

    void update() {
        previous_state = current_state;
        current_state = ~gpio_get_all() & MASK;
    }

    bool get(uint32_t mask) {
        return (current_state & mask) & ~(previous_state & mask);
    }

    bool get_current(uint32_t mask) {
        return current_state & mask;
    }

    bool get_changed(uint32_t mask) {
        return (current_state & mask) ^ (previous_state & mask);
    }

}
