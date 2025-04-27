#pragma once

#include <stdint.h>

#define MICROPY_BANNER_MACHINE      "HackGBGay Badge 2025"
#define MICROPY_CONFIG_ROM_LEVEL    (MICROPY_CONFIG_ROM_LEVEL_MINIMUM)
#define MICROPY_ENABLE_COMPILER     (1)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_NONE)
#define MICROPY_HEAP_SIZE           (4096)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_MIN_USE_CORTEX_CPU  (1)
#define MICROPY_MIN_USE_STM32_MCU   (1)

typedef int32_t mp_int_t; // must be pointer size
typedef uint32_t mp_uint_t; // must be pointer size
typedef long mp_off_t;

// Need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MP_STATE_PORT MP_STATE_VM
