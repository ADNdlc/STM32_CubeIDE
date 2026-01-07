#include "style_util.h"

static lv_style_t base_style_default;
static lv_style_t base_style_checked;

static lv_style_t tab_add_style_default;
static lv_style_t tab_add_style_checked;

static lv_style_t style_gradient_top;
static lv_style_t style_gradient_bottom;

//关闭切换动画回调
static void anim_scroll_disable_cb(lv_event_t* e){
    // 从事件参数中获取动画描述符
    lv_anim_t* a = (lv_anim_t*)lv_event_get_param(e);
    // 检查动画描述符是否存在，并将其动画时间设置为 0
    if (a) { a->time = 0;}
}

/**
 * @brief 主体样式设置
 * @param  NULL
 */
static void set_basetab_style(void) {
    lv_style_init(&base_style_default);
    lv_style_init(&base_style_checked);
    /* 未选中的按钮 */
    lv_style_set_bg_color(&base_style_default, lv_color_hex(0xcfcfcf)); //滤镜,ITEMS是指部件有多个类似组成部件
    lv_style_set_bg_opa(&base_style_default, 200);                      //透明度
    lv_style_set_text_font(&base_style_default, &lv_font_montserrat_20); //设置字体大小
    lv_style_set_text_color(&base_style_default, lv_color_hex(0x4f4f4f)); //未选中的按钮文本

    /*选中的按钮*/
    lv_style_set_bg_color(&base_style_checked, lv_color_hex(0xe5e5e5));   //设置CHECKED按下状态的样式,与主体背景同色
    lv_style_set_bg_opa(&base_style_checked, 200);
    lv_style_set_text_font(&base_style_checked, &lv_font_montserrat_24); //设置字体大小
    lv_style_set_text_color(&base_style_checked, lv_color_hex(0x4169E1));//选中的按钮文本,蓝
}
/**
 * @brief 添加页面样式设置
 * @param  NULL
 */
static void set_addtab_style(void) {
    lv_style_init(&tab_add_style_default);
    lv_style_init(&tab_add_style_checked);
    /* 未选中的按钮 */
    lv_style_set_bg_color(&tab_add_style_default, lv_color_hex(0xcfcfcf)); //滤镜,ITEMS是指部件有多个类似组成部件,灰
    lv_style_set_bg_opa(&tab_add_style_default, 200);                      //透明度
    lv_style_set_text_font(&tab_add_style_default, &lv_font_montserrat_18); //设置字体大小
    lv_style_set_text_color(&tab_add_style_default, lv_color_hex(0x4f4f4f)); //未选中的按钮文本,黑灰
    /*选中的按钮*/
    lv_style_set_bg_color(&tab_add_style_checked, lv_color_hex(0xd2d2d2));   //设置CHECKED按下状态的样式
    lv_style_set_bg_opa(&tab_add_style_checked, 200);
    lv_style_set_text_font(&tab_add_style_checked, &lv_font_montserrat_20); //设置字体大小
    lv_style_set_text_color(&tab_add_style_checked, lv_color_hex(0x4169E1));//选中的按钮文本,蓝
}

static void set_gradient_style(void) {
    lv_style_init(&style_gradient_top);
    lv_style_init(&style_gradient_bottom);

    /*Make a gradient*/
    lv_style_set_bg_opa(&style_gradient_top, LV_OPA_COVER);
    static lv_grad_dsc_t grad_top;
    grad_top.dir = LV_GRAD_DIR_VER;
    grad_top.stops_count = 2;
    grad_top.stops[0].color = lv_color_hex(0xf5f5f5);  // 顶部与背景色相同
    grad_top.stops[1].color = lv_color_hex(0xe0e0e0);  // 轻微变暗
    /* 设置渐变位置，让过渡更平滑 */
    grad_top.stops[0].frac = 0;    // 顶部开始
    grad_top.stops[1].frac = 255;  // 底部结束

    lv_style_set_bg_opa(&style_gradient_bottom, LV_OPA_COVER);
    static lv_grad_dsc_t grad_bottom;
    grad_bottom.dir = LV_GRAD_DIR_VER;
    grad_bottom.stops_count = 2;
    grad_bottom.stops[0].color = lv_color_hex(0xe0e0e0);  // 顶部与上半部分底部相同
    grad_bottom.stops[1].color = lv_color_hex(0xf5f5f5);  // 底部与背景色相同
    /* 设置渐变位置，让过渡更平滑 */
    grad_bottom.stops[0].frac = 0;    // 顶部开始
    grad_bottom.stops[1].frac = 255;  // 底部结束

    lv_style_set_bg_grad(&style_gradient_top, &grad_top);
    lv_style_set_bg_grad(&style_gradient_bottom, &grad_bottom);
}


void style_init(void) {
    set_basetab_style();
    set_addtab_style();
    set_gradient_style();
}

void style_deinit(void) {
    lv_style_reset(&base_style_default);
    lv_style_reset(&base_style_checked);
    lv_style_reset(&tab_add_style_default);
    lv_style_reset(&tab_add_style_checked);
}

/**
 * @brief 平面化tabview样式(关闭了动画)
 * @param tabview  选项卡对象
 * @param default  默认时btn组件样式
 * @param checked  按下时btn组件样式
 */
void style_tabview_simple(lv_obj_t* tabview, lv_style_t* _default, lv_style_t* _checked) {
    /* ===== 动画 ===== */
    // 关闭切换动画
    lv_obj_t* content = lv_tabview_get_content(tabview);
    lv_obj_add_event_cb(content, anim_scroll_disable_cb, LV_EVENT_SCROLL_BEGIN, NULL);
    // 关闭滑动切换
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    /* ===== 按钮部分 ===== */
    lv_obj_t* btn = lv_tabview_get_tab_btns(tabview);
    //设置样式
    lv_obj_add_style(btn, _default, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_add_style(btn, _checked, LV_PART_ITEMS | LV_STATE_CHECKED);

    //去除选中边框
    lv_obj_set_style_border_width(btn, 0, LV_PART_ITEMS | LV_STATE_CHECKED);
    //取消聚焦
    lv_obj_clear_state(btn, LV_STATE_FOCUS_KEY);
}


void obj_align_card(lv_obj_t* obj, uint8_t count, lv_coord_t width, lv_coord_t height) {
    static uint8_t i;
    i++;
    if (count == 1) {
        lv_obj_align(obj, LV_ALIGN_CENTER, width / 8, height/15);
    }
    else if (count == 2) {
        lv_obj_align(obj, LV_ALIGN_TOP_MID, width / 8, (height/4)*i + 10);
    }
    else{
        lv_obj_align(obj, LV_ALIGN_TOP_MID, width / 8, (height/5)*i + 4);
    }
    if ( i >= count) {i = 0;}
}

lv_style_t* style_get_base_default(void) {
    return &base_style_default;
}
lv_style_t* style_get_base_checked(void) {
    return &base_style_checked;
}
lv_style_t* style_get_addtab_default(void) {
    return &tab_add_style_default;
}
lv_style_t* style_get_addtab_checked(void) {
    return &tab_add_style_checked;
}
lv_style_t* style_get_gradient_style_top(void) {
    return &style_gradient_top;
}
lv_style_t* style_get_gradient_style_bottom(void) {
    return &style_gradient_bottom;
}


