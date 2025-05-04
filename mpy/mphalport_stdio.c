#include "mpconfigport.h"
#include "mphalport.h"

#ifndef NO_QSTR
#include <pico.h>
#include <tusb.h>
#include <lvgl/lvgl.h>
#endif

#include <py/stream.h>

#include "py/runtime.h"

uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) {
    uintptr_t ret = 0;
    if ((poll_flags & MP_STREAM_POLL_RD) && tud_cdc_available())
        ret |= MP_STREAM_POLL_RD;
    if ((poll_flags & MP_STREAM_POLL_WR) && tud_connected() && tud_cdc_write_available())
        ret |= MP_STREAM_POLL_WR;
    return ret;
}

int mp_hal_stdin_rx_chr() {
    int ch = -1;
    while (ch == -1) {
        while (tud_task_event_ready())
            tud_task();
        lv_timer_handler();
        mp_handle_pending(true);
        ch = tud_cdc_read_char();
    }
    return ch;
}

mp_uint_t mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    tud_cdc_write(str, len);
    tud_cdc_write_flush();
    return len;
}


