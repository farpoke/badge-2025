#include <assets.hpp>
#include <cmath>
#include <cstdio>

#include <pico/stdlib.h>
#include <pico/time.h>

#include <tusb.h>

#include <badge/buttons.hpp>
#include <badge/drawing.hpp>
#include <badge/font.hpp>
#include <core/core1.hpp>
#include <ui/splash.hpp>
#include <ui/ui.hpp>
#include <usb/usb.hpp>

class FontTest final : public ui::State {
public:
    void draw() override {
        drawing::draw_image(0, 0, image::splash_bg);

        auto x = 2;
        auto y = 2;
        font::TextDraw render = {};

        auto do_render = [&](auto font, auto text) {
            render = font.render(text);
            drawing::draw_text(x - render.dx, y - render.dy, COLOR_BLACK, 150, COLOR_WHITE, render);
            y += render.height + 2;
        };

        do_render(font::lucida, "\"lucida\" Hello world!");
        do_render(font::m5x7, "\"m5x7\" Hello world!");
        do_render(font::m6x11, "\"m6x11\" Hello world!");
        do_render(font::noto_sans, "\"Noto Sans\" Hello world!");
        do_render(font::noto_sans_cm, "\"Noto Sans Condensed Medium\" Hello world!");

    }
};

[[noreturn]] int main()
{
    stdio_init_all();
    printf("\n===== lcd-test =====\n");

    usb::init();
    buttons::init();

    core1::reset_and_launch();

    ui::push_new_state<FontTest>();
    ui::push_new_state<ui::SplashScreen>();

    printf("> Main loop...\n");
    auto last_frame_time = get_absolute_time();
    while(true) {

        while (tud_task_event_ready())
            tud_task();

        const auto now = get_absolute_time();
        const auto delta_time_ms = absolute_time_diff_us(last_frame_time, now) / 1000;

        if (delta_time_ms < 10) continue;

        last_frame_time = now;

        core1::swap_frame();

        buttons::update();

        ui::update(delta_time_ms);
        ui::draw();
    }
}
