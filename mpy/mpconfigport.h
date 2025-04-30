#pragma once

#include <stdint.h>

#define MICROPY_BANNER_MACHINE                 "HackGBGay Badge 2025"
#define MICROPY_CONFIG_ROM_LEVEL               (MICROPY_CONFIG_ROM_LEVEL_CORE_FEATURES)
#define MICROPY_ENABLE_COMPILER                (1)
#define MICROPY_ENABLE_GC                      (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF (1)
#define MICROPY_STACK_CHECK_MARGIN             (256)
#define MICROPY_LONGINT_IMPL                   (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL                     (MICROPY_FLOAT_IMPL_FLOAT)
#define MICROPY_SCHEDULER_DEPTH                (8)
#define MICROPY_SCHEDULER_STATIC_NODES         (1)
#define MICROPY_USE_INTERNAL_ERRNO             (1)
#define MICROPY_ERROR_REPORTING                (MICROPY_ERROR_REPORTING_NONE)
#define MICROPY_HEAP_SIZE                      (4096)
#define MICROPY_HELPER_REPL                    (1)
#define MICROPY_MIN_USE_CORTEX_CPU             (1)
#define MICROPY_MIN_USE_STM32_MCU              (1)
#define MICROPY_PY_THREAD                      (0)
#define MICROPY_PY_THREAD_GIL                  (0)
#define MICROPY_HW_SOFT_TIMER_ALARM_NUM        (2)
#define MICROPY_MAKE_POINTER_CALLABLE(p)       ((void *)((mp_uint_t)(p) | 1))
#define MICROPY_PY_SYS_ATTR_DELEGATION         (1)

#define MP_SSIZE_MAX (0x7fffffff)

typedef int32_t  mp_int_t;  // must be pointer size
typedef uint32_t mp_uint_t; // must be pointer size
typedef long     mp_off_t;

// Need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MP_STATE_PORT MP_STATE_VM
