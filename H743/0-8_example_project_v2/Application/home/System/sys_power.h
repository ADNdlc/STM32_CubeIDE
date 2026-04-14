#ifndef _SYS_POWER_H
#define _SYS_POWER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SCREEN_ON,
    SCREEN_OFF
} screen_state_t;

void sys_power_init(void);
void sys_power_process(void);
void sys_power_refresh(void);
screen_state_t sys_power_get_screen_state(void);

#endif
