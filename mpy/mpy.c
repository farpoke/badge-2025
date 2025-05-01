
#include "mpconfigport.h"
#include "mphalport.h"

#include <time.h>

#include <py/gc.h>
#include <py/lexer.h>
#include <py/mperrno.h>
#include <py/runtime.h>

#include <shared/runtime/gchelper.h>

#ifndef NO_QSTR
#include <pico.h>
#include <tusb.h>
#include <pico/aon_timer.h>
#include <shared/timeutils/timeutils.h>
#endif

void init_time(void) {
    // Start and initialise the RTC
    struct timespec ts = { 0, 0 };
    ts.tv_sec = timeutils_seconds_since_epoch(2025, 1, 1, 0, 0, 0);
    aon_timer_start(&ts);
    mp_hal_time_ns_set_from_rtc();
}

void gc_collect(void) {
    gc_collect_start();
    gc_helper_collect_regs_and_stack();
    gc_collect_end();
}

void nlr_jump_fail(void *val) {
    mp_printf(&mp_plat_print, "FATAL: uncaught exception %p\n", val);
    mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(val));
    for (;;) {
        __breakpoint();
    }
}

mp_lexer_t *mp_lexer_new_from_file(qstr filename) {
    mp_raise_OSError(MP_ENOENT);
}
