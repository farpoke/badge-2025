// C.f. micropython/ports/rp2/mphalport.c

#include "mpconfigport.h"
#include "mphalport.h"

#ifndef NO_QSTR
#include <hardware/timer.h>
#include <pico/aon_timer.h>
#endif

#include <py/runtime.h>

static uint64_t time_us_64_offset_from_epoch;

void mp_hal_delay_ms(mp_uint_t ms) {
    mp_uint_t start = mp_hal_ticks_ms();
    mp_uint_t elapsed = 0;
    do {
        mp_event_wait_ms(ms - elapsed);
        elapsed = mp_hal_ticks_ms() - start;
    } while (elapsed < ms);
}

void mp_hal_delay_us(mp_uint_t us) {
    // Avoid calling sleep_us() and invoking the alarm pool by splitting long
    // sleeps into an optional longer sleep and a shorter busy-wait
    uint64_t end = time_us_64() + us;
    if (us > 1000) {
        mp_hal_delay_ms(us / 1000);
    }
    while (time_us_64() < end) {
        // Tight loop busy-wait for accurate timing
    }
}

uint64_t mp_hal_time_ns(void) {
    return (time_us_64_offset_from_epoch + time_us_64()) * 1000ULL;
}

void mp_hal_time_ns_set_from_rtc(void) {
    // Outstanding RTC register writes need at least two RTC clock cycles to
    // update. (See RP2040 datasheet section 4.8.4 "Reference clock").
    mp_hal_delay_us(44);

    // Sample RTC and time_us_64() as close together as possible, so the offset
    // calculated for the latter can be as accurate as possible.
    struct timespec ts;
    aon_timer_get_time(&ts);
    uint64_t us = time_us_64();

    // Calculate the difference between the RTC Epoch and time_us_64().
    time_us_64_offset_from_epoch = ((uint64_t)ts.tv_sec * 1000000ULL) + ((uint64_t)ts.tv_nsec / 1000ULL) - us;
}

