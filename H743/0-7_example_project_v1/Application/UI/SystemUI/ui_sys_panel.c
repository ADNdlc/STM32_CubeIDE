#include "ui_sys_panel.h"
#include "../../System/sys_state.h"

/*********************
 *      DEFINES
 *********************/

/*********************
 *      EXTERNS
 *********************/
LV_IMG_DECLARE(icon_bright);
LV_IMG_DECLARE(icon_wifi);

/*********************
 *  STATIC VARIABLES
 *********************/
static lv_obj_t *panel_bg = NULL;      // The full-screen background
static lv_obj_t *panel_content = NULL; // The actual panel content
static bool is_panel_active = false;   // Active state

/*********************
 *  STATIC PROTOTYPES
 *********************/
static void event_panel_drag_cb(lv_event_t *e);
static void delete_anim_ready_cb(lv_anim_t *anim);
static void event_vol_cb(lv_event_t *e);
static void event_bri_cb(lv_event_t *e);
static void event_wifi_cb(lv_event_t *e);

/*********************
 *   STYLE HELPERS
 *********************/
static void obj_style_set(lv_obj_t *obj) {
  lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN);
  lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
}

/**
 * @brief Create a child container block for the menu grid
 */
static lv_obj_t *menu_child_container_create(lv_obj_t *parent, uint8_t col,
                                             uint8_t col_span, uint8_t row,
                                             uint8_t row_span) {
  // Block container for alignment
  lv_obj_t *block = lv_obj_create(parent);
  lv_obj_set_style_bg_opa(block, 0, LV_PART_MAIN);
  obj_style_set(block);
  lv_obj_set_grid_cell(block, LV_GRID_ALIGN_STRETCH, col, col_span,
                       LV_GRID_ALIGN_STRETCH, row, row_span);

  // Content background
  lv_obj_t *block_bg = lv_obj_create(block);
  lv_obj_set_size(block_bg, LV_PCT(100),
                  LV_PCT(100)); // Will be adjusted by padding if needed, but
                                // grid usually handles it
  // Wait, Popup.c used update_layout and manual size. Grid cell with stretch is
  // better. Popup.c: lv_obj_set_size(block_bg, lv_obj_get_width(block) - 20,
  // lv_obj_get_height(block) - 20); Let's use padding on the block instead?
  // Following Popup.c logic closer for safety:
  lv_obj_set_size(block_bg, LV_PCT(90), LV_PCT(90)); // Approx 20px buffer
  lv_obj_align(block_bg, LV_ALIGN_CENTER, 0, 0);

  obj_style_set(block_bg);
  lv_obj_set_style_radius(block_bg, 30, LV_PART_MAIN);
  lv_obj_set_style_bg_color(block_bg, lv_color_hex(0x828282), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(block_bg, 200, LV_PART_MAIN);

  return block_bg;
}

/*********************
 *      FUNCTIONS
 *********************/

void ui_sys_panel_init(void) {
  // Nothing to init globally yet, panel is created on demand
}

void ui_sys_panel_show(void) {
  if (is_panel_active || panel_bg != NULL) {
    return;
  }

  is_panel_active = true;

  // 1. Create Background (Layer Top)
  panel_bg = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(panel_bg);
  lv_obj_set_size(panel_bg, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(panel_bg, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(panel_bg, LV_OPA_0, 0); // Initially transparent
  lv_obj_clear_flag(panel_bg, LV_OBJ_FLAG_SCROLLABLE);

  // 2. Create Content Container
  panel_content = lv_obj_create(panel_bg);
  lv_obj_set_size(panel_content, LV_PCT(100),
                  LV_PCT(100)); // Full screen size content
  lv_obj_set_y(panel_content,
               -lv_disp_get_ver_res(NULL)); // Start off-screen (top)

  // Style
  lv_obj_set_style_bg_color(panel_content, lv_color_hex(0x000000),
                            LV_PART_MAIN);
  lv_obj_set_style_bg_opa(panel_content, 200,
                          LV_PART_MAIN); // Semi-transparent black
  obj_style_set(panel_content);

  // Grid Layout
  static lv_coord_t col_dsc[] = {
      LV_GRID_FR(1),        98, 98, 98, 98, 98, 98, 98, LV_GRID_FR(1),
      LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {
      LV_GRID_FR(1), 98, 98, 98, 98, LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(panel_content, col_dsc, row_dsc);
  lv_obj_set_style_pad_all(panel_content, 0, 0);
  lv_obj_set_style_pad_gap(panel_content, 20, 20);

  // 3. Bottom Drag Handle Line
  lv_obj_t *handle_area = lv_obj_create(panel_content);
  lv_obj_set_style_bg_color(handle_area, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(handle_area, 40, 0);
  obj_style_set(handle_area);
  lv_obj_set_grid_cell(handle_area, LV_GRID_ALIGN_STRETCH, 0, 9,
                       LV_GRID_ALIGN_STRETCH, 5, 1);

  // Line
  static lv_point_t line_points[] = {{0, 0}, {60, 0}};
  lv_obj_t *line = lv_line_create(handle_area);
  lv_line_set_points(line, line_points, 2);
  lv_obj_align(line, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_line_width(line, 5, 0);
  lv_obj_set_style_line_color(line, lv_color_hex(0x353535), 0);
  lv_obj_set_style_line_rounded(line, true, 0);

  // 4. Populate Content
  const sys_state_t *state = sys_state_get();

  // -- Brightness Slider (Col 3, Row 1, Span 2 Rows)
  lv_obj_t *bri_bg = menu_child_container_create(panel_content, 3, 1, 1, 2);
  lv_obj_t *slider_bri = lv_slider_create(bri_bg);
  lv_obj_set_size(slider_bri, LV_PCT(100), LV_PCT(100));
  lv_obj_center(slider_bri);
  lv_obj_remove_style(slider_bri, NULL, LV_PART_KNOB);
  lv_obj_set_style_radius(slider_bri, 30, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(slider_bri, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(slider_bri, 30, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider_bri, lv_color_hex(0xF0F0F0),
                            LV_PART_INDICATOR);
  lv_slider_set_range(slider_bri, 0, 100);
  lv_slider_set_value(slider_bri, state->brightness, LV_ANIM_OFF);
  lv_obj_add_event_cb(slider_bri, event_bri_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Brightness Icon
  lv_obj_t *img_bri = lv_img_create(slider_bri);
  lv_img_set_src(img_bri, &icon_bright);
  lv_obj_align(img_bri, LV_ALIGN_CENTER, 0, 47);
  lv_obj_set_style_img_recolor(img_bri, lv_color_hex(0x505050), 0);
  lv_opa_t bri_opa =
      lv_map(state->brightness, 0, 100, LV_OPA_COVER, LV_OPA_TRANSP);
  lv_obj_set_style_img_recolor_opa(img_bri, bri_opa, 0);

  // -- Volume Slider (Col 4, Row 1, Span 2 Rows)
  lv_obj_t *vol_bg = menu_child_container_create(panel_content, 4, 1, 1, 2);
  lv_obj_t *slider_vol = lv_slider_create(vol_bg);
  lv_obj_set_size(slider_vol, LV_PCT(100), LV_PCT(100));
  lv_obj_center(slider_vol);
  lv_obj_remove_style(slider_vol, NULL, LV_PART_KNOB);
  lv_obj_set_style_radius(slider_vol, 30, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(slider_vol, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(slider_vol, 30, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider_vol, lv_color_hex(0xF0F0F0),
                            LV_PART_INDICATOR);
  lv_slider_set_range(slider_vol, 0, 100);
  lv_slider_set_value(slider_vol, state->volume, LV_ANIM_OFF);
  lv_obj_add_event_cb(slider_vol, event_vol_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Volume Icon/Label
  lv_obj_t *lbl_vol = lv_label_create(slider_vol);
  lv_label_set_text(lbl_vol, LV_SYMBOL_VOLUME_MID);
  lv_obj_set_style_text_font(lbl_vol, &lv_font_montserrat_30, 0);
  lv_obj_align(lbl_vol, LV_ALIGN_CENTER, 0, 50);

  // -- WiFi Button (Col 5, Row 1)
  lv_obj_t *wifi_bg = menu_child_container_create(panel_content, 5, 1, 1, 1);
  lv_obj_t *btn_wifi = lv_btn_create(wifi_bg);
  lv_obj_set_size(btn_wifi, LV_PCT(100), LV_PCT(100));
  lv_obj_center(btn_wifi);
  lv_obj_set_style_radius(btn_wifi, 30, LV_PART_MAIN);
  lv_obj_add_flag(btn_wifi, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_style_bg_color(btn_wifi, lv_color_hex(0x828282), LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(btn_wifi, lv_color_hex(0x1E90FF), LV_STATE_CHECKED);
  if (state->wifi_connected)
    lv_obj_add_state(btn_wifi, LV_STATE_CHECKED);
  lv_obj_add_event_cb(btn_wifi, event_wifi_cb, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *img_wifi = lv_img_create(btn_wifi);
  lv_img_set_src(img_wifi, &icon_wifi);
  lv_obj_center(img_wifi);
  lv_img_set_zoom(img_wifi, 300);
  lv_obj_set_style_img_recolor(img_wifi, lv_color_hex(0x333333), 0);
  lv_obj_set_style_img_recolor_opa(img_wifi, LV_OPA_COVER, 0);
  lv_obj_set_user_data(btn_wifi, img_wifi); // Store for recurrence

  // 5. Add Events to Background for Dragging
  lv_obj_set_user_data(panel_bg, panel_content);
  lv_obj_add_event_cb(panel_bg, event_panel_drag_cb, LV_EVENT_ALL, NULL);

  // 6. Animate In (Slide Down)
  // Actually we start animation here?
  // If called by drag, we should manually position.
  // If called by button, we animate.
  // Assuming button call:
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, panel_content);
  lv_anim_set_values(&a, -lv_disp_get_ver_res(NULL), 0);
  lv_anim_set_time(&a, 300);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
  lv_anim_start(&a);
}

void ui_sys_panel_hide(void) {
  if (!is_panel_active || !panel_bg || !panel_content)
    return;

  // Animate Out
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, panel_content);
  lv_anim_set_values(&a, lv_obj_get_y(panel_content),
                     -lv_disp_get_ver_res(NULL));
  lv_anim_set_time(&a, 300);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
  lv_anim_set_ready_cb(&a, delete_anim_ready_cb);
  lv_anim_start(&a);
}

bool ui_sys_panel_is_visible(void) { return is_panel_active; }

/*********************
 *   EVENT CALLBACKS
 *********************/

static void delete_anim_ready_cb(lv_anim_t *anim) {
  lv_obj_del(panel_bg);
  panel_bg = NULL;
  panel_content = NULL;
  is_panel_active = false;
}

static void event_panel_drag_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *bg = lv_event_get_target(e);
  lv_obj_t *content = lv_obj_get_user_data(bg);
  if (!content)
    return;

  lv_indev_t *indev = lv_indev_get_act();
  if (!indev)
    return;

  lv_point_t p;
  lv_indev_get_point(indev, &p);

  lv_coord_t h = lv_obj_get_height(content);
  lv_coord_t y = lv_obj_get_y(content);

  // Opacity based on position
  lv_coord_t threshold = -h / 2;
  uint8_t opa = 0;
  if (y > threshold)
    opa = 150;
  else
    opa = lv_map(y, -h, threshold, 0, 150);
  lv_obj_set_style_bg_opa(bg, opa, 0);

  if (code == LV_EVENT_PRESSING) {
    lv_obj_set_y(content, p.y - h);
  } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
    if (p.y > lv_disp_get_ver_res(NULL) / 2) {
      // Slide In Fully
      lv_anim_t a;
      lv_anim_init(&a);
      lv_anim_set_var(&a, content);
      lv_anim_set_values(&a, y, 0);
      lv_anim_set_time(&a, 300);
      lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
      lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
      lv_anim_start(&a);
    } else {
      // Slide Out (Close)
      ui_sys_panel_hide();
    }
  }
}

static void event_vol_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  uint8_t val = (uint8_t)lv_slider_get_value(slider);

  sys_state_set_volume(val);

  // Update Icon
  lv_obj_t *lbl = lv_obj_get_child(slider, 0);
  if (lbl) {
    if (val < 40)
      lv_label_set_text(lbl, LV_SYMBOL_MUTE);
    else if (val < 70)
      lv_label_set_text(lbl, LV_SYMBOL_VOLUME_MID);
    else
      lv_label_set_text(lbl, LV_SYMBOL_VOLUME_MAX);
  }
}

static void event_bri_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  uint8_t val = (uint8_t)lv_slider_get_value(slider);

  sys_state_set_brightness(val);

  // Update Icon Opacity
  lv_obj_t *img = lv_obj_get_child(slider, 0);
  if (img) {
    lv_opa_t opa = lv_map(val, 0, 100, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_obj_set_style_img_recolor_opa(img, opa, 0);
  }
}

static void event_wifi_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  bool checked = lv_obj_has_state(btn, LV_STATE_CHECKED);

  sys_state_set_wifi(checked);

  // Update Icon
  lv_obj_t *img = lv_obj_get_user_data(btn);
  if (img) {
    if (checked)
      lv_obj_set_style_img_recolor(img, lv_color_hex(0xEEEEEE), 0);
    else
      lv_obj_set_style_img_recolor(img, lv_color_hex(0x333333), 0);
  }
}
