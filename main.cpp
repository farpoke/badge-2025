#include <assets.hpp>
#include <cmath>
#include <cstdio>

#include <pico/bootrom.h>
#include <pico/stdlib.h>
#include <pico/time.h>

#include <tusb.h>

#include <badge/buttons.hpp>
#include <badge/drawing.hpp>
#include <badge/font.hpp>
#include <core/core1.hpp>
#include <games/blocks.hpp>
#include <games/flappy.hpp>
#include <games/othello.hpp>
#include <games/snek.hpp>
#include <ui/code_entry.hpp>
#include <ui/menu.hpp>
#include <ui/readme.hpp>
#include <ui/splash.hpp>
#include <ui/ui.hpp>
#include <usb/usb.hpp>


class FontTest final : public ui::State {
public:
    void update(int delta_ms) override {
        State::update(delta_ms);
        if (buttons::b())
            ui::pop_state();
    }

    void draw() override {
        drawing::draw_image(0, 0, image::splash_bg);
        drawing::fill_rect(0, 0, lcd::WIDTH, lcd::HEIGHT, COLOR_BLACK, 150);

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

[[noreturn]] int main() {
    stdio_init_all();
    printf("\n===== lcd-test =====\n");

    usb::init();
    buttons::init();

    core1::reset_and_launch();

    const auto menu = ui::make_state<ui::MainMenu>();
    menu->add_item("README", ui::make_state<ui::Readme>());
    menu->add_item("Code Entry", ui::make_state<ui::CodeEntry>());
    menu->add_item("Found Flags", nullptr);
    menu->add_item("Blocks", ui::make_state<blocks::BlocksGame>());
    menu->add_item("Snek", ui::make_state<snek::SnekGame>());
    menu->add_item("Othello", ui::make_state<othello::OthelloGame>());
    // menu->add_item("Flappy", ui::make_state<flappy::FlappyGame>());
    menu->add_item("GPIO Control", nullptr);
    menu->add_item("SAO Control", nullptr);
    // menu->add_item("Font Test", ui::make_state<FontTest>());
    menu->add_item("Bootloader", [] { rom_reset_usb_boot_extra(-1, 0, false); });

    ui::push_state(menu);
    ui::push_new_state<ui::SplashScreen>();

    ui::push_new_state<othello::OthelloGame>();

    printf("> Main loop...\n");
    auto last_frame_time = get_absolute_time();
    while (true) {

        while (tud_task_event_ready())
            tud_task();

        const auto now = get_absolute_time();
        const auto delta_time_ms = absolute_time_diff_us(last_frame_time, now) / 1000;

        if (delta_time_ms < 10)
            continue;

        last_frame_time = now;

        core1::swap_frame();

        buttons::update();

        ui::update(delta_time_ms);
        ui::draw();
    }
}
