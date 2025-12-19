#include "ui_sys_panel.h"
#include "input_manager.h"
#include "sys_state.h"
#include "ui_helpers.h"

#define LOG_TAG "UI_SYS_PANEL"
#include "elog.h"

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
static lv_obj_t *panel_bg = NULL;      // 下拉菜单手势识别和遮罩区域
static lv_obj_t *panel_content = NULL; // 下拉菜单主体
static bool is_panel_active = false;   // 菜单状态

/*********************
 *  STATIC PROTOTYPES
 *********************/
static void event_panel_drag_cb(lv_event_t *e);    // 拖动回调
static void delete_anim_ready_cb(lv_anim_t *anim); // 删除动画完成回调
static void event_vol_cb(lv_event_t *e);           // 音量调节回调
static void event_bri_cb(lv_event_t *e);           // 亮度调节回调
static void event_wifi_cb(lv_event_t *e);          // WiFi开关回调

/*********************
 *   STYLE HELPERS
 *********************/

// 设置容器样式和属性(纯粹背景)
static void container_style_set(lv_obj_t *obj)
{
  lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN); // 阴影
  lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN); // 边框
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);      // 不可滚动
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);       // 不可点击，防止遮挡手势
}

/*********************
 *   CREATE HELPERS
 *********************/
/**
 * @brief 下拉菜单子项容器创建(不可见,不可点击,不可滚动)
 * @param Parent  父类
 * @param x       x坐标
 * @param span_x  x跨度
 * @param y       y坐标
 * @param span_y  y跨度
 * @return       返回一个容器块背景
 */
static lv_obj_t *menu_child_container_create(lv_obj_t *parent, uint8_t col,
                                             uint8_t col_span, uint8_t row,
                                             uint8_t row_span)
{
  // 块容器(不可见,用于对齐组件)
  lv_obj_t *block = lv_obj_create(parent);
  // lv_obj_set_style_opa(block, 0, LV_PART_MAIN); //不要让整体透明，
  // 这会影响子项
  lv_obj_set_style_bg_opa(block, 0, LV_PART_MAIN);
  container_style_set(block);
  lv_obj_set_grid_cell(block, LV_GRID_ALIGN_STRETCH, col, col_span,
                       LV_GRID_ALIGN_STRETCH, row, row_span); // x-y拉伸对齐

  // 子组件背景(可见用于样式)
  lv_obj_t *block_bg = lv_obj_create(block);
  lv_obj_update_layout(block_bg);
  lv_obj_set_size(block_bg, lv_obj_get_width(block) - 20,
                  lv_obj_get_height(block) - 20);
  lv_obj_align(block_bg, LV_ALIGN_CENTER, 0, 0); // 子项对齐,中心
  container_style_set(block_bg);
  lv_obj_set_style_radius(block_bg, 30, LV_PART_MAIN); // 圆角
  lv_obj_set_style_bg_color(block_bg, lv_color_hex(0x828282), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(block_bg, 200, LV_PART_MAIN);

  lv_obj_update_layout(block_bg); // 更新信息

  return block_bg;
}

/* -- 子组件创建 -- */
static lv_obj_t *multimedia_module_create(lv_obj_t *parent, uint8_t col,
                                          uint8_t col_span, uint8_t row,
                                          uint8_t row_span)
{
  lv_obj_t *multimedia =
      menu_child_container_create(parent, col, col_span, row, row_span);
  // 暂定

  return multimedia;
}

static lv_obj_t *brightness_module_create(lv_obj_t *parent, uint8_t value,
                                          uint8_t col, uint8_t col_span,
                                          uint8_t row, uint8_t row_span)
{
  lv_obj_t *brightness =
      menu_child_container_create(parent, col, col_span, row, row_span);

  // slider作为主体
  lv_obj_t *slider_bri = lv_slider_create(brightness);
  // 样式
  lv_obj_set_size(slider_bri, lv_obj_get_width(brightness), lv_obj_get_height(brightness));
  lv_obj_center(slider_bri);
  lv_obj_remove_style(slider_bri, NULL, LV_PART_KNOB);
  lv_obj_set_style_radius(slider_bri, 30, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(slider_bri, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(slider_bri, 30, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider_bri, lv_color_hex(0xF0F0F0),
                            LV_PART_INDICATOR);
  // 值
  lv_slider_set_range(slider_bri, 0, 100);                                     // 设置范围
  lv_slider_set_value(slider_bri, value, LV_ANIM_OFF);                         // 初始值
  lv_obj_add_event_cb(slider_bri, event_bri_cb, LV_EVENT_VALUE_CHANGED, NULL); // 回调(值改变)
  lv_obj_add_event_cb(slider_bri, event_bri_cb, LV_EVENT_PRESSED, NULL);       // 回调(按下)
  lv_obj_add_event_cb(slider_bri, event_bri_cb, LV_EVENT_RELEASED, NULL);      // 回调(松开)

  // 图标
  lv_obj_t *brightness_icon =
      lv_img_create(slider_bri); // slider_bri作为父类，因为icon需要在回调中获取
  lv_img_set_src(brightness_icon, &icon_bright);
  lv_obj_align(brightness_icon, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_img_recolor(brightness_icon, lv_color_hex(0x505050), 0);
  lv_opa_t bri_opa = lv_map(value, 0, 100, LV_OPA_COVER, LV_OPA_TRANSP);
  lv_obj_set_style_img_recolor_opa(brightness_icon, bri_opa, 0);

  return brightness;
}

static lv_obj_t *volume_module_create(lv_obj_t *parent, uint8_t value,
                                      uint8_t col, uint8_t col_span,
                                      uint8_t row, uint8_t row_span)
{
  lv_obj_t *volume =
      menu_child_container_create(parent, col, col_span, row, row_span);
  // slider作为主体
  lv_obj_t *slider_vol = lv_slider_create(volume);
  // 样式
  lv_obj_set_size(slider_vol, lv_obj_get_width(volume), lv_obj_get_height(volume));
  lv_obj_center(slider_vol);
  lv_obj_remove_style(slider_vol, NULL, LV_PART_KNOB);
  lv_obj_set_style_radius(slider_vol, 30, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(slider_vol, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(slider_vol, 30, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider_vol, lv_color_hex(0xF0F0F0),
                            LV_PART_INDICATOR);
  lv_slider_set_range(slider_vol, 0, 100);
  // 值
  lv_slider_set_value(slider_vol, value, LV_ANIM_OFF);
  lv_obj_add_event_cb(slider_vol, event_vol_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // 图标(使用lvgl内部label图标)
  lv_obj_t *lbl_vol = lv_label_create(slider_vol);
  // 创建值对应图标
  char icon[4];
  if (value == 0)
  {
    sprintf(icon, LV_SYMBOL_MUTE);
  }
  else if (value < 50)
  {
    sprintf(icon, LV_SYMBOL_VOLUME_MID);
  }
  else
  {
    sprintf(icon, LV_SYMBOL_VOLUME_MAX);
  }
  lv_label_set_text(lbl_vol, icon);
  lv_obj_set_style_text_font(lbl_vol, &lv_font_montserrat_30, 0);
  lv_obj_align(lbl_vol, LV_ALIGN_BOTTOM_MID, 0, -20);

  return volume;
}

static lv_obj_t *wifi_module_create(lv_obj_t *parent, uint8_t value,
                                    uint8_t col, uint8_t col_span, uint8_t row,
                                    uint8_t row_span)
{
  lv_obj_t *wifi =
      menu_child_container_create(parent, col, col_span, row, row_span);
  // btn作为主体
  lv_obj_t *btn_wifi = lv_btn_create(wifi);
  // 样式
  lv_obj_set_size(btn_wifi, lv_obj_get_width(wifi), lv_obj_get_height(wifi));
  lv_obj_center(btn_wifi);
  lv_obj_set_style_radius(btn_wifi, 30, LV_PART_MAIN);
  lv_obj_add_flag(btn_wifi, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_style_bg_color(btn_wifi, lv_color_hex(0x828282),
                            LV_STATE_DEFAULT); // 默认状态
  lv_obj_set_style_bg_color(btn_wifi, lv_color_hex(0x1E90FF),
                            LV_STATE_CHECKED); // 选中状态
  if (value)
    lv_obj_add_state(btn_wifi, LV_STATE_CHECKED);
  lv_obj_add_event_cb(btn_wifi, event_wifi_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // icon
  lv_obj_t *img_wifi = lv_img_create(btn_wifi);
  lv_img_set_src(img_wifi, &icon_wifi);
  lv_obj_center(img_wifi);
  lv_img_set_zoom(img_wifi, 300);
  lv_obj_set_style_img_recolor(img_wifi, lv_color_hex(0x333333), 0);
  lv_obj_set_style_img_recolor_opa(img_wifi, LV_OPA_COVER, 0);

  return wifi;
}

/**
 * @brief 创建菜单主体和其中内容
 * @param parent 传入panel_bg作为父对象
 * @return 返回菜单主体
 */
static lv_obj_t *ui_sys_panel_content_create(lv_obj_t *parent)
{
  // 创建一个内容容器
  lv_obj_t *content_container = lv_obj_create(parent);
  // 初始时让它和父容器一样大，但要把它放在屏幕外
  lv_obj_set_size(content_container, scr_act_width(), scr_act_height());
  lv_obj_set_y(content_container, -scr_act_height()); // 完全隐藏在屏幕上方
  /* 主体样式 */
  lv_obj_set_style_bg_color(content_container, lv_color_hex(0x000000),
                            LV_PART_MAIN);
  lv_obj_set_style_bg_opa(content_container, 80,
                          LV_PART_MAIN); // 下拉菜单背景透明度
  container_style_set(content_container);
  // 设置网格布局
  static lv_coord_t col_dsc[] = {
      LV_GRID_FR(1), 98, 98, 98, 98, 98, 98, 98, LV_GRID_FR(1),
      LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {
      LV_GRID_FR(1), 98, 98, 98, 98, LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(content_container, col_dsc, row_dsc);
  lv_obj_set_style_pad_all(content_container, 0,
                           0);                         // 边距(是整体边距,不是子项间的距离)
  lv_obj_set_style_pad_gap(content_container, 20, 20); // 子项边距

  // 底部拖拽区
  lv_obj_t *obj_line = lv_obj_create(content_container);
  lv_obj_set_style_bg_color(obj_line, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(
      obj_line, 40,
      LV_PART_MAIN); // 注意子项和父项重叠区opa会叠加,实际上是在设置子比父深多少
  container_style_set(obj_line);
  lv_obj_set_grid_cell(obj_line, LV_GRID_ALIGN_STRETCH, 0, 9,
                       LV_GRID_ALIGN_STRETCH, 5, 1);
  static lv_point_t line_points[] = {{0, 0}, {60, 0}};
  lv_obj_t *line = lv_line_create(obj_line);
  lv_line_set_points(line, line_points, 2);           // 设置线条坐标点,创建线条
  lv_obj_align(line, LV_ALIGN_CENTER, 0, 0);          // 设置位置
  lv_obj_set_style_line_width(line, 5, LV_PART_MAIN); // 设线的宽度
  lv_obj_set_style_line_color(line, lv_color_hex(0x353535),
                              LV_PART_MAIN);               // 线的颜色
  lv_obj_set_style_line_rounded(line, true, LV_PART_MAIN); // 设置线条圆角
  lv_obj_set_grid_cell(line, LV_GRID_ALIGN_STRETCH, 5, 1, LV_GRID_ALIGN_STRETCH,
                       5, 1);

  // 获取系统状态
  const sys_state_t *state = sys_state_get();

  /* 播放块 */
  multimedia_module_create(content_container, 1, 2, 1, 2);
  /* 亮度滑块 */
  brightness_module_create(content_container, state->brightness, 3, 1, 1, 2);
  /* 音量滑块 */
  volume_module_create(content_container, state->volume, 4, 1, 1, 2);
  /* WiFi 按钮 */
  wifi_module_create(content_container, state->wifi_connected, 5, 1, 1, 1);

  // #ifndef NODEBUG
  //// 布局测试
  // #include "components/util.h"
  //   test_layout_grid(content_container, , );
  // #endif

  return content_container; // 添加返回语句
}

/*********************
 *   SET HELPERS
 *********************/

// 动画设置辅助
static lv_anim_t *slide_anim_set(lv_obj_t *obj, lv_coord_t start,
                                 lv_coord_t end)
{
  static lv_anim_t a; // 必须静态
  lv_anim_init(&a);
  lv_anim_set_var(&a, obj);
  lv_anim_set_values(&a, start, end);
  lv_anim_set_time(&a, 400);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
  return &a;
}

/**
 * @brief
 * 聚焦模式,隐藏指定级父项下的所有对象,排除聚焦子项和顶级项下它的所有父项(
 * 适用于不知道父项地址 )
 *
 * @param focused_bg 聚焦对象
 * @param Top_parent 父项级
 * @param enable     此函数操作可还原
 */
static void pd_menu_set_focus_mode(lv_obj_t *focused_bg,
                                   uint8_t Top_parent_level, bool enable)
{
  if (focused_bg == NULL)
    return;
  lv_obj_t *Top_parent = NULL;
  lv_obj_t *except = focused_bg;
  // 获取指定级级父容器
  while (Top_parent_level)
  {
    Top_parent = lv_obj_get_parent(except);
    Top_parent_level--;
    if (Top_parent_level == 0 || Top_parent == NULL)
    {
      break;
    }
    // 继续向上
    except = Top_parent;
  }
  if (Top_parent == NULL && Top_parent_level > 0)
    return; // 级别错误

  // 隐藏顶级父项,和下拉菜单遮罩
  if (enable)
  {
    lv_obj_set_style_bg_opa(Top_parent, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel_bg, 0, LV_PART_MAIN);
  }
  else
  {
    lv_obj_set_style_bg_opa(Top_parent, 80,
                            LV_PART_MAIN); // 这里获取到的是菜单背景
    lv_obj_set_style_bg_opa(
        panel_bg, 150,
        LV_PART_MAIN); // 调节亮度时菜单一定是完全展开态,值是确定的
  }

  // 遍历 Top_parent 下的所有子对象
  uint32_t child_count = lv_obj_get_child_cnt(Top_parent); // 子项数量
  log_d("focus_mode:child_count=%d", child_count);
  for (uint32_t i = 0; i < child_count; i++)
  {
    lv_obj_t *child = lv_obj_get_child(Top_parent, i);
    // 如果这个子对象不是我们想要聚焦的那个
    if (child != except)
    {
      if (enable)
      {
        // 【开启聚焦】：隐藏其他所有功能块
        lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
      }
      else
      {
        // 【关闭聚焦】：恢复显示其他所有功能块
        lv_obj_clear_flag(child, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
}

/*********************
 *      FUNCTIONS
 *********************/
/**
 * @brief 显示系统菜单
 *
 * 绑定于全局下拉手势
 */
void ui_sys_panel_show(void)
{
  if (is_panel_active || panel_bg != NULL)
  {
    return;
  }
  is_panel_active = true;

  // 创建遮罩层(此层负责跟踪滑动手势和渐变遮罩效果)
  panel_bg = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(panel_bg);
  lv_obj_set_size(panel_bg, scr_act_width(), scr_act_height());
  lv_obj_set_style_bg_color(panel_bg, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(panel_bg, LV_OPA_0, 0); // 刚开始透明
  lv_obj_clear_flag(panel_bg, LV_OBJ_FLAG_SCROLLABLE);

  // 创建内容
  panel_content = ui_sys_panel_content_create(panel_bg); // 使用完整实现而非测试代码

  if (!panel_content)
  {
    lv_obj_del(panel_bg);
    log_e("ui_sys_panel_show: panel_content create failed");
    return;
  }

  // 设置用户数据和事件回调
  lv_obj_set_user_data(panel_bg, panel_content);
  lv_obj_add_event_cb(panel_bg, event_panel_drag_cb, LV_EVENT_ALL, NULL);
}

/**
 * @brief 关闭系统菜单(外部调用,进入关闭判断)
 *
 * 绑定于全局上拉手势,此函数不负责关闭菜单,只负责进入 event_panel_drag_cb
 * 判断是否关闭
 */
static void ui_sys_panel_begin_close(void)
{
  if (!is_panel_active || !panel_bg || !panel_content)
    return;
  log_d("pulldown_menu_begin_close");

  // 重新为菜单背景绑定事件处理器，让它能再次响应拖动
  lv_obj_add_event_cb(panel_bg, event_panel_drag_cb, LV_EVENT_ALL, NULL);

  // "吸附"到手指：让菜单的下边沿立刻跟随手指
  lv_coord_t content_h = lv_obj_get_height(panel_content);
  lv_point_t p;
  lv_indev_get_point(lv_indev_get_act(), &p);
  lv_obj_set_y(panel_content, p.y - content_h); // 下边沿吸附

  // 添加这一行来重置输入设备，使手势能够继续跟踪
  lv_indev_reset(lv_indev_get_act(), NULL);
}

bool ui_sys_panel_is_visible(void) { return is_panel_active; }

/*********************
 *   事件回调
 *********************/

// 动画结束回调(下拉菜单删除)
static void delete_anim_ready_cb(lv_anim_t *anim)
{
  lv_obj_del(panel_bg);
  panel_bg = NULL;
  panel_content = NULL;
  is_panel_active = false;
}

static void event_panel_drag_cb(lv_event_t *e)
{
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

  lv_coord_t content_h = lv_obj_get_height(content);
  lv_coord_t current_y = lv_obj_get_y(content);

  lv_coord_t threshold = -(content_h / 2); // 最大位置阈值
  uint8_t opa = 0;
  if (current_y >= threshold)
    opa = 150; // 最大遮罩
  else
    opa = lv_map(current_y, -content_h, threshold, 0, 150); // 线性变化

  // 如果菜单还在从完全隐藏到一半的过程中，则进行线性映射
  // 映射到 opa 从 [0, 150] 的范围

  lv_obj_set_style_bg_opa(bg, opa, 0);

  lv_anim_t *a;
  // 事件1：当手指在背景上拖动时
  if (code == LV_EVENT_PRESSING)
  {
    // 将菜单下部对齐当前点y坐标
    lv_obj_set_y(content, p.y - content_h); /* 根据按下点计算控件左上角的位置 */
  }
  // 事件2：当手指松开时
  else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
  {
    lv_coord_t now_y = lv_obj_get_y(content); // 获取控件当前y
    // lv_coord_t final_y = p.y - ; //计算菜单y坐标(控件的坐标在左上角)
    //  滑入还是滑出
    if (p.y > scr_act_height() / 2)
    { // 松手点在屏幕下部1/3
      // --- 自动滑入 ---
      a = slide_anim_set(content, now_y, 0);
      lv_anim_set_path_cb(a, lv_anim_path_ease_out);
      // 动画结束后，解除拖动事件回调
      lv_obj_remove_event_cb(bg, event_panel_drag_cb);
      lv_anim_start(a);
    }
    else
    {
      // --- 自动滑出并销毁 ---
      a = slide_anim_set(content, now_y, -(content_h + 10));
      lv_anim_set_path_cb(a, lv_anim_path_ease_in);
      // 动画结束后，调用删除整个菜单的函数
      lv_anim_set_ready_cb(a, delete_anim_ready_cb);
      lv_anim_start(a);
    }
  }
}

/* -- 子组件相关 -- */
// 音量滑块事件
static void event_vol_cb(lv_event_t *e)
{
  lv_obj_t *slider = lv_event_get_target(e);
  uint8_t val = (uint8_t)lv_slider_get_value(slider);

  // 设置系统状态
  sys_state_set_volume(val);

  // Update Icon
  lv_obj_t *lbl = lv_obj_get_child(slider, 0);
  if (lbl)
  {
    if (val = 0)
      lv_label_set_text(lbl, LV_SYMBOL_MUTE);
    else if (val < 50)
      lv_label_set_text(lbl, LV_SYMBOL_VOLUME_MID);
    else
      lv_label_set_text(lbl, LV_SYMBOL_VOLUME_MAX);
  }
}

// 亮度滑块事件
static void event_bri_cb(lv_event_t *e)
{
  lv_obj_t *slider = lv_event_get_target(e);
  lv_event_code_t event_code = lv_event_get_code(e); // 事件代码

  // 事件1：当滑块值改变时，更新图标的染色效果
  if (event_code == LV_EVENT_VALUE_CHANGED)
  {
    int32_t value = lv_slider_get_value(slider);         // 滑块值
    lv_obj_t *bright_icon = lv_obj_get_child(slider, 0); // 图标
    log_d("bright_slider_value:%d", value);
    // 设置系统状态
    sys_state_set_brightness(value);

    if (bright_icon == NULL)
    {
      return;
    }
    // Update Icon
    lv_opa_t mapped_opa = lv_map(value, 0, 100, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_obj_set_style_img_recolor_opa(bright_icon, mapped_opa, LV_PART_MAIN);
  }
  // 事件2：当手指按下时，开启聚焦模式
  else if (event_code == LV_EVENT_PRESSED)
  {
    log_d("bright: Focus Mode");
    pd_menu_set_focus_mode(slider, 3, true); // 聚焦
  }
  // 事件3：当手指松开或移出时，关闭聚焦模式
  else if (event_code == LV_EVENT_RELEASED ||
           event_code == LV_EVENT_PRESS_LOST)
  {
    log_d("bright: Exit Focus Mode");
    pd_menu_set_focus_mode(slider, 3, false); // 恢复
  }
}

// WiFi 按钮事件
static void event_wifi_cb(lv_event_t *e)
{
  lv_obj_t *btn = lv_event_get_target(e);
  bool checked = lv_obj_has_state(btn, LV_STATE_CHECKED);

  // 设置WiFi状态
  sys_state_set_wifi(checked);

  // Update Icon
  lv_obj_t *img = lv_obj_get_user_data(btn);
  if (img)
  {
    if (checked)
      lv_obj_set_style_img_recolor(img, lv_color_hex(0xEEEEEE), 0);
    else
      lv_obj_set_style_img_recolor(img, lv_color_hex(0x333333), 0);
  }
}

// --- Gesture Callbacks ---

static void on_pulldown_gesture(void)
{
  log_d("UiSysPanel: Pulldown gesture received. Panel visible: %d",
         ui_sys_panel_is_visible());
  // 只有在面板不可见时才显示
  if (!ui_sys_panel_is_visible())
  {
    ui_sys_panel_show();
  }
}

static void on_close_gesture(void)
{
  log_d("UiSysPanel: Close gesture received. Panel visible: %d",
         ui_sys_panel_is_visible());
  // 只有在面板可见时才处理关闭
  if (ui_sys_panel_is_visible())
  {
    ui_sys_panel_begin_close();
  }
}

void ui_sys_panel_init(void)
{
  // 注册下拉手势 (顶部下滑)
  input_manager_register_callback(GESTURE_TOP_SWIPE_DOWN, on_pulldown_gesture);

  // 注册关闭手势 (底部按下)
  input_manager_register_callback(GESTURE_BOTTOM_PRESS, on_close_gesture);
}
