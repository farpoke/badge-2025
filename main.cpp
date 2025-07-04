
#include <cmath>
#include <cstdio>

#include <pico/bootrom.h>
#include <pico/stdlib.h>
#include <pico/time.h>

#include <tusb.h>

#include <assets.hpp>

#include <badge/buttons.hpp>
#include <badge/drawing.hpp>
#include <badge/factory_test.hpp>
#include <badge/font.hpp>
#include <badge/storage.hpp>
#include <games/blocks.hpp>
#include <games/flappy.hpp>
#include <games/othello.hpp>
#include <games/snek.hpp>
#include <ui/animation.hpp>
#include <ui/code_entry.hpp>
#include <ui/flag_view.hpp>
#include <ui/menu.hpp>
#include <ui/qr_code.hpp>
#include <ui/readme.hpp>
#include <ui/splash.hpp>
#include <ui/ui.hpp>
#include <usb/usb.hpp>


extern "C" void launch_doom(void);


void enable_stdio_to_usb();


void enable_pwr_leds() {
    gpio_set_function(PWR_LED_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(PWR_LED_PIN, true);
    gpio_put(PWR_LED_PIN, true);
}


void disable_pwr_leds() {
    gpio_set_function(PWR_LED_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(PWR_LED_PIN, true);
    gpio_put(PWR_LED_PIN, false);
}


class Website final : public ui::State {
public:
    ui::qr::QrCode code;

    Website() {
        code.version = ui::qr::Version::V2_25x25;
        code.ec = ui::qr::ErrorCorrection::MEDIUM;
        code.content = "https://hack.gbgay.com/";
    }

    void update(int delta_ms) override {
        State::update(delta_ms);
        if (buttons::b())
            ui::pop_state();
    }

    void draw() override {
        drawing::clear(COLOR_WHITE);

        code.draw((lcd::WIDTH - code.get_image_size()) / 2, (lcd::HEIGHT - code.get_image_size()) / 2);
    }

    void pause() override {
        code.reset();
    }

    void resume() override {
        code.generate();
        code.render(4);
    }
};


class FontTest final : public ui::State {
public:
    void update(int delta_ms) override {
        State::update(delta_ms);
        if (buttons::b())
            ui::pop_state();
    }

    void draw() override {
        drawing::clear(COLOR_BLACK);

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

ui::StatePtr create_gallery_menu() {
    auto menu = ui::make_state<ui::Menu>();
    menu->add_item("Blahaj", ui::make_state<ui::Animation>(&anim::blahaj_spin));
    menu->add_item("Dramatic", ui::make_state<ui::Animation>(&anim::dramatic));
    menu->add_item("Fire", ui::make_state<ui::Animation>(&anim::fire));
    menu->add_item("Hi There", ui::make_state<ui::Animation>(&anim::hi_there));
    menu->add_item("Pedro", ui::make_state<ui::Animation>(&anim::pedro));
    menu->add_item("Rap Win", ui::make_state<ui::Animation>(&anim::rap_win));
    menu->add_item("gbgay{", ui::make_state<ui::Animation>(&anim::rick));
    return menu;
}

ui::StatePtr create_main_menu() {

    auto menu = ui::make_state<ui::Menu>();
    menu->is_main = true;
    menu->add_item("README", ui::make_state<ui::Readme>());
    menu->add_item("Website", ui::make_state<Website>());
    menu->add_item("Code Entry", ui::make_state<ui::CodeEntry>());
    menu->add_item("Found Flags", ui::make_state<ui::FlagView>());
    menu->add_item("Snek", ui::make_state<snek::SnekGame>());
    menu->add_item("Blocks", ui::make_state<blocks::BlocksGame>());
    menu->add_item("Othello", ui::make_state<othello::OthelloGame>());
    // menu->add_item("Flappy", ui::make_state<flappy::FlappyGame>());
    menu->add_item("Gallery", create_gallery_menu());
    // menu->add_item("GPIO Control", nullptr);
    // menu->add_item("SAO Control", nullptr);
    // menu->add_item("Font Test", ui::make_state<FontTest>());
    menu->add_item("Bootloader", [] { rom_reset_usb_boot_extra(-1, 0, false); });

    return menu;

}

[[noreturn]] int main() {

    enable_pwr_leds();

    stdio_init_all();
    printf("\n===== HackGBGay 2025 =====\n");

    usb::init();
    buttons::init();
    storage::init();
    flags::init();
    lcd::init();

#if !FACTORY_TEST

    const auto menu = create_main_menu();
    ui::push_state(menu);
    ui::push_new_state<ui::SplashScreen>();

    disable_pwr_leds();

#else

    ui::push_new_state<factory::FactoryTest>();

#endif

    /*
    enable_stdio_to_usb();
    auto start = get_absolute_time();
    while (absolute_time_diff_us(start, get_absolute_time()) < 5'000'000) {
        tud_task();
    }
    */

    stdio_flush();

    // launch_doom();

    printf("> Main loop...\n");
    auto last_frame_time = get_absolute_time();
    while (true) {

        while (tud_task_event_ready())
            tud_task();

        const auto now = get_absolute_time();
        const auto delta_time_ms = absolute_time_diff_us(last_frame_time, now) / 1000;

        if (delta_time_ms < 30) {
            sleep_ms(30 - delta_time_ms);
            continue;
        }

        last_frame_time = now;

        lcd::swap();

        buttons::update();

        ui::update(delta_time_ms);
        ui::draw();
    }
}
