
#include <badge/lcd.hpp>
#include <mpy/mphalport.h>

#include <lvgl/lvgl.h>

namespace
{
    constexpr auto BUFFER_SIZE = lcd::FRAME_SIZE / 10;

    lv_display_t* _display = nullptr;

    uint8_t* _buffer1 = nullptr;
    uint8_t* _buffer2 = nullptr;

    void send_cmd(lv_display_t*, const uint8_t* cmd, size_t cmd_size, const uint8_t* param, size_t param_size) {
        lcd::internal::select_command();
        lcd::internal::write(cmd, cmd_size);
        if (param_size > 0) {
            lcd::internal::select_data();
            lcd::internal::write(param, param_size);
        }
        lcd::internal::deselect();
    }

    void send_color(lv_display_t*, const uint8_t* cmd, size_t cmd_size, uint8_t* param, size_t param_size) {
        assert(param_size > 0);
        assert((param_size & 1) == 0);

        for (size_t i = 0; i < param_size; i += 2) {
            const auto tmp = param[i];
            param[i] = param[i + 1];
            param[i + 1] = tmp;
        }

        lcd::internal::select_command();
        lcd::internal::write(cmd, cmd_size);
        lcd::internal::select_data();
        lcd::internal::write(param, param_size);
        lcd::internal::deselect();

        lv_display_flush_ready(_display);
    }

}

void setup_lvgl() {

    // Initialize LVGL.
    lv_init();

    // Tell LVGL how to get the current time since boot.
    lv_tick_set_cb(mp_hal_ticks_ms);

    // Initialize the LCD and turn on the backlight.
    lcd::internal::init();
    lcd::internal::exit_sleep();
    lcd::internal::display_on();
    lcd::internal::read_id();
    lcd::internal::read_status();

    lcd::backlight_on(20);

    // Create our LVGL display as an instance of LVGL's ST7789 driver.
    _display = lv_st7789_create(lcd::HEIGHT, lcd::WIDTH, 0, send_cmd, send_color);
    lv_lcd_generic_mipi_set_gap(_display, lcd::internal::COL_OFFSET, lcd::internal::ROW_OFFSET);
    lv_display_set_rotation(_display, LV_DISPLAY_ROTATION_90);
    // lv_lcd_generic_mipi_set_address_mode(_display, true, false, true, false);

    // Allocate and assign draw buffers to our display.
    _buffer1 = new uint8_t[BUFFER_SIZE];
    _buffer2 = new uint8_t[BUFFER_SIZE];
    lv_display_set_buffers(_display, _buffer1, _buffer2, BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Initialize and apply the default theme.
    auto* theme = lv_theme_default_init(
        _display,
        lv_color_hex(0x00'FF'FF),
        lv_color_hex(0xFF'88'00),
        true,
        &lv_font_montserrat_12
    );
    lv_display_set_theme(_display, theme);

}
