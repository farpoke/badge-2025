#include "../i_system.h"
#include "../d_net.h"
#include "../i_video.h"
#include "../doomdef.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <pico/time.h>

namespace
{
    absolute_time_t _initTime;
    ticcmd_t _emptyTickCommand = {};

    byte _zoneMemory[4096];
}

void I_Init() {
    I_InitGraphics();
    _initTime = get_absolute_time();
}

byte *I_ZoneBase(int *size) {
    *size = sizeof(_zoneMemory);
    return _zoneMemory;
}

int I_GetTime() {
    const auto now = get_absolute_time();
    const auto us = absolute_time_diff_us(_initTime, now);
    return us * TICRATE / 1'000'000;
}

void I_StartFrame() {}

void I_StartTic() {}

ticcmd_t *I_BaseTiccmd(void) {
    return &_emptyTickCommand;
}

void I_Quit() {
    D_QuitNetGame();
    I_ShutdownGraphics();
}

byte *I_AllocLow(int length) {
    const auto memory = static_cast<byte*>(malloc(length));
    memset(memory, 0, length);
    return memory;
}

void I_Tactile(int on, int off, int total) {
    (void)on;
    (void)off;
    (void)total;
}

void I_Error(char *error, ...) {
    va_list va;

    va_start(va, error);
    printf("\n\nDOOM ERROR:\n");
    vprintf(error, va);
    printf("\n\n");
    va_end(va);

    I_Quit();
}
