#ifndef _CORE_APP_H
#define _CORE_APP_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_t; // Forward declaration

// 回调接口类型
typedef lv_obj_t *(*app_create_cb)(void);
typedef void (*app_destroy_cb)(struct app_t *app);
typedef void (*app_pause_cb)(struct app_t *app);
typedef void (*app_resume_cb)(struct app_t *app);

// app类定义
typedef struct {
  const char *name; // 名称
  const void *icon; // 图标(可缺省)
  // 必须实现的回调
  app_create_cb create;   // 创建
  app_destroy_cb destroy; // 销毁
  app_pause_cb pause;     // 暂停
  app_resume_cb resume;   // 恢复
} app_def_t;

// 屏幕栈节点
typedef struct screen_node_t {
  lv_obj_t *obj;
  struct screen_node_t *next;
} screen_node_t;

// app活动实例
typedef struct app_t {
  const app_def_t *def;        // app对象
  screen_node_t *screen_stack; // 内部屏幕栈 (Stack Top)
  void *user_data;             // 用户数据
  struct app_t *prev;          // App 列表指针
} app_t;

#ifdef __cplusplus
}
#endif

#endif
