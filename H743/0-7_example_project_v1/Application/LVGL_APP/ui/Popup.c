#include "Popup.h"
#include "util/util.h"

LV_IMG_DECLARE(icon_bright); // 亮度滑块图标
LV_IMG_DECLARE(icon_wifi);   // WiFi按钮图标

// 弹窗实例结构体 (链表节点,代表一个显示中的弹窗实例)
typedef struct
{
    lv_obj_t *obj;                // 指向弹窗的LVGL对象
    bool is_modal;                // 是否阻挡操作
    uint32_t dismiss_at_tick;     // 自动销毁的时刻,0表示永不
    popup_close_cb_t on_close_cb; // 关闭回调
    void *user_data;              // 自定义数据
} popup_instance_t;

/* ============================================= 私有变量 ============================================= */
static popup_instance_t G_active_popups[POPUP_REGION_COUNT]; // 弹窗实例列表,索引代表屏幕区域
static lv_obj_t *G_modal_curtain = NULL;                     // 遮罩幕布,用于模态
static lv_timer_t *G_task_timer = NULL;                      // 计时器

/* 下拉菜单相关 */
static lv_obj_t *G_pd_menu = NULL;       // 下拉菜单手势识别和遮罩区域。
static bool G_is_pd_menu_active = false; // 活动状态
// 获取菜单活动状态
bool popup_is_pd_menu_active(void)
{
    return G_is_pd_menu_active;
}

/* ============================================= 私有函数 ============================================= */
// 回调函数前向声明
static void bright_event_cb(lv_event_t *e); // 亮度滑块回调
static void volume_event_cb(lv_event_t *e); // 音量滑块回调
static void WiFi_event_cb(lv_event_t *e);   // WiFi按钮回调

static void pd_menu_event_cb(lv_event_t *e);               // 下拉菜单运动动画回调
static void pd_menu_delete_anim_ready_cb(lv_anim_t *anim); // 下拉菜单关闭回调
static void delete_obj_anim_ready_cb(lv_anim_t *anim);     // 动画结束删除对象回调
static void opa_scale_anim(void *obj, int32_t v);          // 中心弹窗的动画回调(效果:浮现)
static void popup_task_handler(lv_timer_t *timer);         // 定时器回调
static void update_modal_curtain();                        // 模态幕布更新
static void alert_event_cb(lv_event_t *e);                 // 警告框回调

// 设置菜单主体样式和属性(纯粹背景)
static void obj_style_set(lv_obj_t *obj)
{
    lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN); // 阴影
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN); // 边框
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);      // 不可滚动
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);       // 不可点击，防止遮挡手势
}

/**
 * @brief 下拉菜单子项容器创建(不可见,不可点击,不可滚动)
 * @param Parent 父类
 * @param x       x坐标
 * @param span_x  x跨度
 * @param y       y坐标
 * @param span_y  y跨度
 * @return        返回一个容器快背景
 */
static lv_obj_t *menu_child_container_create(lv_obj_t *Parent, uint8_t x, uint8_t span_x, uint8_t y, uint8_t span_y)
{
    // 快背景(用于对齐布局)
    lv_obj_t *block = lv_obj_create(Parent);
    // lv_obj_set_style_opa(block, 0, LV_PART_MAIN);//不要让整体透明，这会影响子项
    lv_obj_set_style_bg_opa(block, 0, LV_PART_MAIN);
    obj_style_set(block);
    lv_obj_set_grid_cell(block, LV_GRID_ALIGN_STRETCH, x, span_x, LV_GRID_ALIGN_STRETCH, y, span_y);//拉伸对齐
    // 内容背景
    lv_obj_t *block_bg = lv_obj_create(block);
    lv_obj_update_layout(block_bg); // 更新信息
    //边距固定(20 大部件和小部件相同)
    lv_obj_set_size(block_bg, lv_obj_get_width(block) - 20, lv_obj_get_height(block) - 20);

    //设置对齐
    lv_obj_align(block_bg, LV_ALIGN_CENTER, 0, 0); // 子项对齐,中心

    obj_style_set(block_bg);
    lv_obj_set_style_radius(block_bg, 30, LV_PART_MAIN);                       // 圆角
    lv_obj_set_style_bg_color(block_bg, lv_color_hex(0x828282), LV_PART_MAIN); // 块背景
    lv_obj_set_style_bg_opa(block_bg, 200, LV_PART_MAIN);

    return block_bg; // 返回内容背景在外面接收它，它们是同一个对象
}

/**
 * @brief 创建菜单主体和其中内容
 * @param parent 传入G_pd_menu作为父对象
 * @return 返回菜单主体
 */
static lv_obj_t *create_pd_menu_content(lv_obj_t *parent, const pulldown_menu_params_t *params)
{
    // 创建一个内容容器
    lv_obj_t *content_container = lv_obj_create(parent);
    // 初始时让它和父容器一样大，但要把它放在屏幕外
    lv_obj_set_size(content_container, scr_act_width(), scr_act_height());
    lv_obj_set_y(content_container, -scr_act_height()); // 完全隐藏在屏幕上方
    /* 主体样式 */
    lv_obj_set_style_bg_color(content_container, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(content_container, 80, LV_PART_MAIN); // 下拉菜单背景透明度
    obj_style_set(content_container);
    // 设置网格布局
    static lv_coord_t col_dsc[] = { LV_GRID_FR(1), 98, 98, 98, 98, 98, 98, 98,LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1), 98, 98, 98, 98, LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(content_container, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(content_container, 0, 0); // 边距(是整体边距,不是子项间的距离)
    lv_obj_set_style_pad_gap(content_container, 20, 20); // 子项边距
    // 底部拖拽区
    lv_obj_t *obj_line = lv_obj_create(content_container);
    lv_obj_set_style_bg_color(obj_line, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj_line, 40, LV_PART_MAIN); // 注意子项和父项重叠区opa会叠加,实际上是在设置子比父深多少
    obj_style_set(obj_line);
    lv_obj_set_grid_cell(obj_line, LV_GRID_ALIGN_STRETCH, 0, 9, LV_GRID_ALIGN_STRETCH, 5, 1);
    static lv_point_t line_points[] = {{0, 0}, {60, 0}};
    lv_obj_t *line = lv_line_create(obj_line);
    lv_line_set_points(line, line_points, 2);                                // 设置线条坐标点,创建线条
    lv_obj_align(line, LV_ALIGN_CENTER, 0, 0);                               // 设置位置
    lv_obj_set_style_line_width(line, 5, LV_PART_MAIN);                      // 设线的宽度
    lv_obj_set_style_line_color(line, lv_color_hex(0x353535), LV_PART_MAIN); // 线的颜色
    lv_obj_set_style_line_rounded(line, true, LV_PART_MAIN);                 // 设置线条圆角
    lv_obj_set_grid_cell(line, LV_GRID_ALIGN_STRETCH, 5, 1, LV_GRID_ALIGN_STRETCH, 5, 1);

    /* 这里暂时写死，后续考虑扩展性 */
#if 1
    /* 播放块 */
    lv_obj_t *app_bg = menu_child_container_create(content_container, 1, 2, 1, 2);

    /* 亮度块 */
    lv_obj_t *bright_bg = menu_child_container_create(content_container, 3, 1, 1, 2);
    lv_obj_t *bright = lv_slider_create(bright_bg);
    lv_obj_update_layout(bright_bg);
    lv_obj_set_size(bright, lv_obj_get_width(bright_bg), lv_obj_get_height(bright_bg));
    lv_obj_center(bright);
    lv_obj_remove_style(bright, NULL, LV_PART_KNOB);                              // 移除手柄
    lv_obj_set_style_radius(bright, 30, LV_PART_MAIN);                            // 主体圆角,这会限制指示器
    lv_obj_set_style_bg_opa(bright, 0, LV_PART_MAIN);                             // 设置主体不可见
    lv_obj_set_style_radius(bright, 30, LV_PART_INDICATOR);                       // 指示器圆角
    lv_obj_set_style_bg_color(bright, lv_color_hex(0xF0F0F0), LV_PART_INDICATOR); // 指示器颜色
    lv_slider_set_range(bright, 0, 100);                                          // 设置范围(最终映射到遮罩opa值255~0)
#ifndef NDEBUG
    printf("menu_content:slider_get_value=%d\r\n", params->initial_brightness);
#endif
    lv_slider_set_value(bright, params->initial_brightness, LV_ANIM_OFF); // 设置初始值
    // 回调
    lv_obj_add_event_cb(bright, bright_event_cb, LV_EVENT_VALUE_CHANGED, NULL); // 回调(值改变)
    lv_obj_add_event_cb(bright, bright_event_cb, LV_EVENT_PRESSED, NULL);       // 回调(按下)
    lv_obj_add_event_cb(bright, bright_event_cb, LV_EVENT_RELEASED, NULL);      // 回调(松开)
    // 图标
    lv_obj_t *bright_img = lv_img_create(bright);
    lv_img_set_src(bright_img, &icon_bright); // 传入图片源
    lv_obj_align(bright_img, LV_ALIGN_CENTER, 0, 47);
    // 图标滤镜
    lv_opa_t bright_img_opa = lv_map(params->initial_brightness, 0, 100, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_obj_set_style_img_recolor(bright_img, lv_color_hex(0x505050), LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(bright_img, bright_img_opa, LV_PART_MAIN); // 滤镜强度

    /* 音量块 */
    lv_obj_t *volume_bg = menu_child_container_create(content_container, 4, 1, 1, 2);
    lv_obj_t *volume = lv_slider_create(volume_bg);
    lv_obj_update_layout(volume_bg);
    lv_obj_set_size(volume, lv_obj_get_width(volume_bg), lv_obj_get_height(volume_bg));
    lv_obj_center(volume);
    lv_obj_remove_style(volume, NULL, LV_PART_KNOB);                              // 移除手柄
    lv_obj_set_style_radius(volume, 30, LV_PART_MAIN);                            // 主体圆角,这会限制指示器
    lv_obj_set_style_bg_opa(volume, 0, LV_PART_MAIN);                             // 设置主体不可见
    lv_obj_set_style_radius(volume, 30, LV_PART_INDICATOR);                       // 指示器圆角
    lv_obj_set_style_bg_color(volume, lv_color_hex(0xF0F0F0), LV_PART_INDICATOR); // 指示器颜色
    lv_slider_set_range(volume, 0, 100);                                          // 设置范围
    lv_slider_set_value(volume, params->initial_volume, LV_ANIM_OFF);             // 设置初始值
    // 回调
    lv_obj_add_event_cb(volume, volume_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    // 图标
    lv_obj_t *volume_label = lv_label_create(volume);
    lv_label_set_text(volume_label, LV_SYMBOL_MUTE); // 音量,小
    lv_obj_set_style_text_font(volume_label, &lv_font_montserrat_30, LV_STATE_DEFAULT);
    lv_obj_align(volume_label, LV_ALIGN_CENTER, 0, 50);
    // 根据获取值设置样式
    if (params->initial_volume < 40)
    {
        lv_label_set_text(volume_label, LV_SYMBOL_MUTE);
    }
    else if (params->initial_volume < 70)
    {
        lv_label_set_text(volume_label, LV_SYMBOL_VOLUME_MID);
    }
    else
    {
        lv_label_set_text(volume_label, LV_SYMBOL_VOLUME_MAX);
    }

    /* WiFi */
    lv_obj_t *WiFi_bg = menu_child_container_create(content_container, 5, 1, 1, 1);

    lv_obj_t *WiFi = lv_btn_create(WiFi_bg); // 创建按钮
    lv_obj_update_layout(WiFi_bg);
    lv_obj_set_size(WiFi, lv_obj_get_width(WiFi_bg), lv_obj_get_height(WiFi_bg));
    lv_obj_align(WiFi, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(WiFi, 30, LV_PART_MAIN);                           // 指示器圆角
    lv_obj_add_flag(WiFi, LV_OBJ_FLAG_CHECKABLE);                              // 设置按钮为切换
    lv_obj_set_style_bg_color(WiFi, lv_color_hex(0x828282), LV_STATE_DEFAULT); // 默认 "释放" 时背景颜色为灰色
    lv_obj_set_style_bg_color(WiFi, lv_color_hex(0x1E90FF), LV_STATE_CHECKED); // 设置按钮 "选中" 时背景颜色为蓝色
    lv_obj_add_event_cb(WiFi, WiFi_event_cb, LV_EVENT_VALUE_CHANGED, NULL);    // 回调(值改变)

    lv_obj_t *WiFi_icon = lv_img_create(WiFi); // 创建按钮
    lv_img_set_src(WiFi_icon, &icon_wifi);     // 传入图片源
    lv_obj_center(WiFi_icon);
    lv_img_set_zoom(WiFi_icon, 300);
    lv_obj_set_style_img_recolor(WiFi_icon, lv_color_hex(0x333333), LV_PART_MAIN); // 图标颜色
    lv_obj_set_style_img_recolor_opa(WiFi_icon, LV_OPA_COVER, LV_PART_MAIN);       // 图标颜色透明度

    lv_obj_set_user_data(WiFi, WiFi_icon); // 存储图标对象,方便回调获取

    /* WiFi2 */
    lv_obj_t* WiFi_bg2 = menu_child_container_create(content_container, 5, 1, 2, 1);

    lv_obj_t* WiFi2 = lv_btn_create(WiFi_bg2); // 创建按钮
    lv_obj_update_layout(WiFi_bg2);
    lv_obj_set_size(WiFi2, lv_obj_get_width(WiFi_bg2), lv_obj_get_height(WiFi_bg2));
    lv_obj_align(WiFi2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(WiFi2, 30, LV_PART_MAIN);                           // 指示器圆角
    lv_obj_add_flag(WiFi2, LV_OBJ_FLAG_CHECKABLE);                              // 设置按钮为切换
    lv_obj_set_style_bg_color(WiFi2, lv_color_hex(0x828282), LV_STATE_DEFAULT); // 默认 "释放" 时背景颜色为灰色
    lv_obj_set_style_bg_color(WiFi2, lv_color_hex(0x1E90FF), LV_STATE_CHECKED); // 设置按钮 "选中" 时背景颜色为蓝色

#endif

#if 0
    test_layout_grid(content_container,6,9); // 布局测试
#endif
    return content_container;
}

/**
 * @brief 获取对象的opa值
 * @param obj 对象
 * @param part 部分
 * @return opa(0~255)
 */
static inline lv_opa_t obj_get_style_opa(const struct _lv_obj_t *obj, uint32_t part)
{
    lv_style_value_t v = lv_obj_get_style_prop(obj, part, LV_STYLE_OPA);
    return (lv_opa_t)v.num;
}

/**
 * @brief 聚焦模式,隐藏指定级父项下的所有对象,排除聚焦子项和顶级项下它的所有父项( 适用于不知道父项地址 )
 *
 * @param focused_bg 聚焦对象
 * @param Top_parent 父项级
 * @param enable     此函数操作可还原
 */
static void pd_menu_set_focus_mode(lv_obj_t *focused_bg, uint8_t Top_parent_level, bool enable)
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
        lv_obj_set_style_bg_opa(G_pd_menu, 0, LV_PART_MAIN);
    }
    else
    {
        lv_obj_set_style_bg_opa(Top_parent, 80, LV_PART_MAIN); // 这里获取到的是菜单背景
        lv_obj_set_style_bg_opa(G_pd_menu, 150, LV_PART_MAIN); // 调节亮度时菜单一定是完全展开态,值是确定的
    }

    // 遍历 Top_parent 下的所有子对象
    uint32_t child_count = lv_obj_get_child_cnt(Top_parent); // 子项数量
#ifdef NDEBUG
    printf("focus_mode:child_count=%d\r\n", child_count);
#endif
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

/**
 * @brief 隐藏一个对象的所有兄弟节点( 适用于知道父项地址 )
 *
 * @param obj     需要保留的对象
 * @param enable  true: 隐藏兄弟, false: 显示兄弟
 */
//static void hide_siblings(lv_obj_t *obj, bool enable)
//{
//    if (obj == NULL)
//        return;
//    lv_obj_t *parent = lv_obj_get_parent(obj);
//    if (parent == NULL)
//        return;
//
//    uint32_t child_cnt = lv_obj_get_child_cnt(parent);
//    for (uint32_t i = 0; i < child_cnt; i++)
//    {
//        lv_obj_t *child = lv_obj_get_child(parent, i);
//        if (child != obj)
//        { // 如果不是自己
//            if (enable)
//            {
//                lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
//            }
//            else
//            {
//                lv_obj_clear_flag(child, LV_OBJ_FLAG_HIDDEN);
//            }
//        }
//    }
//}

/* ============================================= 功能函数(外部api) ============================================= */

/**
 * @brief 弹窗功能初始化
 */
void popup_manager_init(void)
{
    // 初始化所有记录
    memset(G_active_popups, 0, sizeof(G_active_popups));

    // 创建全局遮罩，初始隐藏
    G_modal_curtain = lv_obj_create(lv_layer_top());
    lv_obj_set_size(G_modal_curtain, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(G_modal_curtain, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(G_modal_curtain, LV_OPA_20, 0); // 半透明
    lv_obj_add_flag(G_modal_curtain, LV_OBJ_FLAG_HIDDEN);

    // 创建窗口后台任务定时器
    G_task_timer = lv_timer_create(popup_task_handler, 100, NULL);
}

/* ------------------------------------------- 下拉菜单相关 --------------------------------------------- */
/**
 * @brief 显示下拉菜单
 * @param params 传递数据
 */
void popup_show_pulldown_menu(const popup_param_base_t *params)
{
    // 如果菜单已存在，则不执行任何操作
    if (G_pd_menu != NULL)
    {
        return;
    }

    // 【新增】在函数开头，解析传入的参数
    int32_t initial_brightness = 0; // 默认值
    int32_t initial_volume = 0;     // 默认值

    pulldown_menu_params_t content_params; // 继续传给内容创建函数

    // 安全地检查和复制参数
    if (params != NULL && params->type == POPUP_PARAM_TYPE_PULLDOWN_MENU)
    {
        content_params = *(const pulldown_menu_params_t *)params;
    }
    else
    {
        // 如果没有传入参数，则使用默认值初始化本地副本
        content_params.base.type = POPUP_PARAM_TYPE_PULLDOWN_MENU;
        content_params.initial_brightness = 0;
        content_params.initial_volume = 0;
    }

    // 标记菜单存在
    G_is_pd_menu_active = true;
    // 创建一个全屏的、半透明的背景对象
    // 这个背景对象将负责捕获所有的拖动和释放事件
    G_pd_menu = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(G_pd_menu); // 移除所有默认样式
    lv_obj_set_size(G_pd_menu, scr_act_width(), scr_act_height());
    lv_obj_set_style_bg_color(G_pd_menu, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(G_pd_menu, LV_OPA_0, 0);      // 刚开始半透明
    lv_obj_clear_flag(G_pd_menu, LV_OBJ_FLAG_SCROLLABLE); // 不可滚动

    // 加载内容
    lv_obj_t *content_container = create_pd_menu_content(G_pd_menu, &content_params);

    // 将内容容器的指针存入背景对象的 user_data，方便在事件回调中获取
    G_pd_menu->user_data = content_container;

    // 为背景绑定事件处理器
    lv_obj_add_event_cb(G_pd_menu, pd_menu_event_cb, LV_EVENT_ALL, NULL);
}

/**
 * @brief 下拉菜单关闭事件(外部调用,进入关闭判断)
 */
void popup_pulldown_menu_begin_close(void)
{
    if (G_pd_menu == NULL)
    {
        return;
    }
#ifdef NDEBUG
    printf("pulldown_menu_begin_close\r\n");
#endif
    // 重新为菜单背景绑定事件处理器，让它能再次响应拖动
    lv_obj_add_event_cb(G_pd_menu, pd_menu_event_cb, LV_EVENT_ALL, NULL);

    // "吸附"到手指：让菜单的下边沿立刻跟随手指
    lv_obj_t *content_container = G_pd_menu->user_data;
    lv_coord_t content_h = lv_obj_get_height(content_container);

    lv_point_t p;
    lv_indev_get_point(lv_indev_get_act(), &p);
    lv_obj_set_y(content_container, p.y - content_h); // 下边沿吸附
}

/* ------------------------------------------- toast相关 --------------------------------------------- */
/**
 * @brief 在底部显示一个标签
 * @param text 内容
 * @param duration_ms 持续时间
 */
void popup_show_toast(const char *text, uint32_t duration_ms)
{
    // 创建一个简单的标签作为Toast
    lv_obj_t *toast_label = lv_label_create(lv_layer_top());
    lv_label_set_text(toast_label, text);
    lv_obj_set_style_bg_color(toast_label, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(toast_label, LV_OPA_0, 0);
    lv_obj_set_style_text_color(toast_label, lv_color_white(), 0);
    lv_obj_set_style_text_opa(toast_label, LV_OPA_0, 0);
    lv_obj_set_style_pad_all(toast_label, 10, 0);              // 边距
    lv_obj_set_style_radius(toast_label, LV_RADIUS_CIRCLE, 0); // 圆角

    popup_show_custom(toast_label, POPUP_REGION_BOTTOM_MID, false, duration_ms, NULL, NULL);
}

/* ------------------------------------------- 警告框相关 --------------------------------------------- */
/**
 * @brief 显示一个警告框(未完成)
 * @param title 标题
 * @param text 内容
 * @param buttons 按钮
 * @param callback 回调函数
 * @param user_data 传递书记处
 */
void popup_show_alert(const char *title, const char *text, const char *buttons, popup_close_cb_t callback, void *user_data)
{

    lv_obj_t *alert_box = lv_msgbox_create(NULL, title, text, buttons, true);
    // 获取msgbox的父对象，也就是它自带的那个遮罩
    lv_obj_t *msgbox_background = lv_obj_get_parent(alert_box);
    assert(msgbox_background != NULL);
    // 将其自带的遮罩设置为完全透明，使其失效
    // 我们不用隐藏它，因为msgbox的关闭逻辑依赖它，设为透明最安全
    lv_obj_set_style_bg_opa(msgbox_background, LV_OPA_TRANSP, 0);
    lv_obj_add_event_cb(alert_box, alert_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    // 添加到显示区域
    popup_show_custom(alert_box, POPUP_REGION_CENTER, true, 0, callback, user_data);
}

/* ------------------------------------------- 核心函数 --------------------------------------------- */
/**
 * @brief 在指定区域显示一个弹窗(通用)
 * @param popup     应用创建的 lv_obj_t* 容器
 * @param region    希望弹窗显示的区域
 * @param is_modal  是否阻断背景操作
 * @param duration_ms 自动关闭的时长 (0 表示不自动关闭)
 * @param callback  弹窗关闭时的回调
 * @param user_data 自定义数据
 */
void popup_show_custom(lv_obj_t *popup, popup_region_t region, bool is_modal, uint32_t duration_ms, popup_close_cb_t callback, void *user_data)
{
    // 如果下拉菜单激活，则阻止侧边弹窗
    if (G_is_pd_menu_active && (region == POPUP_REGION_SIDE_LEFT || region == POPUP_REGION_SIDE_RIGHT))
    {
#ifndef NDEBUG
        printf("Pull-down menu is exist!!\r\n");
#endif
        lv_obj_del(popup); // 必须删除传入的对象，防止内存泄漏
        return;
    }

    if (region >= POPUP_REGION_COUNT)
        return;
    // 如果该区域已有弹窗，先关闭它
    if (G_active_popups[region].obj != NULL)
    {
        popup_close_by_region(region);
    }

    // 记录新的弹窗信息(设置结构体数组元素)
    G_active_popups[region] = (popup_instance_t){
        .obj = popup,                                                             // 内容容器
        .is_modal = is_modal,                                                     // 遮罩
        .dismiss_at_tick = (duration_ms > 0) ? (lv_tick_get() + duration_ms) : 0, // 关闭时间
        .on_close_cb = callback,                                                  // 关闭回调
        .user_data = user_data                                                    // 自定义数据
    };

    // 设置父对象并对齐,大小需要传入前计算好
    lv_obj_set_parent(popup, lv_layer_top());
    lv_obj_update_layout(popup);
    // 获取对象和屏幕的尺寸
    lv_coord_t obj_width = lv_obj_get_width(popup);          // 窗口宽度
    lv_coord_t obj_height = lv_obj_get_height(popup);        // 窗口高度
    lv_coord_t scr_width = lv_obj_get_width(lv_scr_act());   // 屏幕宽度
    lv_coord_t scr_height = lv_obj_get_height(lv_scr_act()); // 屏幕高度

    switch (region)
    {
    /* ==================== 顶部中间 ==================== */
    case POPUP_REGION_TOP_MID:
    {
        // --- 动画准备阶段 ---
        // 计算最终的X, Y坐标
        lv_coord_t final_x = (scr_width - obj_width) / 2; // 水平居中
        lv_coord_t final_y = obj_height;
        // 计算起始的Y坐标 (屏幕正上方，完全隐藏)
        lv_coord_t start_y = -(obj_height);

        // 先使用 lv_obj_set_pos 设置对象的初始位置,放置在外面,让它移动进来
        lv_obj_set_pos(popup, final_x, start_y);

        // --- 动画执行阶段 ---
        lv_anim_t a;
        lv_anim_init(&a);           // 初始化
        lv_anim_set_var(&a, popup); // 应用到对象
        // 动画只改变Y坐标
        lv_anim_set_values(&a, start_y, final_y);                  // 设置关键帧的值
        lv_anim_set_time(&a, 300);                                 // 动画时长
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y); // 动画执行函数：使用Y坐标设置函数
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);            // 动画路径：平滑的动画曲线
        lv_anim_start(&a);
        break;
    }
    /* ==================== 屏幕正中 ==================== */
    case POPUP_REGION_CENTER:
    {
        // 设置位置(不能使用对齐样式)
        lv_obj_set_pos(popup, (scr_width - obj_width) / 2, (scr_height - obj_height) / 2);

        //// 可以为中间弹窗添加淡入动画

        lv_anim_t a_fade;
        lv_anim_init(&a_fade);
        lv_anim_set_var(&a_fade, popup);
        lv_anim_set_values(&a_fade, LV_OPA_TRANSP, LV_OPA_COVER);
        lv_anim_set_time(&a_fade, 150);
        // 自定义回调(同时改变背景和文本颜色)
        lv_anim_set_exec_cb(&a_fade, opa_scale_anim);

        lv_anim_start(&a_fade);
        break;
    }
    /* ==================== 屏幕左侧 ==================== */
    case POPUP_REGION_SIDE_LEFT:
    {
        // 计算最终的X, Y坐标
        lv_coord_t final_y = (scr_height - obj_height) / 2; // 垂直居中
        lv_coord_t final_x = obj_width;
        // 计算起始的x坐标 (屏幕左侧，完全隐藏)
        lv_coord_t start_x = -(obj_width);

        // 先使用 lv_obj_set_pos 设置对象的初始位置,放置在外面,让它移动进来
        lv_obj_set_pos(popup, start_x, final_y);

        // --- 动画执行阶段 ---
        lv_anim_t a;
        lv_anim_init(&a);           // 初始化
        lv_anim_set_var(&a, popup); // 应用到对象
        // 动画只改变Y坐标
        lv_anim_set_values(&a, start_x, final_x);                  // 设置关键帧的值
        lv_anim_set_time(&a, 300);                                 // 动画时长
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x); // 动画执行函数：使用Y坐标设置函数
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);            // 动画路径：平滑的动画曲线
        lv_anim_start(&a);
        break;
    }
    /* ==================== 屏幕右侧 ==================== */
    case POPUP_REGION_SIDE_RIGHT:
    {
        // 计算最终的X, Y坐标
        lv_coord_t final_y = (scr_height - obj_height) / 2; // 垂直居中
        lv_coord_t final_x = scr_width - 2 * obj_width;
        // 计算起始的x坐标 (屏幕右侧，完全隐藏)
        lv_coord_t start_x = scr_width + obj_width;

        // 先使用 lv_obj_set_pos 设置对象的初始位置,放置在外面,让它移动进来
        lv_obj_set_pos(popup, start_x, final_y);

        // --- 动画执行阶段 ---
        lv_anim_t a;
        lv_anim_init(&a);           // 初始化
        lv_anim_set_var(&a, popup); // 应用到对象
        // 动画只改变Y坐标
        lv_anim_set_values(&a, start_x, final_x);                  // 设置关键帧的值
        lv_anim_set_time(&a, 300);                                 // 动画时长
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x); // 动画执行函数：使用Y坐标设置函数
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);            // 动画路径：平滑的动画曲线
        lv_anim_start(&a);
        break;
    }
    /* ==================== 底部中间 ==================== */
    case POPUP_REGION_BOTTOM_MID:
    {
        // --- 动画准备阶段 ---
        // 计算最终的X, Y坐标
        lv_coord_t final_x = (scr_width - obj_width) / 2; // 水平居中
        lv_coord_t final_y = scr_height - 2 * obj_height;
        // 计算起始的Y坐标 (屏幕正下方，完全隐藏)
        lv_coord_t start_y = scr_height + obj_height;

        lv_obj_set_pos(popup, final_x, start_y);

        // --- 动画执行阶段 ---
        lv_anim_t a;
        lv_anim_init(&a);           // 初始化
        lv_anim_set_var(&a, popup); // 应用到对象
        // 动画只改变Y坐标
        lv_anim_set_values(&a, start_y, final_y);                  // 设置关键帧的值
        lv_anim_set_time(&a, 300);                                 // 动画时长
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y); // 动画执行函数：使用Y坐标设置函数
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);            // 动画路径：平滑的动画曲线
        lv_anim_start(&a);
        break;
    }
    /* ==================== 全屏(下拉菜单) ==================== */
    case POPUP_REGION_FULLSCREEN:
    {

        lv_obj_align(popup, LV_ALIGN_CENTER, 0, 0);
        break;
    }
    default:
        lv_obj_align(popup, LV_ALIGN_CENTER, 0, 0); // 默认对齐
        break;
    }

    if (G_is_pd_menu_active)
    {
        // 如果新弹窗不是底部弹窗，就必须被下拉菜单覆盖
        if (region != POPUP_REGION_BOTTOM_MID)
        {
            // 将下拉菜单的背景移动到最前景，从而盖住刚创建的弹窗
            lv_obj_move_foreground(G_pd_menu);
        }
    }

    // 处理模态
    update_modal_curtain();
    if (is_modal)
    {
        lv_obj_move_foreground(popup); // 确保弹窗在幕布之上
    }
}

/**
 * @brief 关闭指定区域当前的弹窗
 * @param region 区域
 */
void popup_close_by_region(popup_region_t region)
{
    if (region >= POPUP_REGION_COUNT || G_active_popups[region].obj == NULL)
    {
        return;
    }

    popup_instance_t *inst = &G_active_popups[region];
    lv_obj_t *obj_to_close = inst->obj; // 先把要关闭的对象指针存起来
    // 如果有关闭回调，先调用它
    if (inst->on_close_cb)
    {
        inst->on_close_cb(obj_to_close, -1, inst->user_data);
    }

    inst->obj = NULL; // 立即清理记录，防止对同一个区域重复调用关闭

    lv_obj_update_layout(obj_to_close); // 确保能获取到正确的尺寸
    switch (region)
    {
    /* ==================== 顶部中间 ==================== */
    case POPUP_REGION_TOP_MID:
    {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, obj_to_close);
        lv_anim_set_values(&a, lv_obj_get_y(obj_to_close), -lv_obj_get_height(obj_to_close));
        lv_anim_set_time(&a, 300);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
        // 动画结束后，调用删除函数
        lv_anim_set_ready_cb(&a, delete_obj_anim_ready_cb);
        lv_anim_start(&a);
        break;
    }
    /* ==================== 屏幕正中 ==================== */
    case POPUP_REGION_CENTER:
    {

        lv_anim_t a_fade;
        lv_anim_init(&a_fade);
        lv_anim_set_var(&a_fade, obj_to_close);
        lv_anim_set_values(&a_fade, LV_OPA_COVER, LV_OPA_TRANSP);
        lv_anim_set_time(&a_fade, 150);
        // 自定义回调(同时改变背景和文本颜色)
        lv_anim_set_exec_cb(&a_fade, opa_scale_anim);
        lv_anim_set_ready_cb(&a_fade, delete_obj_anim_ready_cb);
        lv_anim_start(&a_fade);
        break;
    }
    /* ==================== 屏幕左侧 ==================== */
    case POPUP_REGION_SIDE_LEFT:
    {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, obj_to_close);
        // 计算并设置起始和终止x坐标
        lv_anim_set_values(&a, lv_obj_get_x(obj_to_close), -lv_obj_get_width(obj_to_close));
        lv_anim_set_time(&a, 300);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
        // 动画结束后，调用删除函数
        lv_anim_set_ready_cb(&a, delete_obj_anim_ready_cb);
        lv_anim_start(&a);
        break;
    }
    /* ==================== 屏幕右侧 ==================== */
    case POPUP_REGION_SIDE_RIGHT:
    {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, obj_to_close);
        // 计算并设置起始和终止x坐标
        lv_anim_set_values(&a, lv_obj_get_x(obj_to_close), lv_obj_get_width(lv_scr_act()) + lv_obj_get_width(obj_to_close));
        lv_anim_set_time(&a, 300);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
        // 动画结束后，调用删除函数
        lv_anim_set_ready_cb(&a, delete_obj_anim_ready_cb);
        lv_anim_start(&a);
        break;
    }
    /* ==================== 底部中间 ==================== */
    case POPUP_REGION_BOTTOM_MID:
    {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, obj_to_close);
        lv_anim_set_values(&a, lv_obj_get_y(obj_to_close), lv_obj_get_height(lv_scr_act()) + lv_obj_get_height(obj_to_close));
        lv_anim_set_time(&a, 300);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
        // 动画结束后，调用删除函数
        lv_anim_set_ready_cb(&a, delete_obj_anim_ready_cb);
        lv_anim_start(&a);
        break;
    }

    default:
        // 对于没有定义退出动画的区域，直接删除
        lv_obj_del(obj_to_close);
        break;
    }
    // 检查是否还需要幕布
    update_modal_curtain();
}

/* ================================================= 回调函数 ================================================= */

/* --------------------------------- 下拉菜单相关 ------------------------------------ */
/**
 * @brief 音量滑块回调
 * @param e
 */
static void volume_event_cb(lv_event_t *e)
{
    lv_obj_t *volume = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(volume);

    // 更新音量值
    volume_set_value(value);

    lv_obj_t *icon_label = lv_obj_get_child(volume, 0);
    if (icon_label == NULL)
    {
        return;
    }
    // 根据滑块的值，来更新音量图标
    if (value < 40)
    {
        lv_label_set_text(icon_label, LV_SYMBOL_MUTE);
    }
    else if (value < 70)
    {
        lv_label_set_text(icon_label, LV_SYMBOL_VOLUME_MID);
    }
    else
    {
        lv_label_set_text(icon_label, LV_SYMBOL_VOLUME_MAX);
    }
}

/**
 * @brief 亮度滑块回调
 * @param e
 */
static void bright_event_cb(lv_event_t *e)
{
    lv_obj_t *bright = lv_event_get_target(e);         // 触发对象(滑块)
    lv_obj_t *bright_bg = lv_obj_get_parent(bright);   // 父容器
    lv_event_code_t event_code = lv_event_get_code(e); // 事件代码

    // 当手指按下时，开启聚焦模式
    if (event_code == LV_EVENT_PRESSED)
    {
#ifndef NDEBUG
        printf("bright: Focus Mode\r\n");
#endif
        pd_menu_set_focus_mode(bright, 3, true); // 聚焦
    }

    // 事件2：当手指松开或移出时，关闭聚焦模式
    else if (event_code == LV_EVENT_RELEASED || event_code == LV_EVENT_PRESS_LOST)
    {
#ifndef NDEBUG
        printf("bright: Exit Focus Mode\r\n");
#endif
        pd_menu_set_focus_mode(bright, 3, false); // 恢复
    }

    // 事件3：当滑块值改变时，更新图标的染色效果
    else if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        int32_t value = lv_slider_get_value(bright);        // 滑块值
        lv_obj_t *bright_img = lv_obj_get_child(bright, 0); // 图标
#ifdef NDEBUG
        printf("bright_slider_value:%d\r\n", value);
#endif
        if (bright_img == NULL)
        {
            return;
        }
        // 设置亮度遮罩（测试）
        activity_bright_set(value);

        // 根据滑块的值，来更新图标的滤镜强度
        lv_opa_t mapped_opa = lv_map(value, 0, 100, LV_OPA_COVER, LV_OPA_TRANSP);
        lv_obj_set_style_img_recolor_opa(bright_img, mapped_opa, LV_PART_MAIN);
    }
}

/**
 * @brief WiFi按钮回调
 * @param e
 */
static void WiFi_event_cb(lv_event_t *e)
{
    // 获取事件码和WiFi按钮
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *WiFi_btn = lv_event_get_target(e);
    lv_obj_t *WiFi_icon = lv_obj_get_user_data(WiFi_btn); // 获取图标标签

    if (code == LV_EVENT_VALUE_CHANGED)
    {

        // 检查按钮当前是否处于“选中”状态
        if (lv_obj_has_state(WiFi_btn, LV_STATE_CHECKED))
        { // 如果是，说明状态刚刚从“未选中”变为“选中”
            // 重着色,白
            lv_obj_set_style_img_recolor(WiFi_icon, lv_color_hex(0xEEEEEE), LV_PART_MAIN); // 图标颜色
#ifndef NDEBUG
            printf("WiFi button turned ON (Checked)\r\n");
#endif
            // 在这里执行开启WiFi的逻辑
            //extern void wificonnect(void);
            //wificonnect();
        }
        else
        { // 如果不是，说明状态刚刚从“选中”变为“未选中”
            // 重着色,灰
            lv_obj_set_style_img_recolor(WiFi_icon, lv_color_hex(0x333333), LV_PART_MAIN); // 图标颜色
#ifndef NDEBUG
            printf("WiFi button turned OFF (Default)\r\n");
#endif
            // 在这里执行关闭WiFi的逻辑
            //extern void wifidisconnect(void);
            //wifidisconnect();
        }
    }
}

/**
 * @brief 负责跟踪拖拽手势并设置下拉菜单运动动画
 * @param e 即G_pd_menu负责接收手势并触发回调
 */
static void pd_menu_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);       // 事件代码
    lv_obj_t *bg = lv_event_get_target(e);             // 事件目标是背景
    lv_obj_t *content = bg->user_data;                 // 从user_data获取内容容器(content_container)
    lv_coord_t content_h = lv_obj_get_height(content); // 菜单高度
    lv_indev_t *indev = lv_indev_get_act();            // 输入设备
    if (indev == NULL)
        return;

    lv_point_t p;
    lv_indev_get_point(indev, &p); // 获取当前点

    // 根据Y坐标，动态计算并设置背景透明度
    lv_coord_t current_y = lv_obj_get_y(content);
    lv_coord_t threshold_y = -content_h / 2; // 拉到一半的位置
    uint8_t bg_opa;
    if (current_y >= threshold_y)
    {
        // 如果菜单已经拉下来超过一半，则背景完全不透明(在我们的设定中是150)
        bg_opa = 150;
    }
    else
    {
        // 如果菜单还在从完全隐藏到一半的过程中，则进行线性映射
        // 将 current_y 从 [-content_h, threshold_y] 的范围
        // 映射到 opa 从 [0, 150] 的范围
        bg_opa = lv_map(current_y, -content_h, threshold_y, 0, 150);
    }
    // 应用计算出的透明度
    lv_obj_set_style_bg_opa(bg, bg_opa, 0);

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
        // lv_coord_t final_y = p.y - ;             //计算菜单y坐标(控件的坐标在左上角)
        //  滑入还是滑出
        if (p.y > scr_act_height() / 2)
        { // 松手点在屏幕下部1/3
            // --- 自动滑入 ---
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, content);
            lv_anim_set_values(&a, p.y - content_h, 0); // 注意此时左上角在屏幕外
            lv_anim_set_time(&a, 400);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            // 动画结束后，解除移动事件回调
            lv_obj_remove_event_cb(bg, pd_menu_event_cb);
            lv_anim_start(&a);
        }
        else
        {
            // --- 自动滑出并销毁 ---
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, content);
            lv_anim_set_values(&a, p.y - content_h, -(content_h + 10)); // 移动到屏幕外
            lv_anim_set_time(&a, 400);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
            // 动画结束后，调用删除整个菜单的函数
            lv_anim_set_ready_cb(&a, pd_menu_delete_anim_ready_cb);
            lv_anim_start(&a);
        }
    }
}

/**
 * @brief 下拉菜单关闭回调
 * @param anim
 */
static void pd_menu_delete_anim_ready_cb(lv_anim_t *anim)
{
    lv_obj_del(G_pd_menu);
    G_pd_menu = NULL;
    G_is_pd_menu_active = false;
}

/* --------------------------------- 通用 ------------------------------------ */

/**
 * @brief 定时器回调,关闭定义了超时时间的弹窗
 * @param timer
 */
static void popup_task_handler(lv_timer_t *timer)
{
    for (int i = 0; i < POPUP_REGION_COUNT; i++)
    {
        if (G_active_popups[i].obj != NULL && G_active_popups[i].dismiss_at_tick != 0)
        {
            if (lv_tick_get() >= G_active_popups[i].dismiss_at_tick)
            {
                // 时间到了，关闭这个弹窗
                popup_close_by_region((popup_region_t)i);
            }
        }
    }
}

// 检查是否还需要显示模态幕布
static void update_modal_curtain()
{
    bool any_modal_active = false;
    for (int i = 0; i < POPUP_REGION_COUNT; i++)
    {
        if (G_active_popups[i].obj != NULL && G_active_popups[i].is_modal)
        {
            any_modal_active = true;
            break;
        }
    }
    if (any_modal_active)
    {
        lv_obj_clear_flag(G_modal_curtain, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(G_modal_curtain); // 确保幕布在最前面
        // (在show的时候会把真正的弹窗再移到幕布前)
    }
    else
    {
        lv_obj_add_flag(G_modal_curtain, LV_OBJ_FLAG_HIDDEN); // 隐藏遮罩
    }
}

/**
 * @brief 动画结束回调，用于在动画结束后删除对象
 * @param anim 对象绑定的动画
 */
static void delete_obj_anim_ready_cb(lv_anim_t *anim)
{
    lv_obj_del(anim->var); // 当初设置的目标对象
}

/**
 * @brief 中心弹窗的动画回调(效果:浮现)
 * @param obj 对象
 * @param v 值
 */
static void opa_scale_anim(void *obj, int32_t v)
{
    lv_obj_set_style_bg_opa(obj, v, 0);
    lv_obj_set_style_text_opa(obj, v, 0);
}

// 警告框的内部事件回调，用于链接到回调系统
static void alert_event_cb(lv_event_t *e)
{
    lv_obj_t *alert_box = lv_event_get_current_target(e);
    uint32_t button_id = lv_msgbox_get_active_btn(alert_box);

    // 从警告框找到它属于哪个区域实例
    for (int i = 0; i < POPUP_REGION_COUNT; i++)
    {
        if (G_active_popups[i].obj == alert_box)
        {
            // 在关闭之前，先调用回调
            if (G_active_popups[i].on_close_cb)
            {
                G_active_popups[i].on_close_cb(alert_box, button_id, G_active_popups[i].user_data);
            }
            // lv_msgbox_close 会自动删除对象，只清理自己的记录
            G_active_popups[i].obj = NULL;
            update_modal_curtain();
            break;
        }
    }
}
