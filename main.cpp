#include <cmath>
#include <cstdio>

#include <pico/stdlib.h>
#include <pico/time.h>

#include <tusb.h>

#include <lvgl/lvgl.h>

#include <badge/buttons.hpp>
#include <badge/lcd.hpp>
#include <mpy/mphalport.h>
#include <mpy/mpy.hpp>
#include <usb/usb.hpp>

#include <assets.hpp>

void setup_lvgl();

[[noreturn]] int main()
{
    stdio_init_all();
    printf("\n===== lcd-test =====\n");

    usb::init();
    buttons::init();
    mpy::init();

    stdio_flush();

    soft_timer_init();

    setup_lvgl();

    // core1::reset_and_launch();

    // drawing::draw_image(0, 0, image::splash_bg);
    // drawing::fill_masked(0, 0, lcd::from_grayscale(0), image::splash_fg);
    // drawing::fill_rect(20, 20, 80, 80, lcd::to_pixel(20, 30, 40));
    // drawing::draw_rect(25, 25, 70, 70, lcd::to_pixel(255, 128, 0));
    // const auto line_color = lcd::to_pixel(0, 255, 255);
    // for (int i = 25; i < 95; i += 10) {
    //     drawing::draw_line_aa(60, 60, 25, i, line_color);
    //     drawing::draw_line_aa(60, 60, 94, i, line_color);
    //     drawing::draw_line_aa(60, 60, i, 25, line_color);
    //     drawing::draw_line_aa(60, 60, i, 94, line_color);
    // }
    // core1::swap_frame();

    while (false) {
        mpy::repl();
    }

    auto *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);

    auto *container = lv_obj_create(screen);
    lv_obj_set_size(container, lcd::WIDTH, lcd::HEIGHT);
    lv_obj_set_pos(container, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_border_width(container, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(container, lv_color_white(), LV_PART_MAIN);

    auto *label = lv_label_create(container);
    lv_label_set_text(label, "Hello World!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    printf("> Main loop...\n");
    bool active = true;
    while(true) {
        
        buttons::update();
        if (buttons::rh_push()) {
            active = !active;

            if (active) {
                lcd::backlight_on(20);
            }
            else {
                lcd::backlight_off();
            }
        }

        if (!active) {
            continue;
        }

        if (buttons::lh_push()) {
            printf("  Hi!\n");
            usb::write("Hi!\n");
        }

        while (tud_task_event_ready())
            tud_task();

        lv_timer_handler();
    }
}
