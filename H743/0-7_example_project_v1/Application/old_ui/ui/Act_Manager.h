#ifndef __Act_Manager_H__
#define __Act_Manager_H__

#ifdef __cplusplus
extern "C"
{
#endif

//#define NDEBUG

#include "lvgl.h"
#include <assert.h> //断言
#ifndef NDEBUG
#include <stdio.h>
#endif

    struct activity_t; // 活动声明
    /* -------------------------------------------- 应用接口定义 --------------------------------------------- */
    // 定义活动生命周期回调函数指针
    // 这些接口函数由每个具体的应用去实现
    typedef lv_obj_t *(*app_create_cb)(void);                    // 创建UI (返回屏幕对象)
    typedef void (*app_destroy_cb)(struct activity_t *activity); // 销毁UI
    typedef void (*app_pause_cb)(struct activity_t *activity);   // 暂停 (当被新活动覆盖时调用)
    typedef void (*app_resume_cb)(struct activity_t *activity);  // 恢复 (当返回到此活动时调用)

    // 应用定义结构体 (应用的“类”)
    typedef struct
    {
        const char *name;       // 活动的唯一名称/ID
        const void *icon;       // 指向图标资源的指针
        app_create_cb create;   // 创建UI
        app_destroy_cb destroy; // 销毁UI
        app_pause_cb pause;     // 暂停
        app_resume_cb resume;   // 恢复
    } app_def_t;

    /**
     * @brief 初始化活动管理器
     */
    void act_manager_init(void);

    /**
     * @brief 注册一个app定义 (在系统启动时调用)
     * @param activity_def 指向一个静态的活动定义结构体
     */
    void act_manager_register(const app_def_t *activity_def);

    /**
     * @brief 创建并切换到一个新的活动
     * @param name 要启动的活动的app
     * @param user_data 传递给create回调的自定义数据 (可选)
     */
    void act_manager_switch_to(app_def_t *new_app, void *user_data);

    /**
     * @brief 返回到上一个活动
     */
    void act_manager_go_back(void);

    /**
     * @brief 返回到主页面
     */
    void act_manager_go_home(void);

    /**
     * @brief 活动管理器的后台任务，应在主循环中定期调用
     *        负责处理UI的自动释放等
     */
    void act_manager_task(void);

    /* ======================================================== 外部调用api ============================================================= */
    /**
     * @brief 设置当前的亮度值
     * @param bright 0~100
     */
    void activity_bright_set(uint8_t bright);

    /**
     * @brief 获取当前的亮度值
     * @param bright 0~100
     */
    uint8_t brightness_get_value(void);

    /**
     * @brief 获取当前的音量值
     * @return 0~100
     */
    uint8_t volume_get_value(void);

    /**
     * @brief 设置当前的音量值
     * @param vol 0~100
     */
    void volume_set_value(uint8_t vol);

    /**
     * @brief 根据名称查找已注册的app
     * @param name app名称
     * @return app定义
     */
    const app_def_t *find_def_by_name(const char *name);

    /**
     * @brief 根据索引获取一个app的定义
     * @param index 索引值 (0 to count-1)
     * @return const app_def_t* 如果索引有效，则返回app定义的只读指针，否则返回NULL
     */
    const app_def_t *activity_manager_get_def_by_index(uint8_t index);

    /**
     * @brief 获取已注册的app数量
     * @return app数量
     */
    uint8_t activity_manager_get_registered_count(void);

#ifdef __cplusplus
}
#endif

#endif
