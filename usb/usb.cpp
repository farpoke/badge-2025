#include "usb.hpp"

#include <tusb.h>

namespace usb
{

    void init() {
        printf("> Init USB ... ");

        constexpr tusb_rhport_init_t dev_init = {
            .role = TUSB_ROLE_DEVICE,
            .speed = TUSB_SPEED_AUTO,
        };
        tusb_init(0, &dev_init);

        printf("OK\n");
    }

    void write(const char *text) {
        for (int i = 0; i < CFG_TUD_CDC_TX_BUFSIZE && text[i] != 0; i++) {
            if (text[i] == '\n' && (i == 0 || text[i-1] != '\r')) {
                tud_cdc_write_char('\r');
            }
            tud_cdc_write_char(text[i]);
            if (text[i] == '\n')
                tud_cdc_write_flush();
        }
    }

}


