#include "lcd.hpp"

#include <cstdio>
#include <cstring>

#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/spi.h>

#include <pico/stdlib.h>

#define WRITE_VALUE(name, value) gpio_put(name, (value) ? 1 : 0)
#define WRITE_HIGH(name)         WRITE_VALUE(name, true)
#define WRITE_LOW(name)          WRITE_VALUE(name, false)

#define READ_VALUE(name) gpio_get(name)

#define LCD_SPI_DELAY()                                                                                                \
    do {                                                                                                               \
        asm volatile("nop");                                                                                           \
    } while (0)

namespace lcd::internal
{

    int txDmaChannel = -1;
    dma_channel_config txDmaConfig = {};

    Pixel *_onScreenFrame = nullptr;
    Pixel *_offScreenFrame = nullptr;

    static uint8_t _frameBufferBlob[FRAME_SIZE * 2];

    bool _inDoomMode = false;

    bool _dmaActive = true;

    void wait_for_spi() {
        if (_dmaActive) {
            dma_channel_wait_for_finish_blocking(txDmaChannel);
            spi_set_format(LCD_SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
            deselect();
            _dmaActive = false;
        }
        while (spi_is_busy(LCD_SPI_PORT))
            asm("nop");
    }

    void dir_out() {
        wait_for_spi();
        WRITE_HIGH(LCD_SPI_DIR_PIN);
    }

    void dir_in() {
        wait_for_spi();
        WRITE_LOW(LCD_SPI_DIR_PIN);
    }

    void select_command() {
        wait_for_spi();
        WRITE_LOW(LCD_DCX_PIN);
        WRITE_LOW(LCD_CS_PIN);
    }

    void select_data() {
        wait_for_spi();
        WRITE_HIGH(LCD_DCX_PIN);
        WRITE_LOW(LCD_CS_PIN);
    }

    void deselect() {
        WRITE_HIGH(LCD_CS_PIN);
    }

    void clock_cycle() {
        wait_for_spi();
        gpio_set_function(LCD_SPI_CLK_PIN, GPIO_FUNC_SIO);
        gpio_set_dir(LCD_SPI_CLK_PIN, true);
        gpio_put(LCD_SPI_CLK_PIN, 1);
        LCD_SPI_DELAY();
        gpio_put(LCD_SPI_CLK_PIN, 0);
        LCD_SPI_DELAY();
        gpio_set_function(LCD_SPI_CLK_PIN, GPIO_FUNC_SPI);
    }

    void write(uint8_t byte) {
        dir_out();
        auto n = spi_write_blocking(LCD_SPI_PORT, &byte, 1);
        if (n != 1) {
            printf("! lcd::write(%02x) wrote %d bytes\n", byte, n);
        }
    }

    void write(const void *data, int n_bytes) {
        dir_out();
        const auto ptr = static_cast<const uint8_t *>(data);
        const int n = spi_write_blocking(LCD_SPI_PORT, ptr, n_bytes);
        if (n != n_bytes) {
            printf("! lcd::write(..., %d) wrote %d bytes\n", n_bytes, n);
        }
    }

    void write_dma16(const void *data, int n_bytes) {
        dir_out();
        spi_set_format(LCD_SPI_PORT, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
        _dmaActive = true;
        dma_channel_configure(txDmaChannel, &txDmaConfig, &spi_get_hw(LCD_SPI_PORT)->dr, data, n_bytes / 2, true);
    }

    void begin_read_sequence() {
        wait_for_spi();
        // Lower baud rate?
    }

    void read(void *buffer, int n_bytes, bool dummy_first) {
        dir_in();
        if (dummy_first)
            clock_cycle();
        const auto ptr = static_cast<uint8_t *>(buffer);
        const int n = spi_read_blocking(LCD_SPI_PORT, 0, ptr, n_bytes);
        if (n != n_bytes) {
            printf("! lcd::read(..., %d, %d) read %d bytes\n", n_bytes, dummy_first, n);
        }
    }

    void end_read_sequence() {
        wait_for_spi();
        // Restore baud rate?
    }

    void simple_cmd(Command cmd) {
        wait_for_spi();
        select_command();
        write(cmd);
        deselect();
    }

    template<typename T = uint8_t>
    void simple_cmd_write(Command cmd, const T &data) {
        wait_for_spi();
        select_command();
        write(cmd);
        select_data();
        write(&data, sizeof(T));
        deselect();
    }

    template<typename T>
    T simple_cmd_read(Command cmd) {
        begin_read_sequence();
        // Send command byte.
        select_command();
        write(cmd);
        // Read data.
        select_data();
        T result = {};
        read(&result, sizeof(result), sizeof(T) > 1);
        // Need extra trailing clock cycle.
        clock_cycle();
        deselect();
        // Extra clock cycle after deselect, to ensure the padding needed by the driver IC.
        clock_cycle();
        end_read_sequence();
        return result;
    }

} // namespace lcd::internal

namespace lcd
{
    using namespace internal;

    void init() {
        printf("> lcd::init ");

        const auto baud = spi_init(LCD_SPI_PORT, SPI_FREQ);
        printf(" (%d.%d MHz) ", baud / 1'000'000, (baud / 100'000) % 10);

        gpio_set_function(LCD_SPI_CLK_PIN, GPIO_FUNC_SPI);
        gpio_set_function(LCD_SPI_OUT_PIN, GPIO_FUNC_SPI);
        gpio_set_function(LCD_SPI_IN_PIN, GPIO_FUNC_SPI);

        uint32_t sio_mask = (1 << LCD_SPI_DIR_PIN) | (1 << LCD_CS_PIN) | (1 << LCD_DCX_PIN) | (1 << LCD_LED_PIN) |
                            (1 << LCD_NRST_PIN);

        gpio_set_function_masked(sio_mask, GPIO_FUNC_SIO);
        gpio_set_dir_out_masked(sio_mask);
        gpio_put_masked(sio_mask, sio_mask);

        txDmaChannel = dma_claim_unused_channel(true);
        txDmaConfig = dma_channel_get_default_config(txDmaChannel);
        channel_config_set_transfer_data_size(&txDmaConfig, DMA_SIZE_16);
        channel_config_set_dreq(&txDmaConfig, spi_get_dreq(LCD_SPI_PORT, true));

        printf("OK\n");

        backlight_off();

        reset();
        exit_sleep();
        display_on();
        read_id();
        read_status();

        backlight_on(20);
    }

    void reset() {
        printf("> lcd::reset ");

        // Put all the pins to default/idle levels.
        WRITE_LOW(LCD_DCX_PIN);
        WRITE_HIGH(LCD_CS_PIN);
        WRITE_HIGH(LCD_LED_PIN);

        // Pull reset low for required duration, then wait for a bit longer.
        WRITE_LOW(LCD_NRST_PIN);
        sleep_ms(1);
        WRITE_HIGH(LCD_NRST_PIN);
        sleep_ms(150);

        // Set driver pixel format to 16-bit color (5 red, 6 green, 5 blue).
        simple_cmd_write(CMD_INTERFACE_PIXEL_FORMAT, 0x55);

        // Tell the LCD driver to flip the y-axis (MY) and swap x and y (MV).
        // This puts the origin in the top left with the screen rotated to "landscape mode".
        simple_cmd_write(CMD_MEMORY_DATA_AC, 0b1010'0000);

        // Set column and row addresses to match display size.
        simple_cmd_write(CMD_COL_ADDRESS, Address(COL_OFFSET, WIDTH + COL_OFFSET - 1));
        simple_cmd_write(CMD_ROW_ADDRESS, Address(ROW_OFFSET, HEIGHT + ROW_OFFSET - 1));

        if (_onScreenFrame == nullptr)
            _onScreenFrame = reinterpret_cast<Pixel *>(_frameBufferBlob);

        if (_offScreenFrame == nullptr)
            _offScreenFrame = reinterpret_cast<Pixel *>(_frameBufferBlob + FRAME_SIZE);

        memset(_onScreenFrame, 0, WIDTH * HEIGHT * sizeof(Pixel));
        memset(_offScreenFrame, 0, WIDTH * HEIGHT * sizeof(Pixel));

        swap();
        wait_for_spi();

        printf("OK\n");
    }

    DisplayID read_id() {
        printf("> Read ID...\n");
        const auto id = simple_cmd_read<DisplayID>(CMD_READ_DISPLAY_ID);
        printf("  Manufacturer ID : %02x\n", id.manufacturer_id);
        printf("  Driver version  : %02x\n", id.driver_version);
        printf("  Driver ID       : %02x\n", id.driver_id);
        printf("\n");
        return id;
    }

    DisplayStatus read_status() {
        printf("> Read Status...\n");
        const auto status = simple_cmd_read<DisplayStatus>(CMD_READ_DISPLAY_STATUS);
        //
        printf("  Display      : %s\n", status.display_on ? "On" : "Off");
        printf("  Booster      : %s\n", status.booster_on ? "On" : "Off");
        printf("  Idle Mode    : %s\n", status.idle_on ? "On" : "Off");
        printf("  Partial Mode : %s\n", status.partial_on ? "On" : "Off");
        printf("  Sleep Mode   : %s\n", status.sleep_out ? "Out" : "In");
        printf("  Display Mode : %s\n", status.normal_on ? "Normal" : "Partial");
        printf("  Inversion    : %s\n", status.inversion_on ? "On" : "Off");
        printf("  Tear Effect  : %s\n", status.tear_on ? "On" : "Off");
        printf("  Tear Mode    : %s\n", status.tear_mode ? "mode1" : "mode2");
        printf("  Gamma Curve  : %d\n", status.gamma_curve);
        //
        printf("  Row Order    : %s\n", status.row_order ? "Bottom to Top" : "Top to Bottom");
        printf("  Col Order    : %s\n", status.col_order ? "Right to Left" : "Left to Right");
        printf("  Exchange     : %s\n", status.row_col_exch ? "Row/Col Exchanged" : "Normal");
        printf("  Scan Order   : %s\n", status.scan_order ? "Bottom to Top" : "Top to Bottom");
        printf("  H. Order     : %s\n", status.horiz_order ? "Right to Left" : "Left to Right");
        //
        printf("  RGB Order    : %s\n", status.rgb_order ? "BGR" : "RGB");
        printf("  Pixel Fmt    : %s\n",
               status.if_pixel_fmt == IPF_12BPP_4_4_4   ? "4-4-4"
               : status.if_pixel_fmt == IPF_16BPP_5_6_5 ? "5-6-5"
               : status.if_pixel_fmt == IPF_18BPP_6_6_6 ? "6-6-6"
                                                        : "Invalid");
        printf("\n");
        return status;
    }

    void enter_sleep() {
        printf("> Enter Sleep...\n");
        simple_cmd(CMD_MODE_SLEEP_IN);
        sleep_ms(5);
    }

    void exit_sleep() {
        printf("> Exit Sleep...\n");
        simple_cmd(CMD_MODE_SLEEP_OUT);
        sleep_ms(5);
    }

    void display_on() {
        printf("> Display On...\n");
        simple_cmd(CMD_DISPLAY_ON);
        sleep_ms(5);
    }

    void display_off() {
        printf("> Display Off...\n");
        simple_cmd(CMD_DISPLAY_OFF);
        sleep_ms(5);
    }

    void set_idle_mode(bool enabled) {
        printf("> Idle Mode = %s\n", enabled ? "On" : "Off");
        simple_cmd(enabled ? CMD_IDLE_ON : CMD_IDLE_OFF);
    }

    void set_inversion(bool enabled) {
        printf("> Inversion = %s\n", enabled ? "On" : "Off");
        simple_cmd(enabled ? CMD_INVERT_ON : CMD_INVERT_OFF);
    }

    void set_gamma(int idx) {
        printf("> Gamma Curve = %d\n", idx);
        simple_cmd_write<uint8_t>(CMD_GAMMA_SET, 1 << (idx & 3));
    }

    void backlight_on(int pct) {
        printf("> Backlight On %d%%...\n", pct);
        if (pct <= 0) {
            backlight_off();
        }
        else if (pct >= 100) {
            auto pwm_slice = pwm_gpio_to_slice_num(LCD_LED_PIN);
            pwm_set_enabled(pwm_slice, false);
            gpio_set_function(LCD_LED_PIN, GPIO_FUNC_SIO);
            gpio_put(LCD_LED_PIN, false);
        }
        else {
            gpio_set_function(LCD_LED_PIN, GPIO_FUNC_PWM);
            auto pwm_slice = pwm_gpio_to_slice_num(LCD_LED_PIN);
            pwm_set_wrap(pwm_slice, 99);
            pwm_set_enabled(pwm_slice, true);
            pwm_set_gpio_level(LCD_LED_PIN, 100 - pct);
        }
    }

    void backlight_off() {
        printf("> Backlight Off...\n");
        auto pwm_slice = pwm_gpio_to_slice_num(LCD_LED_PIN);
        pwm_set_enabled(pwm_slice, false);
        gpio_set_function(LCD_LED_PIN, GPIO_FUNC_SIO);
        gpio_put(LCD_LED_PIN, true);
    }

    Pixel *get_offscreen_ptr_unsafe() { return _offScreenFrame; }

    void swap() {
        const auto tmp = _onScreenFrame;
        _onScreenFrame = _offScreenFrame;
        _offScreenFrame = tmp;

        wait_for_spi();
        select_command();
        write(CMD_MEMORY_WRITE);
        select_data();
        write_dma16(_onScreenFrame, sizeof(Pixel) * WIDTH * HEIGHT);
    }

} // namespace lcd

extern "C" uint8_t *lcd_change_to_doom_mode() {
    // The DOOM engine wants a bunch of memory for screen buffers. So to let it have that we change how we render
    // to the LCD (DOOM uses an 8-bit palette instead of 16-bit color buffer) and let the DOOM engine have our
    // frame buffer blob.
    lcd::internal::_inDoomMode = true;
    return lcd::internal::_frameBufferBlob;
}
