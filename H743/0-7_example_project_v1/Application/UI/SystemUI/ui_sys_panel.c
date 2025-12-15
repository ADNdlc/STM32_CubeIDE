#include "ui_sys_panel.h"
#include "../../System/sys_state.h"

static lv_obj_t *panel_obj = NULL;
static lv_obj_t *slider_vol;
static lv_obj_t *slider_bri;

static void event_vol_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  sys_state_set_volume((uint8_t)lv_slider_get_value(slider));
}

static void event_bri_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  sys_state_set_brightness((uint8_t)lv_slider_get_value(slider));
}

static void close_cb(lv_event_t *e) { ui_sys_panel_hide(); }

void ui_sys_panel_init(void) {
  // Create hidden
}

void ui_sys_panel_show(void) {
  if (panel_obj)
    return; // Already shown

  // Create a modal-like panel on sys layer
  panel_obj = lv_obj_create(lv_layer_sys());
  lv_obj_set_size(panel_obj, LV_PCT(100), LV_PCT(50)); // Half screen
  lv_obj_align(panel_obj, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(panel_obj, lv_color_hex(0x202020), 0);
  lv_obj_set_style_bg_opa(panel_obj, LV_OPA_90, 0);

  // Initial state
  const sys_state_t *state = sys_state_get();

  // Volume Slider
  slider_vol = lv_slider_create(panel_obj);
  lv_obj_set_width(slider_vol, 200);
  lv_obj_align(slider_vol, LV_ALIGN_CENTER, 0, -30);
  lv_slider_set_value(slider_vol, state->volume, LV_ANIM_OFF);
  lv_obj_add_event_cb(slider_vol, event_vol_cb, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *l1 = lv_label_create(panel_obj);
  lv_label_set_text(l1, "Volume");
  lv_obj_align_to(l1, slider_vol, LV_ALIGN_OUT_TOP_MID, 0, -5);

  // Brightness Slider
  slider_bri = lv_slider_create(panel_obj);
  lv_obj_set_width(slider_bri, 200);
  lv_obj_align(slider_bri, LV_ALIGN_CENTER, 0, 30);
  lv_slider_set_value(slider_bri, state->brightness, LV_ANIM_OFF);
  lv_obj_add_event_cb(slider_bri, event_bri_cb, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *l2 = lv_label_create(panel_obj);
  lv_label_set_text(l2, "Brightness");
  lv_obj_align_to(l2, slider_bri, LV_ALIGN_OUT_TOP_MID, 0, -5);

  // Close on click outside (optional, or just a close button)
  lv_obj_t *btn_close = lv_btn_create(panel_obj);
  lv_obj_align(btn_close, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_label_set_text(lv_label_create(btn_close), "Close");
  lv_obj_add_event_cb(btn_close, close_cb, LV_EVENT_CLICKED, NULL);

  // Animation in
  lv_obj_update_layout(panel_obj);
  lv_coord_t h = lv_obj_get_height(panel_obj);
  lv_obj_set_y(panel_obj, -h);
  lv_obj_set_y(panel_obj, 0); // Animate... standard LVGL doesn't have auto move
                              // anim here without wrapper

  // Simple slide in
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, panel_obj);
  lv_anim_set_values(&a, -h, 0);
  lv_anim_set_time(&a, 300);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
  lv_anim_start(&a);
}

void ui_sys_panel_hide(void) {
  if (!panel_obj)
    return;

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, panel_obj);
  lv_anim_set_values(&a, 0, -lv_obj_get_height(panel_obj));
  lv_anim_set_time(&a, 300);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
  lv_anim_set_ready_cb(&a, lv_obj_delete_anim_completed_cb); // Auto delete
  lv_anim_start(&a);

  panel_obj = NULL;
}

bool ui_sys_panel_is_visible(void) { return panel_obj != NULL; }
