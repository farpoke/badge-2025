# This is mainly taken from micropython's rp2 port.
# See micropython/ports/rp2/CMakeLists.txt

# Use this directory as the base directory of our micropython port.
set(MICROPY_PORT_DIR ${CMAKE_CURRENT_LIST_DIR})

# Tell micropython where we put our port-specific strings.
set(MICROPY_QSTRDEFS_PORT "${MICROPY_PORT_DIR}/qstrdefsport.h")

# Collect a list of sources for our micropython port:
set(MICROPY_SOURCE_PORT
        "${MICROPY_PORT_DIR}/mpy.c"
        "${MICROPY_PORT_DIR}/mpy.cpp"
)

# Collect a list of micropython library sources:
set(MICROPY_SOURCE_LIB
        # ${MICROPY_DIR}/lib/littlefs/lfs1.c
        # ${MICROPY_DIR}/lib/littlefs/lfs1_util.c
        # ${MICROPY_DIR}/lib/littlefs/lfs2.c
        # ${MICROPY_DIR}/lib/littlefs/lfs2_util.c
        # ${MICROPY_DIR}/lib/oofatfs/ff.c
        # ${MICROPY_DIR}/lib/oofatfs/ffunicode.c
        ${MICROPY_DIR}/shared/readline/readline.c
        ${MICROPY_DIR}/shared/runtime/gchelper_native.c
        ${MICROPY_DIR}/shared/runtime/gchelper_thumb1.s
        ${MICROPY_DIR}/shared/runtime/interrupt_char.c
        ${MICROPY_DIR}/shared/runtime/mpirq.c
        ${MICROPY_DIR}/shared/runtime/pyexec.c
        ${MICROPY_DIR}/shared/runtime/stdout_helpers.c
        # ${MICROPY_DIR}/shared/runtime/softtimer.c
        ${MICROPY_DIR}/shared/runtime/sys_stdio_mphal.c
        ${MICROPY_DIR}/shared/timeutils/timeutils.c
)

# Collect a list of sources for string interning:
set(MICROPY_SOURCE_QSTR
        ${MICROPY_SOURCE_PY}
        ${MICROPY_SOURCE_EXTMOD}
        ${MICROPY_SOURCE_USERMOD}
        ${MICROPY_SOURCE_LIB}
        ${MICROPY_SOURCE_PORT}
)

# Tell the pico SDK we want to use the micropython float implementation.
pico_set_float_implementation(${MICROPY_TARGET} micropython)

# Create our custom component library with sources for said float implementation.
pico_add_library(pico_float_micropython)
target_sources(pico_float_micropython INTERFACE
        ${MICROPY_SOURCE_LIB_LIBM}
        ${MICROPY_SOURCE_LIB_LIBM_SQRT_SW}
        ${MICROPY_DIR}/ports/rp2/libm_extra.c
        ${PICO_SDK_PATH}/src/rp2_common/pico_float/float_aeabi_rp2040.S
        ${PICO_SDK_PATH}/src/rp2_common/pico_float/float_init_rom_rp2040.c
        ${PICO_SDK_PATH}/src/rp2_common/pico_float/float_v1_rom_shim_rp2040.S
)

# Wrap low-level floating-point operations, to call the pico-sdk versions.
pico_wrap_function(pico_float_micropython __aeabi_fdiv)
pico_wrap_function(pico_float_micropython __aeabi_fmul)
pico_wrap_function(pico_float_micropython __aeabi_frsub)
pico_wrap_function(pico_float_micropython __aeabi_fsub)
pico_wrap_function(pico_float_micropython __aeabi_cfcmpeq)
pico_wrap_function(pico_float_micropython __aeabi_cfrcmple)
pico_wrap_function(pico_float_micropython __aeabi_cfcmple)
pico_wrap_function(pico_float_micropython __aeabi_fcmpeq)
pico_wrap_function(pico_float_micropython __aeabi_fcmplt)
pico_wrap_function(pico_float_micropython __aeabi_fcmple)
pico_wrap_function(pico_float_micropython __aeabi_fcmpge)
pico_wrap_function(pico_float_micropython __aeabi_fcmpgt)
pico_wrap_function(pico_float_micropython __aeabi_fcmpun)
pico_wrap_function(pico_float_micropython __aeabi_i2f)
pico_wrap_function(pico_float_micropython __aeabi_l2f)
pico_wrap_function(pico_float_micropython __aeabi_ui2f)
pico_wrap_function(pico_float_micropython __aeabi_ul2f)
pico_wrap_function(pico_float_micropython __aeabi_f2iz)
pico_wrap_function(pico_float_micropython __aeabi_f2lz)
pico_wrap_function(pico_float_micropython __aeabi_f2uiz)
pico_wrap_function(pico_float_micropython __aeabi_f2ulz)
pico_wrap_function(pico_float_micropython __aeabi_f2d)

# Add collected micropy sources, libraries, and include directories to our target.
target_sources(${MICROPY_TARGET} PRIVATE
        ${MICROPY_SOURCE_PY}
        ${MICROPY_SOURCE_EXTMOD}
        ${MICROPY_SOURCE_LIB}
        ${MICROPY_SOURCE_PORT}
)
target_link_libraries(${MICROPY_TARGET} usermod)
target_include_directories(${MICROPY_TARGET} PRIVATE
        ${MICROPY_INC_CORE}
        ${MICROPY_INC_USERMOD}
        ${MICROPY_PORT_DIR}
        "${CMAKE_BINARY_DIR}"
)

# Apply optimisations to performance-critical source code.
set_source_files_properties(
            ${MICROPY_PY_DIR}/map.c
            ${MICROPY_PY_DIR}/mpz.c
            ${MICROPY_PY_DIR}/vm.c
        PROPERTIES
            COMPILE_OPTIONS "-O2"
)

# Tell the compiler to ignore stack usage for select units.
set_source_files_properties(
            ${MICROPY_PY_DIR}/compile.c
            ${MICROPY_PY_DIR}/objboundmeth.c
            ${MICROPY_PY_DIR}/objfun.c
        PROPERTIES
            COMPILE_OPTIONS "-Wno-stack-usage"
)
