/*
 * app.c
 *
 *  Created on: Dec 15, 2025
 *      Author: 12114
 */

#include "app.h"
//#include "UI/ui.h"
#include "lvgl.h"

#include "Act_Manager.h"


int app_init(void) {
  //ui_init();
	act_manager_init();

  return 0;
}

void app_run(void) { lv_timer_handler(); }
