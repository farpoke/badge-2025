#include <cmath>
#include <cstdio>

#include <pico/stdlib.h>
#include <pico/time.h>

#include <tusb.h>

#include "buttons.hpp"
#include "font.hpp"
#include "lcd.hpp"
#include "usb/usb.hpp"
#include "mpy/mpy.hpp"
#include "gfx/image.hpp"

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

    auto ptr = lcd::frame_ptr();
    for (size_t row = 0; row < lcd::HEIGHT; row++)
        for (size_t col = 0; col < lcd::WIDTH; col++)
            *ptr++ = color(row, col, frame);
}

[[noreturn]] void __attribute__ ((naked)) isr_hardfault()
{
    uint32_t sp, lr, ipsr;
    asm volatile("mov %0, sp" : "=r"(sp));
    asm volatile("mov %0, lr" : "=r"(lr));
    asm volatile("mrs %0, ipsr" : "=r"(ipsr));

    const auto *stack = reinterpret_cast<uint32_t *>(sp);
    const auto r0 = stack[0];
    const auto r1 = stack[1];
    const auto r2 = stack[2];
    const auto r3 = stack[3];
    const auto r12 = stack[4];
    const auto old_lr = stack[5];
    const auto ret_addr = stack[6];
    const auto xspr = stack[7];

    const auto cpuid = *reinterpret_cast<uint32_t *>(0xD0000000);
    const auto icsr = scb_hw->icsr;
    printf(
        "\n"
        "\n"
        "! CORE %d HARDFAULT  !\n"
        "! ICSR = 0x%08x !\n"
        "! SP   = 0x%08x !\n"
        "! LR'  = 0x%08x !\n"
        "! R0   = 0x%08x !\n"
        "! R1   = 0x%08x !\n"
        "! R2   = 0x%08x !\n"
        "! R3   = 0x%08x !\n"
        "! R12  = 0x%08x !\n"
        "! LR   = 0x%08x !\n"
        "! Ret. = 0x%08x !\n"
        "! xPSR = 0x%08x !\n"
        "! IPSR = 0x%08x !\n"
        "\n"
        "\n",
        cpuid, icsr,
        sp, lr,
        r0, r1, r2, r3, r12, old_lr, ret_addr, xspr,
        ipsr
    );

    while (true) tight_loop_contents();
}

[[noreturn]] int main()
{
    stdio_init_all();
    printf("\n===== lcd-test =====\n");

    usb::init();
    buttons::init();
    mpy::init();

    init_sin_table();

    lcd::init();
    lcd::exit_sleep();
    lcd::display_on();
    lcd::read_id();
    lcd::read_status();

    lcd::begin_frame();
    lcd::write_frame(reinterpret_cast<const lcd::Pixel*>(IMAGE_DATA), lcd::WIDTH * lcd::HEIGHT);

    lcd::backlight_on(20);

    for (int i = 0; i < 1000; i++) {
        tud_task();
        sleep_ms(1);
    }

    mpy::repl();

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

        lcd::begin_swap();
        lcd::end_swap();

        // sleep_ms(10);
    }
}
