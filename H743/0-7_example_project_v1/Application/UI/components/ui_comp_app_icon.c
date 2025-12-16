#include "ui_comp_app_icon.h"

// 内部函数：根据高度选择字体
static const lv_font_t *get_font_by_height(lv_coord_t height) {
  if (height < 85)
    return &lv_font_montserrat_12;
  else if (height < 115)
    return &lv_font_montserrat_14;
  else if (height < 140)
    return &lv_font_montserrat_18;
  else if (height < 170)
    return &lv_font_montserrat_22;
  else if (height < 200)
    return &lv_font_montserrat_24;
  else if (height < 250)
    return &lv_font_montserrat_26;
  else
    return &lv_font_montserrat_30;
}

lv_obj_t *ui_comp_app_icon_create(lv_obj_t *parent, const void *icon_src,
                                  const char *name) {
  // 1. 创建容器
  lv_obj_t *comp = lv_obj_create(parent);
  lv_obj_clear_flag(comp, LV_OBJ_FLAG_SCROLLABLE); // 移除滚动
  lv_obj_set_style_bg_opa(comp, LV_OPA_TRANSP, 0); // 透明
  lv_obj_set_style_border_width(comp, 0, 0);       // 无边框
  lv_obj_set_style_pad_all(comp, 0, 0);            // 无内边距

  // 2. 创建图片
  lv_obj_t *img = lv_img_create(comp);
  lv_img_set_src(img, icon_src);
  lv_obj_align(img, LV_ALIGN_CENTER, 0, -10);  // 暂定偏移
  lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE); // 图片可点击

  // 3. 创建标签
  lv_obj_t *label = lv_label_create(comp);
  lv_label_set_text(label, name);
  lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -5);

  // 默认样式，后续通过 update_layout 调整
  return comp;
}

void ui_comp_app_icon_update_layout(lv_obj_t *comp, lv_coord_t row_height) {
  if (comp == NULL)
    return;

  // 获取子对象 (假设创建顺序: 0=img, 1=label)
  lv_obj_t *img = lv_obj_get_child(comp, 0);
  lv_obj_t *label = lv_obj_get_child(comp, 1);

  // 动态调整图标和字体大小
  lv_img_set_zoom(img, (256 * row_height) / 100);
  lv_obj_align(img, LV_ALIGN_CENTER, 0, -row_height / 15);

  lv_obj_set_style_text_font(label, get_font_by_height(row_height), 0);
  lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -row_height / 14);
}
