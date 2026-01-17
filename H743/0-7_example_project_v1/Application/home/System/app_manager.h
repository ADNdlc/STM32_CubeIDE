#ifndef _APP_MANAGER_H
#define _APP_MANAGER_H

#include "core_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief App Manager
 *
 * 此模块负责管理所有app_ui实例的生命周期和导航。
 *
 */

// Init
void app_manager_init(void);

// app注册与查询
void app_manager_register(const app_def_t *app_def,
                          int page_index); // 注册一个app并指定初始页码
int app_manager_get_app_count(void);       // 获取注册app数量
const app_def_t *app_manager_get_app_by_index(int index); // 通过索引获取app定义
const app_def_t *
app_manager_find_by_name(const char *name); // 通过名称获取app定义

// 布局控制
int app_manager_get_page_index(const app_def_t *app_def); // 获取指定app的页码
void app_manager_set_page_index(const char *name,
                                int page_index); // 运行时修改app的页码

// app控制
void app_manager_start_app(const char *name); // 运行一个app
void app_manager_go_back(void);               // 返回上一个app
void app_manager_go_home(void);               // 返回主界面

// 内部页面控制
void app_manager_push_screen(lv_obj_t *obj); // 在当前app推入新页面
void app_manager_pop_screen(void);           // 弹出当前页面(返回上一个页面)

#ifdef __cplusplus
}
#endif

#endif
