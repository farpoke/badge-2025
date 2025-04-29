#include <cmath>
#include <cstdio>

#include <pico/stdlib.h>
#include <pico/time.h>

#include <tusb.h>

#include "board/buttons.hpp"
#include "board/lcd.hpp"
#include "core/core1.hpp"
#include "drawing/font.hpp"
#include "gfx/image.hpp"
#include "mpy/mpy.hpp"
#include "usb/usb.hpp"

static constexpr auto TABLE_SIZE_POWER = 10;
static constexpr auto TABLE_SIZE = 1 << TABLE_SIZE_POWER;
static constexpr auto TABLE_SIZE_MASK = TABLE_SIZE - 1;

uint8_t sin_table_data[TABLE_SIZE];

void init_sin_table()
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        auto x = (float)i / TABLE_SIZE * 3.14159265f * 2;
        auto y = (sin(x) + 1) / 2;
        y *= y;
        int z = (int)(y * 255);
        if (z < 0)
            z = 0;
        if (z > 255)
            z = 255;
        sin_table_data[i] = (uint8_t)z;
    }
}

uint8_t sin_table(int i)
{
    return sin_table_data[i & TABLE_SIZE_MASK];
}

lcd::Pixel color(int x, int y, int t)
{
    auto r = sin_table(x * 2 + y * 4 + t * 2);
    auto g = sin_table(x * 4 + y * 2 - t * 4);
    auto b = sin_table(x * 2 - y * 4 + t * 6);
    return lcd::to_pixel(r, g, b);
}

void draw_frame()
{
    static int frame = 0;
    frame++;

    auto* ptr = lcd::get_offscreen_ptr_unsafe();

    for (size_t row = 0; row < lcd::HEIGHT; row++)
        for (size_t col = 0; col < lcd::WIDTH; col++)
            *ptr++ = color(row, col, frame);
}

[[noreturn]] int main()
{
    stdio_init_all();
    printf("\n===== lcd-test =====\n");

    usb::init();
    buttons::init();
    mpy::init();

    init_sin_table();

    stdio_flush();

    core1::reset_and_launch();

    lcd::copy(0, lcd::WIDTH - 1, 0, lcd::HEIGHT - 1, reinterpret_cast<const lcd::Pixel*>(IMAGE_DATA));
    core1::swap_frame();

    printf("> Splash screen wait...\n");
    const auto start = get_absolute_time();
    while (absolute_time_diff_us(start, get_absolute_time()) < 1'000'000) {
        tud_task();
        tight_loop_contents();
    }

    // mpy::repl();

    printf("> Main loop...\n");
    bool active = true;
    while(true) {

        tud_task();
        
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

        draw_frame();

        auto x = 3;
        auto y = 17;

        font::lucida.draw(x, y, "\"lucida\" Hello world!");
        y += 22;
        font::m5x7.draw(x, y, "\"m5x7\" Hello world!");
        y += 18;
        font::m6x11.draw(x, y, "\"m6x11\" Hello world!");
        y += 21;
        font::noto_sans.draw(x, y, "\"Noto Sans\" Hello world!");
        y += 22;
        font::noto_sans_cm.draw(x, y, "\"Noto Sans Condensed Medium\" Hello world!");

        core1::swap_frame();
    }
}
