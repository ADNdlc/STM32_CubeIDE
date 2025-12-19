/*
 * app.c
 *
 *  Created on: Dec 15, 2025
 *      Author: 12114
 */

#define USE_OLD_UI 0


#include "app.h"
#include "lvgl.h"
#if USE_OLD_UI
#include "ui/Act_Manager.h"
#else
#include "ui.h"
#endif

int app_init(void) {
#if USE_OLD_UI
  act_manager_init();	// 旧ui入口
#else
  ui_init();
#endif
  return 0;
}

void app_run(void) { lv_timer_handler(); }
