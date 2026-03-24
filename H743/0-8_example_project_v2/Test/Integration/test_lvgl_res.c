#include "test_config.h"

#if ENABLE_TEST_LVGL_RES
#include "Sys.h"
#include "lvgl.h"
#include "test_framework.h"

#include "asset_manager/lvgl_resource/lvgl_resource.h"

lv_obj_t *lvgl_test_icon;

static int16_t x_pos = 0;
static int16_t y_pos = 0;
static int8_t x_dir = 1;
static int8_t y_dir = 1;

static void test_lvgl_res_setup(void) {
  log_i("LVGL Integration Test RES Setup");

  // 初始化资源管理 (如果开启了 RES_BURN_ENABLE 宏，内部会自动执行烧录)
  res_init();

  /* 创建一个图片 */
  lvgl_test_icon = lv_img_create(lv_scr_act());
  const lv_img_dsc_t *img = res_get_img(RES_IMG_TEST);
  if (img) {
      lv_img_set_src(lvgl_test_icon, img);
      lv_obj_set_pos(lvgl_test_icon, 0, 0);
  } else {
      log_w("Resource RES_IMG_TEST not found. Needs burning?");
  }

  x_pos = 0;
  y_pos = 0;
}

static void test_lvgl_res_loop(void) {
  // 简单的运动动画
  x_pos += x_dir * 2;
  y_pos += y_dir * 2;

  if (x_pos >= (LV_HOR_RES - 64) || x_pos <= 0) x_dir = -x_dir;
  if (y_pos >= (LV_VER_RES - 64) || y_pos <= 0) y_dir = -y_dir;

  lv_obj_set_pos(lvgl_test_icon, x_pos, y_pos);

  lv_timer_handler();
  sys_delay_ms(5);
}

static void test_lvgl_res_teardown(void) {
  log_i("LVGL Integration Tes RES Teardown");
  /* 清除屏幕 */
  lv_obj_clean(lv_scr_act());
}

REGISTER_TEST(LVGL_RES, "Resource LVGL integration test", test_lvgl_res_setup,
              test_lvgl_res_loop, test_lvgl_res_teardown);

#endif
