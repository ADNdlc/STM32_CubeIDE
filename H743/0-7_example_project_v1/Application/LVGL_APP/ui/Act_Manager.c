#include "Act_Manager.h"
#include "Home.h"
#include "Popup.h"
#include "util/util.h"

/* 外部引用(获取接口来初始化对应应用) */
#include "APP/Settings/Settings.h"
#include "APP/test1/test1.h"
#include "APP/File/File.h"
#include "APP/colorwheel/colorwheel.h"
#include "APP/Contol/Contol.h"

// #define opa_test

/* =================================================== 活动定义 ============================================= */
// 活动实例结构体 (链表节点,代表一个运行中的活动实例)
typedef struct activity_t
{
    app_def_t *App;          // 指向app定义
    lv_obj_t *screen;        // 指向此app的屏幕
    uint32_t last_used_time; // 最后一次被使用的时间戳 (用于LRU淘汰)
    struct activity_t *prev; // 指向栈中的前一个活动 (链表)
} activity_t;

/* =================================================== 私有变量 ============================================= */
/* ----------------------- 活动管理 ----------------------- */
// 活动栈的栈顶 (链表头),表示当前活动的app
static activity_t *G_activity_stack = NULL;

#define MAX_REGISTERED_ACTIVITIES 16                                  // 最大app数量
static const app_def_t *G_registered_defs[MAX_REGISTERED_ACTIVITIES]; // app列表,存储了指向所有可能的app的指针
static uint8_t G_registered_count = 0;                                // app数量(已注册应用)

static const app_def_t *G_home_activity_def = NULL; // 永远存主页面

/* ----------------------- 弹窗相关 ----------------------- */
static lv_obj_t *G_top_hotzone = NULL;    // 顶部热区对象
static lv_obj_t *G_bottom_hotzone = NULL; // 底部热区对象
//...侧边,更多区域

#define EDGE_GESTURE_HEIGHT 30    // 边缘热区的高度 (像素)
#define EDGE_GESTURE_THRESHOLD 30 // 触发手势所需的最小垂直滑动距离

// 手势状态
static struct
{
    lv_point_t start_pos;     // 手势起始坐标
    lv_point_t now_pos;       // 现在的坐标
    bool is_pressing;         // 是否处于按压状态
    lv_obj_t *target_hotzone; // 触发手势的热区
} G_gesture_state;

/* ----------------------- 系统相关标志量(亮度,音量,WiFi状态) ----------------------- */
static lv_obj_t *Bright_mask = NULL; // 亮度遮罩（测试）,值就为0~255(0为全透明,255为不透明)
static uint8_t volume = 0;           // 音量

/* =================================================== 活动管理函数 ============================================= */
static void edge_gesture_event_cb(lv_event_t *e);
static void act_timer_handler(lv_timer_t *t);

/**
 * @brief 整个ui界面入口,初始化并注册所有应用,创建主页面(不用加载,已绑定到默认屏幕),启动定时器
 */
void act_manager_init(void)
{
    /* ----初始化全局变量---- */
    G_activity_stack = NULL; // 活动界面
    G_registered_count = 0;  // 应用数量

    // 亮度遮罩(软件亮度)，测试
    Bright_mask = lv_obj_create(lv_layer_sys());
    lv_obj_set_size(Bright_mask, scr_act_width(), scr_act_height());
    lv_obj_set_style_shadow_width(Bright_mask, 0, LV_PART_MAIN); // 阴影
    lv_obj_set_style_border_width(Bright_mask, 0, LV_PART_MAIN); // 边框
    lv_obj_clear_flag(Bright_mask, LV_OBJ_FLAG_SCROLLABLE);      // 不可滚动
    lv_obj_clear_flag(Bright_mask, LV_OBJ_FLAG_CLICKABLE);       // 不可点击，防止遮挡手势
    lv_obj_set_style_radius(Bright_mask, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(Bright_mask, lv_color_hex(0x000000), LV_PART_MAIN); // 最大亮度
    lv_obj_set_style_bg_opa(Bright_mask, 0, LV_PART_MAIN);                        // 最大亮度

    /* ----设置全局功能---- */
    // 创建顶部热区
    G_top_hotzone = lv_obj_create(lv_layer_sys());
    lv_obj_set_size(G_top_hotzone, scr_act_width(), EDGE_GESTURE_HEIGHT);
    lv_obj_align(G_top_hotzone, LV_ALIGN_TOP_MID, 0, 0);
#ifndef opa_test
    lv_obj_set_style_bg_opa(G_top_hotzone, LV_OPA_TRANSP, 0); // 完全透明
#endif
    lv_obj_set_style_border_width(G_top_hotzone, 0, 0);          // 去除边框
    lv_obj_remove_style(G_top_hotzone, NULL, LV_PART_SCROLLBAR); // 滚动条
    // 监听点击,释放,按压丢失,持续按压
    lv_obj_add_event_cb(G_top_hotzone, edge_gesture_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(G_top_hotzone, edge_gesture_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(G_top_hotzone, edge_gesture_event_cb, LV_EVENT_PRESS_LOST, NULL);
    lv_obj_add_event_cb(G_top_hotzone, edge_gesture_event_cb, LV_EVENT_PRESSING, NULL);

    // 创建底部热区
    G_bottom_hotzone = lv_obj_create(lv_layer_sys());
    lv_obj_set_size(G_bottom_hotzone, scr_act_width(), EDGE_GESTURE_HEIGHT);
    lv_obj_align(G_bottom_hotzone, LV_ALIGN_BOTTOM_MID, 0, 0);
#ifndef opa_test
    lv_obj_set_style_bg_opa(G_bottom_hotzone, LV_OPA_TRANSP, 0); // 完全透明
#endif
    lv_obj_set_style_border_width(G_bottom_hotzone, 0, 0);          // 去除边框
    lv_obj_remove_style(G_bottom_hotzone, NULL, LV_PART_SCROLLBAR); // 滚动条
    // 监听事件
    lv_obj_add_event_cb(G_bottom_hotzone, edge_gesture_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(G_bottom_hotzone, edge_gesture_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(G_bottom_hotzone, edge_gesture_event_cb, LV_EVENT_PRESS_LOST, NULL);

    /* 窗口系统初始化 */
    popup_manager_init();

    /* ----创建定时器添加回调---- */
    lv_timer_t *act_timer = lv_timer_create(act_timer_handler, 500, NULL);

    /* -------------------------活动注册---------------------------- */
    act_manager_register(home_def_get()); // 注册主页面
    // 下面是应用
    act_manager_register(Settings_def_get());
    //act_manager_register(Contol_def_get());
    act_manager_register(test1_def_get());
    act_manager_register(File_def_get());
    //act_manager_register(colorwheel_def_get());

    /* ----初始化主页面活动---- */
    activity_t *home_activity = (activity_t *)lv_mem_alloc(sizeof(activity_t));
    assert(home_activity != NULL);

    home_activity->App = (app_def_t *)G_home_activity_def;
    home_activity->last_used_time = lv_tick_get();
    home_activity->prev = NULL; // 它是栈底，没有前一个活动

    // 2. 调用Home的create回调，创建UI
    home_activity->screen = home_activity->App->create();
    assert(home_activity->screen != NULL);

    // 3. 将Home活动作为第一个元素压入栈中
    G_activity_stack = home_activity;

    // 4. 加载Home屏幕
    //    注意：由于这是第一次加载，LVGL的默认屏幕和home_activity->screen可能是同一个。
    //    但这样写逻辑更统一。
    // lv_scr_load(home_activity->screen);

    assert(G_activity_stack != NULL && G_activity_stack->prev == NULL);

#ifndef NDEBUG
    printf("register app count:%d\r\n", G_registered_count); // 应用数量(主页面也算)
#endif
}

/**
 * @brief 活动注册(把应用添加到G_registered_defs活动列表)
 * @param activity_def 需要注册的活动(第一个注册的必须是是主页面)
 */
void act_manager_register(const app_def_t *activity_def)
{

    if (G_registered_count < MAX_REGISTERED_ACTIVITIES)
    {
        G_registered_defs[G_registered_count] = activity_def;
        G_registered_count++;

        // 第一个注册的必须是是主页面
        if (G_home_activity_def == NULL)
        {
            G_home_activity_def = activity_def;
        }
    }
}

/**
 * @brief 活动跳转
 * @param new_app 要切换到的app定义
 * @param user_data 传递给create回调的自定义数据(可选)
 */
void act_manager_switch_to(app_def_t *new_app, void *user_data)
{
    if (!new_app){
        return;
    }
#ifndef NDEBUG
    printf("switch_to_%s\r\n", new_app->name);
#endif

    // 如果当前有活动正在运行，则调用它的 pause 回调
    if (G_activity_stack != NULL && G_activity_stack->App->pause){
        G_activity_stack->App->pause(G_activity_stack);
    }

    /* 活动包含了指向app定义的指针,app屏幕,活动时间,链表指针
     * 不包含应用定义,释放时主要是要释放屏幕占用内存
     */
    activity_t *new_activity = (activity_t *)lv_mem_alloc(sizeof(activity_t));
    assert(new_activity != NULL);
#ifndef NDEBUG
    printf("malloc_success for %s\r\n", new_app->name);
#endif

    // 初始化新活动的所有内容
    new_activity->App = new_app;
    new_activity->last_used_time = lv_tick_get();
    new_activity->prev = G_activity_stack; // 将新活动的prev指向当前的栈顶(指向前一个活动应用)
    new_activity->screen = new_activity->App->create();
    if (new_activity->screen == NULL){
#ifndef NDEBUG
        printf("create() for %s returned NULL screen\r\n", new_app->name);
#endif
        lv_mem_free(new_activity);
        return;
    }

    // 将新活动压入栈顶
    G_activity_stack = new_activity;

    // 加载新屏幕
    lv_scr_load_anim(new_activity->screen, LV_SCR_LOAD_ANIM_FADE_IN, 300, 0, false);
}

/**
 * @brief 返回到上一个活动
 */
void act_manager_go_back(void)
{
    // 栈为空，或只有一个元素(Home)，则无法返回
    if (G_activity_stack == NULL || G_activity_stack->prev == NULL){
        return;
    }

    // 1. 获取当前活动和前一个活动
    activity_t *current_activity = G_activity_stack;
    activity_t *prev_activity = current_activity->prev;

#ifndef NDEBUG
    printf("Go back from '%s' to '%s'\r\n", current_activity->App->name, prev_activity->App->name);
#endif

    // 2. 将活动栈指针指回上一个活动
    G_activity_stack = prev_activity;

    // 3. 如果上一个活动有 resume 回调，则调用它
    if (prev_activity->App->resume){
        prev_activity->App->resume(prev_activity);
    }

    // 4. 切换回上一个屏幕，并让LVGL在动画结束后自动删除当前屏幕的UI对象
    lv_scr_load_anim(prev_activity->screen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, true);

    // 5. 调用当前活动的 destroy 回调 (如果有)，并释放其活动实例的内存
    //    注意：UI (screen) 的删除由 lv_scr_load_anim 负责
    if (current_activity->App->destroy){
        current_activity->App->destroy(current_activity);
    }
    lv_mem_free(current_activity);
}

/**
 * @brief 返回到主页面
 */
void act_manager_go_home(void)
{
    if (G_activity_stack == NULL || G_activity_stack->prev == NULL)
    {
        return;
    }

    // 直接切换到Home屏幕，并让LVGL负责删除所有旧屏幕
    // 找到Home活动
    activity_t *home_activity = G_activity_stack;
    while (home_activity->prev != NULL)
    {
        home_activity = home_activity->prev;
    }

    // 加载Home屏幕，并设置动画结束后自动删除旧的活动屏幕
    lv_scr_load_anim(home_activity->screen, LV_SCR_LOAD_ANIM_OUT_TOP, 300, 0, true);

    // 释放除了Home之外的所有活动实例
    activity_t *current = G_activity_stack;
    while (current != home_activity)
    {
        activity_t *to_delete = current;

        printf("[Info]Act_manager: free \"%s\"\r\n", to_delete->App->name);

        current = current->prev;
        if (to_delete->App->destroy)
        {
            to_delete->App->destroy(to_delete);
        }
        lv_mem_free(to_delete);
    }

    // 将活动栈指针重置回Home
    G_activity_stack = home_activity;
}

/**
 * @brief 活动管理器的后台任务，应在主循环中定期调用
 *        负责处理UI的自动释放等
 */
void act_manager_task(void);

/* =================================================== 回调函数 ============================================= */

/**
 * @brief 全局边缘手势回调
 * @param e
 */
static void edge_gesture_event_cb(lv_event_t *e)
{
#ifndef NDEBUG
    printf("gesture_event\r\n");
#endif
    lv_event_code_t code = lv_event_get_code(e); // 获取触发事件代码
    lv_indev_t *indev = lv_indev_get_act();      // 获取处理设备
    if (indev == NULL)
        return;

    lv_indev_get_point(indev, &G_gesture_state.now_pos); // 获取此时的点
    /* 当手指按下时 */
    if (code == LV_EVENT_PRESSED)
    {
        // 记录起始位置和状态
        G_gesture_state.target_hotzone = lv_event_get_target(e); // 获取最初触发源(按下区域)
        lv_indev_get_point(indev, &G_gesture_state.start_pos);   // 获取按下时的点
        G_gesture_state.is_pressing = true;

        // 如果底部区域在菜单存在时被按下就触发关闭流程
        if (G_gesture_state.target_hotzone == G_bottom_hotzone && popup_is_pd_menu_active() == true)
        {
            popup_pulldown_menu_begin_close();
            lv_indev_reset(indev, NULL); // 中断当前交互，重新判断按下部件
        }
    }
    /* 按下后没有松开 */
    else if (code == LV_EVENT_PRESSING)
    {
        if (!G_gesture_state.is_pressing)
            return;

        lv_point_t now_pos;
        lv_indev_get_point(indev, &now_pos);
        lv_coord_t dy = now_pos.y - G_gesture_state.start_pos.y;

        // 条件：是顶部热区 && 向下滑动超过阈值 && 菜单尚未被创建
        if (G_gesture_state.target_hotzone == G_top_hotzone && dy > EDGE_GESTURE_THRESHOLD && popup_is_pd_menu_active() == false)
        {
            // 创建菜单
            pulldown_menu_params_t params;
            params.base.type = POPUP_PARAM_TYPE_PULLDOWN_MENU; // 指定数据类型
            params.initial_brightness = brightness_get_value();
            params.initial_volume = volume_get_value();
#ifndef NDEBUG
            printf("gesture_event:get_brightness= %d\r\n", params.initial_brightness);
#endif
            // 创建菜单，并将参数传递过去
            popup_show_pulldown_menu((const popup_param_base_t *)&params);

            // 立刻移交事件所有权
            lv_indev_reset(indev, NULL);
        }
    }

    /* 当手指抬起时 */
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        // 进行手势分析
        if (!G_gesture_state.is_pressing)
        {
            return; // 如果不是从有效的按压状态开始,则忽略
        }
        lv_point_t end_pos;                  // 创建结束点
        lv_indev_get_point(indev, &end_pos); // 获取此时(松开时的点)的坐标
        // 计算移动距离
        lv_coord_t dx = end_pos.x - G_gesture_state.start_pos.x;
        lv_coord_t dy = end_pos.y - G_gesture_state.start_pos.y; // ps:屏幕底部的 y 比顶部大

        if (popup_is_pd_menu_active() == false)
        { // 下拉菜单不存在
            // 事件判断
            // 1.超过了最小触发距离  2.是垂直滑动的(防止翻页时误触)
            if (LV_ABS(dy) > EDGE_GESTURE_THRESHOLD && LV_ABS(dy) > LV_ABS(dx) * 1.5)
            {

                // 底部,且向上
                if (G_gesture_state.target_hotzone == G_bottom_hotzone && dy < 0)
                {
#ifndef NDEBUG
                    printf("gesture_event: Go Home\r\n");
#endif
                    act_manager_go_home(); // 在这里调用返回主页函数
                }
            }
        }
        // 重置手势状态
        G_gesture_state.is_pressing = false;
        G_gesture_state.target_hotzone = NULL;
    }
}

/**
 * @brief 定时器回调,处理后台释放
 * @param t 参数
 */
void act_timer_handler(lv_timer_t *t)
{
}

/* ======================================================== 外部调用api ============================================================= */
/**
 * @brief 设置当前的亮度值
 * @param bright 0~100
 */
void activity_bright_set(uint8_t bright)
{
    // 设置亮度遮罩（测试）
    lv_opa_t value = 0;
    value = lv_map(bright, 0, 100, LV_OPA_COVER, LV_OPA_TRANSP);
    if (value > 240)
    {
        value = 240;
    }
#ifndef NDEBUG
    printf("bright_set_opa:%d\r\n", value);
#endif
    lv_obj_set_style_bg_opa(Bright_mask, value, LV_PART_MAIN);
}

/**
 * @brief 获取当前的亮度值
 * @param bright 0~100
 */
uint8_t brightness_get_value(void)
{
    lv_opa_t value = lv_obj_get_style_bg_opa(Bright_mask, LV_PART_MAIN); // 从遮罩获取亮度（测试）
#ifndef NDEBUG
    printf("get_bright_opa:%d\r\n", value);
#endif
    uint8_t bright = lv_map(value, 240, LV_OPA_TRANSP, 0, 100);
#ifndef NDEBUG
    printf("bright_set_slider:%d\r\n", bright);
#endif
    return bright;
}

uint8_t volume_get_value(void)
{
    return volume; // 返回当前音量值
}

/**
 * @brief 设置当前的音量值
 * @param vol 0~100
 */
void volume_set_value(uint8_t vol)
{
    if (vol > 100)
    {
        vol = 100; // 限制最大值
    }
    volume = vol; // 设置音量
}

/**
 * @brief 根据名称查找已注册的app
 * @param name app名称
 * @return app定义
 */
const app_def_t *find_def_by_name(const char *name)
{
    for (int i = 0; i < G_registered_count; i++)
    {
        if (strcmp(G_registered_defs[i]->name, name) == 0)
        {
            return G_registered_defs[i];
        }
    }
    return NULL;
}

/**
 * @brief 根据索引获取一个app的定义
 * @param index 索引值 (0 to count-1)
 * @return const app_def_t* 如果索引有效，则返回app定义的只读指针，否则返回NULL
 */
const app_def_t *activity_manager_get_def_by_index(uint8_t index)
{
    // 安全检查，防止越界访问
    if (index < G_registered_count)
    {
        return G_registered_defs[index];
    }
    return NULL;
}

/**
 * @brief 获取已注册的app数量
 * @return app数量
 */
uint8_t activity_manager_get_registered_count(void)
{
    return G_registered_count;
}
