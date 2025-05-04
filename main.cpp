#include <cmath>
#include <cstdio>

#include <pico/stdlib.h>
#include <pico/time.h>

#include <tusb.h>

#include <badge/buttons.hpp>
#include <badge/lcd.hpp>
#include <core/core1.hpp>
#include <ui/splash.hpp>
#include <ui/ui.hpp>
#include <usb/usb.hpp>

[[noreturn]] int main()
{
    stdio_init_all();
    printf("\n===== lcd-test =====\n");

    usb::init();
    buttons::init();

    core1::reset_and_launch();

    ui::push_new_state<ui::SplashScreen>();

    printf("> Main loop...\n");
    bool active = true;
    auto last_frame_time = get_absolute_time();
    while(true) {
        
        buttons::update();
        if (buttons::rh_push()) {
            active = !active;

            if (active) {
                lcd::backlight_on(20);
                last_frame_time = get_absolute_time();
            }
            else {
                lcd::backlight_off();
            }
        }

        if (!active) {
            continue;
        }

        while (tud_task_event_ready())
            tud_task();

        const auto now = get_absolute_time();
        const auto delta_time_ms = absolute_time_diff_us(last_frame_time, now) / 1000;
        last_frame_time = now;

        ui::update(delta_time_ms);
        ui::draw();

        core1::swap_frame();
    }
}
