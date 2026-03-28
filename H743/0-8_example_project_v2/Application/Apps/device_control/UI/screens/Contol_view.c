#define LOG_TAG "CTRL_VIEW"
#include "Contol_view.h"
#include "../../System/Contol_controller.h"
#include "../../device_control.h"
#include "../components/style_util.h"
#include "app_manager.h"
#include "elog.h"
#include "lv_util.h"
#include "thing_model/thing_model.h"
#include "lvgl_resource.h"


#if 1

// 主页布局描述
extern void controller_register_ui_control(const char *deviceID,
                                           const char *propID, lv_obj_t *obj);
extern void generic_control_event_cb(lv_event_t *e);

static void settings_btn_event_cb(lv_event_t *e) {
  lv_obj_t *settings_scr = create_dev_control_settings_screen();
  if (settings_scr) {
    app_manager_push_screen(settings_scr);
  }
}

// 主页网格布局定义(静态)
static lv_coord_t main_row_dsc[] = {1,   180, 180,
                                    180, 180, LV_GRID_TEMPLATE_LAST}; // 高
static lv_coord_t main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                    LV_GRID_TEMPLATE_LAST}; // 宽

static lv_obj_t *create_card_icon(lv_obj_t *parent, lv_obj_t *img, int img_w,
                                  int img_h) {
  // 1. 图标区域
  lv_obj_t *img_icon = lv_obj_create(parent);
  lv_obj_set_size(img_icon, img_w, img_h);                  // 目标大小
  lv_obj_set_style_bg_opa(img_icon, LV_OPA_TRANSP, 0);      // 透明背景
  lv_obj_set_style_pad_all(img_icon, 0, 0);                 // 无内边距
  lv_obj_set_style_border_width(img_icon, 0, LV_PART_MAIN); // 无边框
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_SCROLLABLE);      // 禁止滚动
  lv_obj_align(img_icon, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_update_layout(img_icon); // 更新布局信息

  // 获取设备图标
  lv_obj_t *card_img = lv_img_create(img_icon);
  lv_obj_align(card_img, LV_ALIGN_CENTER, 0, 0);
  if (img) {
    lv_img_set_src(card_img, img);
  } else {
    lv_img_set_src(card_img, res_get_img(RES_IMG_ICON_CONTROL)); // 使用缺省值
  }
  // 获取图标原始尺寸
  lv_img_header_t header_img;
  const void *img_src = lv_img_get_src(card_img);
  lv_res_t res = lv_img_decoder_get_info(img_src, &header_img);
  lv_coord_t zoom;
  if (res == LV_RES_OK && header_img.w > 0 &&
      header_img.h > 0) { // 检查图片信息是否成功获取且尺寸有效
    lv_coord_t available_size = lv_obj_get_content_height(img_icon);
    log_d("header size: %d", available_size);

    // 计算基于宽度的缩放比例
    // 将 available_size 转换为 int32_t 进行乘法，避免溢出，然后除以原始宽度
    int32_t zoom_w = (int32_t)available_size * LV_IMG_ZOOM_NONE / header_img.w;

    // 计算基于高度的缩放比例
    int32_t zoom_h = (int32_t)available_size * LV_IMG_ZOOM_NONE / header_img.h;

    // 为了确保图片完全可见且不被裁剪，我们需要选择较小的缩放比例
    // 这样图片的两条边都能被限制在可用空间内
    zoom = LV_MIN(zoom_w, zoom_h);

    // 确保缩放比例不为0，以防极端情况导致显示异常
    if (zoom <= 0) {
      zoom = LV_IMG_ZOOM_NONE; // 如果计算结果不合理，则不缩放
    }

    lv_img_set_zoom(card_img, zoom);
    log_d("img zoom: %d", zoom);
  } else {
    // 如果图片信息无法获取或尺寸无效，则不进行缩放（100%）
    lv_img_set_zoom(card_img, LV_IMG_ZOOM_NONE); // LV_IMG_ZOOM_NONE 等同于 256
  }
  return img_icon;
}

// 释放控件上下文空间的事件回调
static void free_context_event_cb(lv_event_t *e) {
  control_event_ctx_t *ctx = (control_event_ctx_t *)lv_event_get_user_data(e);
  if (ctx) {
    // 释放之前为这个控件分配的上下文内存
    log_d("Freeing context for [%s/%s]", ctx->deviceID, ctx->propID);
    lv_mem_free(ctx);
  } else {
    log_e("free_context_event_cb: ctx is NULL");
  }
}

void create_main(lv_obj_t *tabview) {
  log_i("Creating Main Tab...");
  lv_obj_t *tab_main =
      lv_tabview_add_tab(tabview, LV_SYMBOL_HOME " HOME"); // 添加一个页面
  lv_obj_set_grid_dsc_array(tab_main, main_col_dsc,
                            main_row_dsc); // 主页使用网格布局

  // test_layout_grid(tab_main, 5, 3);//布局测试
  controller_init_main_tab(tab_main); // 填充内容(根据已注册设备创建控制卡片)
}

void create_add(lv_obj_t *tabview) {
  log_i("Creating Add Tab...");
  lv_obj_t *tab_add = lv_tabview_add_tab(tabview, LV_SYMBOL_PLUS " ADD");

  lv_obj_t *add_directory = lv_tabview_create(
      tab_add, LV_DIR_LEFT, scr_act_width() / 6); // 内部使用tableview
  lv_obj_align(add_directory, LV_ALIGN_LEFT_MID, -scr_act_width() / 40,
               0); // 对齐屏幕边缘
  lv_obj_set_size(add_directory, LV_PCT(100),
                  LV_PCT(100)); // 设置 add_directory 占满整个父容器
  style_tabview_simple(add_directory, style_get_addtab_default(),
                       style_get_addtab_checked());

  // 添加设备分类目录
  lv_obj_t *directory_light = lv_tabview_add_tab(add_directory, "Light");
  lv_obj_t *directory_sensor = lv_tabview_add_tab(add_directory, "Sensor");
  lv_obj_t *test1 = lv_tabview_add_tab(add_directory, "test1");
  lv_obj_t *test2 = lv_tabview_add_tab(add_directory, "test2");
  lv_obj_t *test3 = lv_tabview_add_tab(add_directory, "test3");

  lv_obj_t *btns = lv_tabview_get_tab_btns(add_directory);

  controller_init_add_tab(add_directory); // 根据"可添加设备"填充内容
}

void create_user(lv_obj_t *tabview) {
  log_i("Creating User Tab...");
  char *user_name = "user_name"; // 假设获取到了登陆的用户id
  char *uid_str = "1234567890";

  lv_obj_t *tab_user = lv_tabview_add_tab(tabview, LV_SYMBOL_SETTINGS " MY");
  lv_obj_clear_flag(tab_user, LV_OBJ_FLAG_SCROLLABLE); // 禁用容器的滚动功能

  lv_obj_t *user = lv_obj_create(tab_user);
  lv_obj_align(user, LV_ALIGN_TOP_MID, 0, 10); // 顶部居中,有一定间距
  lv_obj_set_size(user, scr_act_width() / 2, scr_act_height() / 6);

  lv_obj_t *img = lv_img_create(user);
  lv_img_set_src(img, res_get_img(RES_IMG_DEFAULT_USER));
  lv_img_set_zoom(img, 450);
  lv_obj_align(img, LV_ALIGN_TOP_LEFT, 10, 3);

  lv_obj_t *name = lv_label_create(user);
  lv_label_set_text(name, user_name);
  lv_obj_set_style_text_font(name, &lv_font_montserrat_20,
                             LV_PART_MAIN); // 设置字体
  lv_obj_align_to(name, img, LV_ALIGN_OUT_RIGHT_TOP, scr_act_width() / 20,
                  -10); // 靠右对齐图片

  lv_obj_t *uid = lv_label_create(user);
  lv_label_set_text(uid, uid_str);
  lv_obj_set_style_text_font(uid, &lv_font_montserrat_16,
                             LV_PART_MAIN); // 设置字体
  lv_obj_align_to(uid, img, LV_ALIGN_OUT_RIGHT_BOTTOM, scr_act_width() / 20,
                  0); // 靠右对齐图片

  // 添加包含Setting, about, help, version 按钮的列表
  // 列表,添加圆角,单列
  lv_obj_t *list = lv_list_create(tab_user); // 创建表格
  lv_obj_set_size(list, scr_act_width() / 2, scr_act_height() / 3 + 10);
  lv_obj_align_to(list, user, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  15); // 与用户信息块有一定间距
  // 函数第一个参数为添加到的列表,
  // 第二个参数为"按钮图标"(可使用lvgl提供的图标枚举)，第三个参数为"按钮文本"。返回值为添加的按钮
  lv_obj_t *Setting = lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "Setting");
  lv_obj_add_event_cb(Setting, settings_btn_event_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_t *about = lv_list_add_btn(list, LV_SYMBOL_WARNING, "about");
  lv_obj_t *help = lv_list_add_btn(list, LV_SYMBOL_PLUS, "help");
  lv_obj_t *version = lv_list_add_btn(list, LV_SYMBOL_FILE, "version");

  // 添加登出按钮,白色按钮，红色字体,去除阴影
  lv_obj_t *logout_btn = lv_btn_create(tab_user);
  lv_obj_set_size(logout_btn, scr_act_width() / 2, scr_act_height() / 10);
  lv_obj_set_style_bg_color(logout_btn, lv_color_hex(0xffffff),
                            LV_PART_MAIN);                     // 白色背景
  lv_obj_set_style_shadow_width(logout_btn, 0, LV_PART_ITEMS); // 去除按钮阴影

  lv_obj_align_to(logout_btn, list, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  10); // 与表格有一定距
  lv_obj_t *logout_label = lv_label_create(logout_btn);
  lv_label_set_text(logout_label, "Logout");
  lv_obj_center(logout_label);
  lv_obj_set_style_text_font(logout_label, &lv_font_montserrat_24,
                             LV_PART_MAIN); // 设置字体
  lv_obj_center(logout_label);
  lv_obj_set_style_text_color(logout_label, lv_color_hex(0xFF6045),
                              LV_PART_MAIN); // 红色字体

  controller_init_user_tab(tab_user);
}

/**
 * @brief 创建一个设备控件卡片
 *
 * @param parent 父容器
 * @param device 设备数据体
 * @param row 网格行位置
 * @param col 网格列位置
 *
 * @return 设备控制卡片对象
 */
lv_obj_t *view_create_device_card(lv_obj_t *parent,
                                  const thing_device_t *device, uint8_t row,
                                  uint8_t col) {
  log_d("Creating card for device: %s at [%d, %d]", device->device_id, row,
        col);
  lv_obj_t *card = lv_obj_create(parent);
  lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));
  // 设置卡片的样式，如大小、圆角、阴影
  lv_obj_set_style_radius(card, 10, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(card, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_all(card, 6, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(card, 5, LV_PART_MAIN); // 子项边距
  lv_obj_clear_flag(card,
                    LV_OBJ_FLAG_SCROLLABLE); // 卡片本身不可滚动，内部容器滚动
  // 添加到网格布局
  lv_obj_set_grid_cell(card, LV_GRID_ALIGN_STRETCH, col, 1,
                       LV_GRID_ALIGN_STRETCH, row, 1);
  lv_obj_update_layout(card); // 更新布局信息,获取容器大小
  lv_coord_t img_size = 8 * lv_obj_get_height(card) / 26; // 图片缩放目标大小

  // 1. 设备图标
  lv_obj_t *img_icon = create_card_icon(card, device->icon, img_size, img_size);

  // 2. 设备名称 (滚动标签)
  lv_obj_t *name_label = lv_label_create(card);
  lv_label_set_text(name_label, device->name);
  lv_obj_set_style_text_font(name_label, &lv_font_montserrat_22, LV_PART_MAIN);
  lv_obj_set_width(name_label,
                   34 * lv_obj_get_width(card) / 50); // 限制宽度以触发滚动
  lv_label_set_long_mode(name_label, LV_LABEL_LONG_SCROLL); // 往复滚动模式
  lv_obj_align(name_label, LV_ALIGN_TOP_LEFT, img_size + 10, img_size / 3);
  log_w("%s name label width: %d", device->name,
        3 * lv_obj_get_width(card) / 5);

  // 3. 创建属性容器 (滚动区域)
  lv_obj_t *prop_cont = lv_obj_create(card);
  lv_obj_set_size(prop_cont, LV_PCT(105), LV_PCT(70)); // 占据剩余空间
  lv_obj_align(prop_cont, LV_ALIGN_BOTTOM_MID, 0, 6);
  lv_obj_set_flex_flow(prop_cont, LV_FLEX_FLOW_COLUMN); // 垂直Flex布局
  lv_obj_set_style_bg_opa(prop_cont, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_width(prop_cont, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(prop_cont, 0, LV_PART_MAIN); // 内边距
  lv_obj_set_style_pad_row(prop_cont, 5, LV_PART_MAIN); // 行间距
  //

  // 根据设备属性动态添加控件
  log_d("Generating %d controls for %s", device->prop_count, device->device_id);
  for (int i = 0; i < device->prop_count; i++) { // 遍历设备的所有属性
    const thing_property_t *prop = &device->properties[i];
    // 分配上下文空间
    control_event_ctx_t *ctx =
        (control_event_ctx_t *)lv_mem_alloc(sizeof(control_event_ctx_t));
    if (!ctx) {
      log_e("Memory alloc failed for control context [%s/%s]",
            device->device_id, prop->id);
      continue;
    }
    snprintf(ctx->deviceID, sizeof(ctx->deviceID), "%s", device->device_id);
    snprintf(ctx->propID, sizeof(ctx->propID), "%s", prop->id);

    // 根据属性类型创建控件
    switch (prop->type) {
    case THING_PROP_TYPE_SWITCH: {
      log_d("  Adding Switch: %s", prop->id);

      // 创建一个"行容器"来容纳"开关属性"的switch控件
      lv_obj_t *row_cont = lv_obj_create(prop_cont);
      lv_obj_set_size(row_cont, LV_PCT(100), 40); // 容器高40
      lv_obj_set_style_bg_opa(row_cont, LV_OPA_TRANSP, LV_PART_MAIN);
      lv_obj_set_style_border_width(row_cont, 0, LV_PART_MAIN);
      lv_obj_clear_flag(row_cont, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_update_layout(row_cont); // 更新布局信息
      // 属性名称
      lv_obj_t *label_sw = lv_label_create(row_cont);
      lv_label_set_text(label_sw, prop->name);
      lv_obj_set_style_text_font(label_sw, &lv_font_montserrat_16,
                                 LV_PART_MAIN);
      lv_obj_set_width(label_sw,
                       lv_obj_get_width(row_cont) / 2); // 限制宽度以触发滚动
      lv_label_set_long_mode(label_sw, LV_LABEL_LONG_SCROLL); // 往复滚动模式
      lv_obj_align(label_sw, LV_ALIGN_LEFT_MID, 0, 0);
      // 属性控件
      lv_obj_t *sw = lv_switch_create(row_cont);
      lv_obj_set_size(sw, 65, 30); // 手动留出边距
      lv_obj_align(sw, LV_ALIGN_RIGHT_MID, 0, 0);

      if (prop->value.b) {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
      }
      // 添加控件事件处理
      ctx->target_obj = sw;
      lv_obj_add_event_cb(sw, generic_control_event_cb, LV_EVENT_VALUE_CHANGED, ctx); // 值改变事件
      lv_obj_add_event_cb(sw, free_context_event_cb, LV_EVENT_DELETE, ctx); // 删除事件

      controller_register_ui_control(device->device_id, prop->id, sw); // 注册设备控件
      break;
    }
    case THING_PROP_TYPE_INT: {
      log_d("  Adding Slider: %s", prop->id);

      // 创建"垂直堆叠"的容器,int类型属性的slider控件
      lv_obj_t *col_cont = lv_obj_create(prop_cont);
      lv_obj_set_size(col_cont, LV_PCT(100), 50);
      lv_obj_set_style_bg_opa(col_cont, LV_OPA_TRANSP, LV_PART_MAIN);
      lv_obj_set_style_border_width(col_cont, 0, LV_PART_MAIN);
      lv_obj_clear_flag(col_cont, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_update_layout(col_cont); // 更新布局信息

      lv_obj_t *label_sld = lv_label_create(col_cont);
      lv_label_set_text(label_sld, prop->name);
      lv_obj_set_style_text_font(label_sld, &lv_font_montserrat_16, LV_PART_MAIN);
      lv_obj_align(label_sld, LV_ALIGN_TOP_LEFT, 0, -20);

      lv_obj_t *sld = simple_slider_create(lv_color_hex(0xFFE4B5), col_cont);
      lv_obj_set_size(sld, LV_PCT(100), 18);
      lv_obj_align(sld, LV_ALIGN_BOTTOM_MID, 0, 10);

      lv_slider_set_range(sld, prop->min, prop->max);
      lv_slider_set_value(sld, prop->value.i, LV_ANIM_OFF);

      ctx->target_obj = sld;
      lv_obj_add_event_cb(sld, generic_control_event_cb, LV_EVENT_VALUE_CHANGED, ctx);
      lv_obj_add_event_cb(sld, free_context_event_cb, LV_EVENT_DELETE, ctx);
      controller_register_ui_control(device->device_id, prop->id, sld);
      break;
    }
    case THING_PROP_TYPE_FLOAT: {
      log_d("  Adding Float Display: %s", prop->id);

      lv_obj_t *row_cont = lv_obj_create(prop_cont);
      lv_obj_set_size(row_cont, LV_PCT(100), 30);
      lv_obj_set_style_bg_opa(row_cont, LV_OPA_TRANSP, LV_PART_MAIN);
      lv_obj_set_style_border_width(row_cont, 0, LV_PART_MAIN);
      lv_obj_clear_flag(row_cont, LV_OBJ_FLAG_SCROLLABLE);

      lv_obj_t *label_name = lv_label_create(row_cont);
      lv_label_set_text(label_name, prop->name);
      lv_obj_set_style_text_font(label_name, &lv_font_montserrat_16, 0);
      lv_obj_align(label_name, LV_ALIGN_LEFT_MID, 0, 0);

      lv_obj_t *label_val = lv_label_create(row_cont);
      char buf[32];
      snprintf(buf, sizeof(buf), "%.1f %s", prop->value.f,
               prop->unit ? prop->unit : ""); // 单位
      lv_label_set_text(label_val, buf);
      lv_obj_set_style_text_font(label_val, &lv_font_montserrat_16, 0);
      lv_obj_align(label_val, LV_ALIGN_RIGHT_MID, 0, 0);

      ctx->target_obj = label_val; // 绑定到数值标签
      lv_obj_add_event_cb(label_val, free_context_event_cb, LV_EVENT_DELETE,
                          ctx);
      controller_register_ui_control(device->device_id, prop->id, label_val);
      break;
    }
    case THING_PROP_TYPE_STRING:
    case THING_PROP_TYPE_ENUM: {
      log_d("  Adding Label Display for type %d: %s", prop->type, prop->id);

      lv_obj_t *row_cont = lv_obj_create(prop_cont);
      lv_obj_set_size(row_cont, LV_PCT(100), 30);
      lv_obj_set_style_bg_opa(row_cont, LV_OPA_TRANSP, 0);
      lv_obj_set_style_border_width(row_cont, 0, 0);
      lv_obj_clear_flag(row_cont, LV_OBJ_FLAG_SCROLLABLE);

      lv_obj_t *label_name = lv_label_create(row_cont);
      lv_label_set_text(label_name, prop->name);
      lv_obj_set_style_text_font(label_name, &lv_font_montserrat_16, 0);
      lv_obj_align(label_name, LV_ALIGN_LEFT_MID, 0, 0);

      lv_obj_t *label_val = lv_label_create(row_cont);
      if (prop->type == THING_PROP_TYPE_STRING) {
        lv_label_set_text(label_val, prop->value.s ? prop->value.s : "N/A");
      } else {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", prop->value.i);
        lv_label_set_text(label_val, buf);
      }
      lv_obj_set_style_text_font(label_val, &lv_font_montserrat_16, 0);
      lv_obj_align(label_val, LV_ALIGN_RIGHT_MID, 0, 0);

      ctx->target_obj = label_val;
      lv_obj_add_event_cb(label_val, free_context_event_cb, LV_EVENT_DELETE,
                          ctx);

      controller_register_ui_control(device->device_id, prop->id, label_val);
      break;
    }
    default:
      log_w("  Skipping unsupported property type %d for %s", prop->type,
            prop->id);
      lv_mem_free(ctx);
      break;
    }
  }

  return card;
}

/**
 * @brief 创建一个分散式的单个属性控件卡片
 */
lv_obj_t *view_create_property_card(lv_obj_t *parent,
                                    const thing_device_t *device,
                                    uint32_t prop_index, uint8_t row,
                                    uint8_t col) {
  if (prop_index >= device->prop_count)
    return NULL;

  const thing_property_t *prop = &device->properties[prop_index];
  log_d("Creating dispersed card for [%s/%s] at [%d, %d]", device->device_id,
        prop->id, row, col);

  lv_obj_t *card = lv_obj_create(parent);
  lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_radius(card, 15, 0);
  lv_obj_set_style_shadow_width(card, 8, 0);
  lv_obj_set_style_pad_all(card, 12, 0);
  lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_grid_cell(card, LV_GRID_ALIGN_STRETCH, col, 1,
                       LV_GRID_ALIGN_STRETCH, row, 1);

  // 1. Icon
  lv_obj_update_layout(card);
  lv_coord_t img_size = 8 * lv_obj_get_height(card) / 30; // 图片缩放目标大小
  lv_obj_t *img_icon = create_card_icon(card, device->icon, img_size, img_size);
  lv_obj_align(img_icon, LV_ALIGN_TOP_LEFT, -img_size / 6, -img_size / 6);

  // 2. 设备名称和属性 (滚动标签)
  lv_obj_t *name_label = lv_label_create(card);
  lv_label_set_text(name_label, device->name);
  lv_obj_set_style_text_font(name_label, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_set_width(name_label, 34 * lv_obj_get_width(card) /
                                   img_size); // 限制宽度以触发滚动
  lv_label_set_long_mode(name_label, LV_LABEL_LONG_SCROLL); // 往复滚动模式
  lv_obj_align(name_label, LV_ALIGN_TOP_LEFT, img_size, img_size / 5);
  lv_obj_t *prop_label = lv_label_create(card);
  lv_label_set_text(prop_label, prop->name);
  lv_obj_set_style_text_font(prop_label, &lv_font_montserrat_24, LV_PART_MAIN);
  lv_obj_align(prop_label, LV_ALIGN_BOTTOM_MID, 0, 0);

  // 3. 创建上下文
  control_event_ctx_t *ctx =
      (control_event_ctx_t *)lv_mem_alloc(sizeof(control_event_ctx_t));
  if (ctx) {
    snprintf(ctx->deviceID, sizeof(ctx->deviceID), "%s", device->device_id);
    snprintf(ctx->propID, sizeof(ctx->propID), "%s", prop->id);
    lv_obj_add_event_cb(card, free_context_event_cb, LV_EVENT_DELETE, ctx);
  }

  // 4. 创建ui控件
  lv_obj_t *ctrl = NULL;
  switch (prop->type) {
  case THING_PROP_TYPE_SWITCH:
    ctrl = lv_switch_create(card);
    lv_obj_set_size(ctrl, LV_PCT(42), 40);
    lv_obj_center(ctrl);
    if (prop->value.b)
      lv_obj_add_state(ctrl, LV_STATE_CHECKED);
    break;

  case THING_PROP_TYPE_INT:
    ctrl = simple_slider_create(lv_color_hex(0x00AEEF), card);
    lv_obj_set_size(ctrl, LV_PCT(95), 30);
    lv_obj_center(ctrl);
    lv_slider_set_range(ctrl, prop->min, prop->max);
    lv_slider_set_value(ctrl, prop->value.i, LV_ANIM_OFF);
    break;

  case THING_PROP_TYPE_FLOAT:
    ctrl = lv_label_create(card);
    lv_obj_set_style_text_font(ctrl, &lv_font_montserrat_32, 0);
    {
      char buf[32];
      snprintf(buf, sizeof(buf), "%.1f %s", prop->value.f,
               prop->unit ? prop->unit : "");
      lv_label_set_text(ctrl, buf);
    }
    lv_obj_center(ctrl);
    break;

  case THING_PROP_TYPE_STRING:
  case THING_PROP_TYPE_ENUM:
    ctrl = lv_label_create(card);
    lv_obj_set_style_text_font(ctrl, &lv_font_montserrat_32, 0);
    if (prop->type == THING_PROP_TYPE_STRING) {
      lv_label_set_text(ctrl, prop->value.s ? prop->value.s : "N/A");
    } else {
      char buf[16];
      snprintf(buf, sizeof(buf), "%d", prop->value.i);
      lv_label_set_text(ctrl, buf);
    }
    lv_obj_center(ctrl);
    break;

  default:
    ctrl = lv_label_create(card);
    lv_label_set_text(ctrl, "Unsupported");
    lv_obj_center(ctrl);
    break;
  }

  if (ctrl && ctx) {
    ctx->target_obj = ctrl;
    lv_obj_add_event_cb(ctrl, generic_control_event_cb, LV_EVENT_VALUE_CHANGED,
                        ctx);
    controller_register_ui_control(device->device_id, prop->id, ctrl);
  }

  return card;
}

#endif
