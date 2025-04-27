#include "mpy.hpp"

#ifndef NO_QSTR
#include <pico.h>
#include <tusb.h>
#endif

extern "C" {

#include <py/gc.h>
#include <py/runtime.h>
#include <py/stackctrl.h>

}

namespace mpy
{

    static char _heap[MICROPY_HEAP_SIZE];

    void init() {
        mp_stack_ctrl_init();
        gc_init(_heap, _heap + sizeof(_heap));
        mp_init();
    }

}
