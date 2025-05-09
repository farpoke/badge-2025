
#include <pico/stdio/driver.h>

#include <tusb.h>

namespace
{
    void out_chars(const char* buf, int len) {
        if (tud_connected()) {
            tud_cdc_write(buf, len);
            tud_cdc_write_flush();
            while (tud_task_event_ready())
                tud_task();
        }
    }
    void out_flush() {
        if (tud_connected())
            tud_cdc_write_flush();
    }
}

stdio_driver usb_stdio_driver = {
    .out_chars = out_chars,
    .out_flush = out_flush,
    .crlf_enabled = true,
};

void enable_stdio_to_usb() {
    stdio_set_driver_enabled(&usb_stdio_driver, true);
}
