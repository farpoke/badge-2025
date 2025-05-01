#pragma once

// C.f. micropython/ports/rp2/pendsv.h

#include <stddef.h>

#if __cplusplus
extern "C" {
#endif

typedef void (*pendsv_callback_t)(void);

void pendsv_suspend(void);
void pendsv_resume(void);

void pendsv_schedule_softtimer(pendsv_callback_t callback);

void pendsv_handle_interrupt(void);

#if __cplusplus
}
#endif
