#ifndef __POPUP_H__
#define __POPUP_H__

#ifdef __cplusplus
extern "C"
{
#endif

// #define NDEBUG
// #include <stdint.h>
#include "lvgl.h"
#include <stdbool.h>
#ifndef NDEBUG
#include <assert.h> //断言
#include <stdio.h>
#endif

    /**
     * @brief 弹窗显示区域枚举
     */
    typedef enum
    {
        POPUP_REGION_CENTER,     // 屏幕正中 (用于警告框)
        POPUP_REGION_TOP_MID,    // 顶部中间 (用于消息提示)
        POPUP_REGION_SIDE_LEFT,  // 左侧 (用于音量条等)
        POPUP_REGION_SIDE_RIGHT, // 右侧
        POPUP_REGION_BOTTOM_MID, // 底部中间
        POPUP_REGION_FULLSCREEN, // 全屏 (用于下拉菜单等)

        POPUP_REGION_COUNT // 用于数组大小，必须是最后一个
    } popup_region_t;

    /**
     * @brief 弹窗数据类型枚举
     */
    typedef enum
    {
        POPUP_PARAM_TYPE_NONE,          // 无数据
        POPUP_PARAM_TYPE_PULLDOWN_MENU, // 下拉菜单
        POPUP_PARAM_TYPE_ALERT,         // 警告框
        // ...
    } popup_param_type_t;

    // 数据类型
    typedef struct
    {
        popup_param_type_t type;
    } popup_param_base_t;


    // 下拉菜单传递数据
    typedef struct
    {
        popup_param_base_t base;    // 必须是第一个成员
        int32_t initial_brightness; // 初始亮度值 (0-255)
        int32_t initial_volume;     // 初始音量值 (0~100)
        // ...
    } pulldown_menu_params_t;

    // 弹窗关闭时的回调函数指针
    // 参数：closed_popup 是被关闭的弹窗对象, button_id 是被点击的按钮索引 (或-1如果非按钮关闭)
    typedef void (*popup_close_cb_t)(lv_obj_t *closed_popup, int32_t button_id, void *user_data);

    /**
     * @brief 初始化Popup管理器
     *        应在UI初始化时调用一次。
     */
    void popup_manager_init(void);

    /* ------------------- 预设的高级API ------------------- */
    /**
     * @brief 显示一个会自动消失的消息提示 (Toast)
     * @param text 消息内容
     * @param duration_ms 显示时长 (毫秒)
     */
    void popup_show_toast(const char *text, uint32_t duration_ms);
    /**
     * @brief 显示一个模态警告框 (Alert)
     * @param title 标题
     * @param text 内容
     * @param buttons 按钮文本数组，以"\n"分隔，如 "确定\n取消"
     * @param callback 点击按钮后的回调函数
     * @param user_data 传递给回调的自定义数据
     */
    void popup_show_alert(const char *title, const char *text, const char *buttons, popup_close_cb_t callback, void *user_data);
    /**
     * @brief 显示一个下拉菜单
     * @param 传递数据
     */
    void popup_show_pulldown_menu(const popup_param_base_t *params);
    /**
     * @brief 获取下拉菜单活动状态
     * @return 是否活动
     */
    bool popup_is_pd_menu_active(void);
    /**
     * @brief 通知Popup模块，开始拖动以关闭下拉菜单
     */
    void popup_pulldown_menu_begin_close(void);

    /* ------------------- 弹窗的核心API ------------------- */
    /**
     * @brief 显示一个由应用创建的自定义弹窗容器
     * @param popup     应用创建的 lv_obj_t* 容器
     * @param region    希望弹窗显示的区域
     * @param is_modal  是否阻断背景操作
     * @param duration_ms 自动关闭的时长 (0 表示不自动关闭)
     * @param callback  弹窗关闭时的回调
     * @param user_data 传递给回调的自定义数据
     */
    void popup_show_custom(lv_obj_t *popup, popup_region_t region, bool is_modal, uint32_t duration_ms, popup_close_cb_t callback, void *user_data);

    /**
     * @brief 手动关闭指定区域的弹窗
     * @param region 要关闭的区域,只能是popup_region_t枚举
     */
    void popup_close_by_region(popup_region_t region);

#ifdef __cplusplus
}
#endif

#endif
