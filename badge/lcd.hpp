#pragma once

#include <cstdint>

namespace lcd
{

    constexpr int SPI_FREQ = 10'000'000;

    constexpr int WIDTH = 160;
    constexpr int HEIGHT = 128;

    constexpr int PIXEL_SIZE = 2;
    constexpr auto FRAME_SIZE = WIDTH * HEIGHT * PIXEL_SIZE;

    namespace internal
    {

        constexpr int COL_OFFSET = 1;
        constexpr int ROW_OFFSET = 2;

        enum Command : uint8_t
        {
            CMD_NOP = 0x00,

            CMD_SOFTWARE_RESET = 0x01,

            CMD_READ_DISPLAY_ID = 0x04,
            CMD_READ_DISPLAY_STATUS = 0x09,
            CMD_READ_DISPLAY_POWER = 0x0A,
            CMD_READ_DISPLAY = 0x0B,
            CMD_READ_DISPLAY_PIXEL = 0x0C,
            CMD_READ_DISPLAY_IMAGE = 0x0D,
            CMD_READ_DISPLAY_SIGNAL = 0x0E,
            CMD_READ_DISPLAY_DIAG = 0x0F,

            CMD_MODE_SLEEP_IN = 0x10,
            CMD_MODE_SLEEP_OUT = 0x11,
            CMD_MODE_PARTIAL = 0x12,
            CMD_MODE_NORMAL = 0x13,

            CMD_INVERT_OFF = 0x20,
            CMD_INVERT_ON = 0x21,

            CMD_GAMMA_SET = 0x26,

            CMD_DISPLAY_OFF = 0x28,
            CMD_DISPLAY_ON = 0x29,

            CMD_IDLE_OFF = 0x38,
            CMD_IDLE_ON = 0x39,

            CMD_MEMORY_WRITE = 0x2C,
            CMD_MEMORY_READ = 0x2E,
            CMD_MEMORY_DATA_AC = 0x36,
            CMD_MEMORY_WRITE_CONTINUE = 0x3C,
            CMD_MEMORY_READ_CONTINUE = 0x3E,

            CMD_COL_ADDRESS = 0x2A,
            CMD_ROW_ADDRESS = 0x2B,
            CMD_PARTIAL_ADDRESS = 0x30,

            CMD_VSCROLL_DEFINITION = 0x33,
            CMD_VSCROLL_START_ADDR = 0x37,

            CMD_TEAR_EFFECT_LINE_OFF = 0x34,
            CMD_TEAR_EFFECT_LINE_ON = 0x35,
            CMD_TEAR_SET_SCANLINE = 0x44,

            CMD_GET_SCANLINE = 0x45,

            CMD_INTERFACE_PIXEL_FORMAT = 0x3A,

            CMD_BRIGHTNESS_WRITE = 0x51,
            CMD_BRIGHTNESS_READ = 0x52,

            CMD_CTRL_WRITE = 0x53,
            CMD_CTRL_READ = 0x54,
        };

        void select_command();
        void select_data();
        void deselect();

        void write(uint8_t byte);
        void write(const void *data, int n_bytes);

        void read(void *buffer, int n_bytes);

    }

    enum InterfacePixelFormat
    {
        IPF_12BPP_4_4_4 = 0b011,
        IPF_16BPP_5_6_5 = 0b101,
        IPF_18BPP_6_6_6 = 0b110,
    };

#pragma pack(push, 1)

    struct DisplayID
    {
        uint8_t manufacturer_id;
        uint8_t driver_version;
        uint8_t driver_id;
    };

    struct DisplayStatus
    {
        bool : 1; ///< B1 D0: Reserved/unused.

        bool horiz_order : 1;  ///< B1 D1: Horizontal order (MH). 0=decrement (refresh left to right), 1=increment (refresh right to left).
        bool rgb_order : 1;    ///< B1 D2: RGB order (RGB). 0=RGB, 1=BGR.
        bool scan_order : 1;   ///< B1 D3: Scan order (ML). 0=decrement (refresh top to bottom), 1=increment (refresh bottom to top).
        bool row_col_exch : 1; ///< B1 D4: Row/Column exchange (MV). 0=normal, 1=row/column exchanged.
        bool col_order : 1;    ///< B1 D5: Column order (MX). 0=increment(left to right), 1=decrement (right to left).
        bool row_order : 1;    ///< B1 D6: Row order (MY). 0=increment (top to bottom), 1=decrement (bottom to top).
        bool booster_on : 1;   ///< B1 D7: Voltage booster on/off (BSTON).

        bool normal_on : 1;  ///< B2 D0: Normal/partial display mode (NORON). 0=partial, 1=normal.
        bool sleep_out : 1;  ///< B2 D1: Sleep mode (SLPOUT). 0=in, 1=out.
        bool partial_on : 1; ///< B2 D2: Partial mode on/off (PTLON).
        bool idle_on : 1;    ///< B2 D3: Idle mode on/off (IDMON).

        InterfacePixelFormat if_pixel_fmt : 3; ///< B2 D4:6: Interface color pixel format (IFPF).

        bool : 1; ///< B2 D7: Reserved/unused.

        bool _gcsel2 : 1;      ///< B3 D0: Gamma curve selection bit 2 (GCSEL2, not used).
        bool tear_on : 1;      ///< B3 D1: Tearing effect on/off (TEON).
        bool display_on : 1;   ///< B3 D2: Display on/off (DISON).
        bool _all_off : 1;     ///< B3 D3: All pixels off (ST11, not used).
        bool _all_on : 1;      ///< B3 D4: All pixels on (ST12, not used).
        bool inversion_on : 1; ///< B3 D5: Inversion mode on/off (INVON).
        bool _hscroll_on : 1;  ///< B3 D6: Horizontal scrolling on/off (ST14, not used).
        bool _vscroll_on : 1;  ///< B3 D7: Vertical scrolling on/off (ST15, not used).

        uint8_t : 5; /// B4 D0:4: Reserved/unused.

        bool tear_mode : 1; ///< B4 D5: Tear effect line mode (TEM). 0=mode1, 1=mode2.

        uint8_t gamma_curve : 2; ///< B4 D6:7: Gamma curve selection (GCSEL0:1).
    };

    struct Address
    {
        uint8_t start_high;
        uint8_t start_low;
        uint8_t end_high;
        uint8_t end_low;

        Address() = default;

        Address(uint16_t start, uint16_t end)
            : start_high(start >> 8), start_low(start & 0xFF), end_high(end >> 8), end_low(end & 0xFF)
        {
        }
    };

    struct VScrollDefinition {
        uint16_t top_fixed;
        uint16_t vscroll;
        uint16_t bottom_fixed;
    };

#pragma pack(pop)

    static_assert(sizeof(DisplayID) == 3);
    static_assert(sizeof(DisplayStatus) == 4);
    static_assert(sizeof(Address) == 4);

    using Pixel = uint16_t;

    constexpr Pixel to_pixel(uint32_t rgb)
    {
        // 8-8-8 : RRRR Rrrr GGGG GGgg BBBB Bbbb
        // 5-6-5 :           RRRR RGGG GGGB BBBB
        const uint16_t rgb565 = ((rgb & 0xF80000) >> 8) | ((rgb & 0x00F800) >> 5) | ((rgb & 0x0000F8) >> 3);
        // System is little-endian, so swap bytes to make the data sent to the LCD driver correct.
        const uint16_t swapped = (rgb565 >> 8) | (rgb565 << 8);
        return swapped;
    }

    constexpr Pixel to_pixel(uint8_t r, uint8_t g, uint8_t b)
    {
        return to_pixel(r << 16 | g << 8 | b);
    }

    constexpr Pixel from_grayscale(uint8_t gray) {
        return to_pixel(gray, gray, gray);
    }

    namespace internal
    {

        void init();
        void reset();

        DisplayID read_id();
        DisplayStatus read_status();

        void enter_sleep();
        void exit_sleep();

        void display_on();
        void display_off();

        void set_idle_mode(bool enabled);
        void set_inversion(bool enabled);
        void set_gamma(int idx);

        void begin_swap();
        void end_swap();

    }

    void backlight_on(int pct);
    void backlight_off();

    Pixel* get_offscreen_ptr_unsafe();

}
