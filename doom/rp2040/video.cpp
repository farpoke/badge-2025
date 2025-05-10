#include "../doomdef.h"
#include "../i_video.h"

#include <pico/time.h>


void I_InitGraphics() {}
void I_ShutdownGraphics() {}

void I_SetPalette(byte *palette) {}

void I_UpdateNoBlit() {}
void I_FinishUpdate() {}

void I_WaitVBL(int count) { sleep_us(count * (1'000'000 / TICRATE)); }

void I_ReadScreen(byte *scr) {}

void I_BeginRead() {}

void I_EndRead() {}

void I_StartFrame() {}

void I_StartTic() {}
