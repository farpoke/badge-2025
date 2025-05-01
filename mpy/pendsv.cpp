// C.f. micropython/ports/rp2/pendsv.c

#include "pendsv.h"

#ifndef NO_QSTR
#include <hardware/irq.h>
#endif

namespace
{

    int _lock = 0;

    pendsv_callback_t _softtimer_callback = nullptr;

    void maybe_run(pendsv_callback_t &callback) {
        if (callback) {
            const auto f = callback;
            callback = nullptr;
            f();
        }
    }

    void maybe_run_all() {
        maybe_run(_softtimer_callback);
    }

    bool any_pending() {
        if (_softtimer_callback) return true;
        return false;
    }

    void set_pendsv_flag() {
        scb_hw->icsr |= 1 << 28;
    }

}

void pendsv_suspend() {
    _lock++;
}

void pendsv_resume() {
    assert(_lock > 0);
    _lock--;
    if (_lock == 0 && any_pending())
        set_pendsv_flag();
}

void pendsv_schedule_softtimer(pendsv_callback_t callback) {
    assert(_softtimer_callback == nullptr || _softtimer_callback == callback);
    _softtimer_callback = callback;
    if (_lock == 0)
        set_pendsv_flag();
}

void pendsv_handle_interrupt() {
    assert(_lock == 0);
    maybe_run_all();
}
