#include "DevControl_settings_view.h"
#include "device_control/device_control.h"
#include "../../System/Contol_controller.h"
#include "lv_util.h"
#include "lvgl.h"
#include <stdint.h>
#include <string.h>

#define LOG_TAG "SETTING_VIEW"
#include "elog.h"


static void mode_selection_event_cb(lv_event_t *e) {
  uint32_t mode = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
  app_settings_t *settings = get_self_settings();
  if (settings && settings->count > 0) {
    settings->configs[0].Int = mode;
    app_settings_update("DevControl", settings);
    log_i("Display mode changed to: %d", mode);
  }
}

lv_obj_t *create_dev_control_settings_screen(void) {
  lv_obj_t *screen = lv_obj_create(NULL);

  // Add a title
  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, "Display Settings");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 40);

  lv_obj_t *list = lv_list_create(screen);
  lv_obj_set_size(list, LV_PCT(80), LV_PCT(60));
  lv_obj_center(list);

  lv_obj_t *btn_compact =
      lv_list_add_btn(list, LV_SYMBOL_LIST, "Compact Card Mode");
  lv_obj_add_event_cb(btn_compact, mode_selection_event_cb, LV_EVENT_CLICKED,
                      (void *)(uintptr_t)UI_FULL_MODE);

  lv_obj_t *btn_dispersed =
      lv_list_add_btn(list, LV_SYMBOL_LIST, "Dispersed Property Mode");
  lv_obj_add_event_cb(btn_dispersed, mode_selection_event_cb, LV_EVENT_CLICKED,
                      (void *)(uintptr_t)UI_COMPACT_MODE);

  return screen;
}
