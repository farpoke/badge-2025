#include <py/gc.h>
#include <py/runtime.h>
#include <shared/runtime/gchelper.h>

#ifndef NO_QSTR
#include <pico.h>
#include <tusb.h>
#endif

void gc_collect(void) {
    gc_collect_start();
    gc_helper_collect_regs_and_stack();
#if MICROPY_PY_THREAD
    mp_thread_gc_others();
#endif
    gc_collect_end();
}

void nlr_jump_fail(void *val) {
    mp_printf(&mp_plat_print, "FATAL: uncaught exception %p\n", val);
    mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(val));
    for (;;) {
        __breakpoint();
    }
}

int __attribute__((used)) mp_hal_stdin_rx_chr() {
    return tud_cdc_read_char();
}

mp_uint_t __attribute__((used)) mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    tud_cdc_write(str, len);
    tud_cdc_write_flush();
    return len;
}
