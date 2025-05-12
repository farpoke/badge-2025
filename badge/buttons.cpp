#include "buttons.hpp"

#include <hardware/gpio.h>

namespace buttons
{
    constexpr uint32_t MASK = 0
        | (1 << BTN_UP)
        | (1 << BTN_DOWN)
        | (1 << BTN_LEFT)
        | (1 << BTN_RIGHT)
        | (1 << BTN_PUSH)
        | (1 << BTN_A)
        | (1 << BTN_B)
        | (1 << BTN_C)
        | (1 << BTN_D);

    uint32_t current_state = 0;
    uint32_t previous_state = 0;

    void init_input(int pin) {
        gpio_set_function(pin, GPIO_FUNC_SIO);
        gpio_set_dir(pin, false);
        gpio_set_input_enabled(pin, true);
        gpio_pull_up(pin);
    }

    void init() {
        for (int i = 0; i < 32; i++)
            if (MASK & (1 << i))
                init_input(i);
    }

    void update() {
        previous_state = current_state;
        current_state = ~gpio_get_all() & MASK;
    }

    uint32_t get(uint32_t mask) {
        return (current_state & mask) & ~(previous_state & mask);
    }

    uint32_t get_current(uint32_t mask) {
        return current_state & mask;
    }

    uint32_t get_changed(uint32_t mask) {
        return (current_state & mask) ^ (previous_state & mask);
    }

}
