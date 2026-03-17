#include "test_config.h"

#if ENABLE_TEST_LVGL_RES
#include "Sys.h"
#include "lvgl.h"
#include "test_framework.h"

#include "asset_manager/lvgl_resource/lvgl_resource.h"

lv_obj_t *lvgl_test_icon;

static void test_lvgl_res_setup(void) {
  log_i("LVGL Integration Test RES Setup");

  /* 创建一个图片 */
  lvgl_test_icon = lv_img_create(lv_scr_act());
  lv_img_set_src(lvgl_test_icon, res_get_src(RES_IMG_TEST));
  lv_obj_align(lvgl_test_icon, LV_ALIGN_CENTER, 0, 0);
}

static void test_lvgl_res_loop(void) {
  lv_timer_handler();
  sys_delay_ms(5);
}

static void test_lvgl_res_teardown(void) {
  log_i("LVGL Integration Tes RES Teardown");
  /* 清除屏幕 */
  lv_obj_clean(lv_scr_act());
}

REGISTER_TEST(LVGL, "Eesource LVGL integration test", test_lvgl_res_setup,
              test_lvgl_res_loop, test_lvgl_res_teardown);

#endif
