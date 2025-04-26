# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.31)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(MICROPY_DIR "$ENV{MICROPY_DIR}" CACHE PATH "Path to micropython repository")
set(MICROPY_TARGET badge-2025)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_SOURCE_DIR})
set(PICO_BOARD badge-2025 CACHE STRING "Board type")

include(${MICROPY_DIR}/py/py.cmake)
include(${MICROPY_DIR}/extmod/extmod.cmake)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(${MICROPY_TARGET} C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(${MICROPY_TARGET}
        main.cpp
        buttons.cpp
        lcd.cpp
        font.cpp
        usb/usb.cpp
        usb/usb-descriptors.cpp
)

pico_set_program_name(${MICROPY_TARGET} "badge-2025")
pico_set_program_version(${MICROPY_TARGET} "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${MICROPY_TARGET} 1)
pico_enable_stdio_usb(${MICROPY_TARGET} 0)

# Add the standard library to the build
target_link_libraries(${MICROPY_TARGET}
        pico_stdlib
        pico_bootsel_via_double_reset
        pico_time
)

# Add the standard include files to the build
target_include_directories(${MICROPY_TARGET} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(${MICROPY_TARGET}
        hardware_spi
        hardware_pwm
        tinyusb_device
)

add_subdirectory(fonts)
add_subdirectory(gfx)
add_subdirectory(filesystem)

target_link_libraries(${MICROPY_TARGET} fonts gfx filesystem)

pico_add_extra_outputs(${MICROPY_TARGET})

