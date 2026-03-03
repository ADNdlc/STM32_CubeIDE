#include "test_config.h"
#if ENABLE_TEST_LVGL
#include "Sys.h"
#include "lvgl.h"
#include "test_framework.h"


static void test_lvgl_setup(void) {
  log_i("LVGL Integration Test Setup");

  /* 创建一个简单的界面 */
  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Hello LVGL! - STM32H743");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 40);
  lv_obj_t *btn_label = lv_label_create(btn);
  lv_label_set_text(btn_label, "Click Me (Simulated)");
}

static void test_lvgl_loop(void) {
  /* LVGL 需要周期性调用 timer_handler */
  lv_timer_handler();
  sys_delay_ms(5);
}

static void test_lvgl_teardown(void) {
  log_i("LVGL Integration Test Teardown");
  /* 清除屏幕 */
  lv_obj_clean(lv_scr_act());
}

REGISTER_TEST(LVGL, "Basic LVGL integration test", test_lvgl_setup,
              test_lvgl_loop, test_lvgl_teardown);

#endif
