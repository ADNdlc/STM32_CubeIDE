#include "sys_power.h"
#include "Sys.h"
#include "lcd_screen_driver.h"
#include "lv_port_disp.h"
#include "Project_cfg.h"

#define LOG_TAG "SYS_POWER"
#include "elog.h"

#ifndef SCREEN_AUTO_OFF_TIMEOUT
#define SCREEN_AUTO_OFF_TIMEOUT 30000 // 30 seconds
#endif

extern lcd_driver_t *lcd;

static uint32_t last_activity_tick = 0;
static screen_state_t current_state = SCREEN_ON;

void sys_power_init(void) {
    last_activity_tick = sys_get_systick_ms();
    current_state = SCREEN_ON;
    log_i("System Power Manager initialized.");
}

void sys_power_refresh(void) {
    last_activity_tick = sys_get_systick_ms();
    
    if (current_state == SCREEN_OFF) {
        log_i("Waking up screen...");
        if (lcd) {
            LCD_DISPLAY_ON(lcd);
        }
        disp_enable_update();
        current_state = SCREEN_ON;
    }
}

void sys_power_process(void) {
    if (current_state == SCREEN_ON) {
        uint32_t now = sys_get_systick_ms();
        if (now - last_activity_tick > SCREEN_AUTO_OFF_TIMEOUT) {
            log_i("Screen timeout, powering off...");
            if (lcd) {
                LCD_DISPLAY_OFF(lcd);
            }
            disp_disable_update();
            current_state = SCREEN_OFF;
        }
    }
}

screen_state_t sys_power_get_screen_state(void) {
    return current_state;
}
