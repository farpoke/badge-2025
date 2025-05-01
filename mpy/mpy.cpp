#include "mpy.hpp"

#ifndef NO_QSTR
#include <tusb.h>
#endif

extern "C" {

#include <py/gc.h>
#include <py/runtime.h>
#include <py/stackctrl.h>

#include <shared/runtime/pyexec.h>

    void init_time(void);

    uint8_t __StackTop; // NOLINT(*-reserved-identifier)
    uint8_t __StackBottom; // NOLINT(*-reserved-identifier)

}

namespace mpy
{

    static char _heap[MICROPY_HEAP_SIZE];

    void init() {
        init_time();
        mp_cstack_init_with_top(&__StackTop, &__StackTop - &__StackBottom);
        gc_init(_heap, _heap + sizeof(_heap));
        mp_init();
    }

    void repl() {
        pyexec_friendly_repl();
    }

}
