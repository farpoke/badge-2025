#include <py/mphal.h>
#include <shared/runtime/softtimer.h>
#include "pendsv.h"

// C.f. micropython/ports/rp2/mphalport.c

uint32_t soft_timer_get_ms(void) {
    return mp_hal_ticks_ms();
}

void soft_timer_schedule_at_ms(uint32_t ticks_ms) {
    int32_t ms = soft_timer_ticks_diff(ticks_ms, mp_hal_ticks_ms());
    ms = MAX(0, ms);
    if (hardware_alarm_set_target(MICROPY_HW_SOFT_TIMER_ALARM_NUM, delayed_by_ms(get_absolute_time(), ms))) {
        // "missed" hardware alarm target
        hardware_alarm_force_irq(MICROPY_HW_SOFT_TIMER_ALARM_NUM);
    }
}

static void soft_timer_hardware_callback(unsigned int) {
    pendsv_schedule_softtimer(soft_timer_handler);
}

void soft_timer_init(void) {
    hardware_alarm_claim(MICROPY_HW_SOFT_TIMER_ALARM_NUM);
    hardware_alarm_set_callback(MICROPY_HW_SOFT_TIMER_ALARM_NUM, soft_timer_hardware_callback);
}
