#pragma once

// C.f. micropython/ports/rp2/mphalport.h

#ifndef NO_QSTR
#include <hardware/timer.h>
#include <pico/time.h>

#include "pendsv.h"
#endif

#define MICROPY_PY_PENDSV_ENTER pendsv_suspend()
#define MICROPY_PY_PENDSV_EXIT  pendsv_resume()

#if __cplusplus
extern "C" {
#endif

void mp_hal_time_ns_set_from_rtc(void);

#if __cplusplus
}
#endif

static inline void mp_hal_set_interrupt_char(char c) {}

static inline mp_uint_t mp_hal_ticks_cpu(void) { return time_us_32(); }
static inline mp_uint_t mp_hal_ticks_us(void) { return time_us_32(); }
static inline mp_uint_t mp_hal_ticks_ms(void) { return to_ms_since_boot(get_absolute_time()); }
