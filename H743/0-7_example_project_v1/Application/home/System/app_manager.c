#include "app_manager.h"
#include <string.h>
#include "elog.h"

#define LOG_TAG "APP_MGR"
#include "elog.h"

#define MAX_APPS 20 // 最大注册app数量

static const app_def_t *registry[MAX_APPS]; // app注册表
static int registry_count = 0;              // 注册数量

static app_t *app_stack = NULL; // 活动app实例栈顶(当前运行的app)

void app_manager_init(void) {
  registry_count = 0;
  app_stack = NULL;
  log_i("Initialized");
}

void app_manager_register(const app_def_t *app_def) {
  if (registry_count < MAX_APPS && app_def) {
    registry[registry_count++] = app_def;
    log_i("Registered app: %s", app_def->name);
  } else {
    log_w("Failed to register app, registry full or invalid app");
  }
}

int app_manager_get_app_count(void) { 
  log_d("App count: %d", registry_count);
  return registry_count; 
}

const app_def_t *app_manager_get_app_by_index(int index) {
  if (index >= 0 && index < registry_count)
    return registry[index];
  log_w("Invalid index %d", index);
  return NULL;
}

const app_def_t *app_manager_find_by_name(const char *name) {
  for (int i = 0; i < registry_count; i++) {
    if (strcmp(registry[i]->name, name) == 0) {
      log_d("Found app by name: %s", name);
      return registry[i];
    }
  }
  log_w("App not found by name: %s", name);
  return NULL;
}

// Internal: 释放一个app实例
static void free_app_instance(app_t *app) {
  if (!app)
    return;
  if (app->def->destroy)
    app->def->destroy(app); // 调用app的destroy回调
  // Note: LVGL objects are usually deleted by screen transition or parent
  // deletion but if the app created a screen, it should be deleted. If the
  // transition deleted it, fine. If not, we might need manual cleanup. Assuming
  // standard transition deletes old screen if properly configured.
  lv_mem_free(app);
}

// 创建app的实例并运行
void app_manager_start_app(const char *name) {

  const app_def_t *def = app_manager_find_by_name(name); // 查找app定义
  if (!def)
    return;

  // 挂起当前app
  if (app_stack && app_stack->def->pause) {
    app_stack->def->pause(app_stack);
  }

  // lvgl相关内存都使用lv_mem_alloc申请，需保证lv_conf配置了足够大小
  app_t *new_app = lv_mem_alloc(sizeof(app_t));
  if (!new_app) {
    LV_LOG_ERROR("Failed to allocate memory for app: %s", name);
    return;
  }

  new_app->def = def;
  new_app->root_obj = def->create(); // 调用app的create回调
  new_app->user_data = NULL;

  new_app->prev = app_stack;
  app_stack = new_app; // Push

  // 使用Animate切换屏幕
  if (new_app->root_obj) {
    lv_scr_load_anim(new_app->root_obj, LV_SCR_LOAD_ANIM_FADE_IN, 300, 0,
                     false); // 关闭自动删除
  } else {
    LV_LOG_WARN("App %s created with NULL root object", name);
  }
}

void app_manager_go_back(void) {
  if (!app_stack || !app_stack->prev)
    return; // Cannot go back from root (Home)

  app_t *current = app_stack;  // 获取栈顶实例
  app_t *prev = current->prev; // 上一个实例

  app_stack = prev; // Pop

  // 恢复上一个实例
  if (prev->def->resume) {
    prev->def->resume(prev);
  }

  // Animate back
  if (prev->root_obj) {
    lv_scr_load_anim(prev->root_obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0,
                     true); // 开启自动删除
  }

  // 释放当前实例
  if (current->def->destroy) {
    current->def->destroy(current);
  }
  lv_mem_free(current);
}

void app_manager_go_home(void) {
  if (!app_stack)
    return;

  // 获取到栈底实例,即主界面
  app_t *home = app_stack;
  while (home->prev) {
    home = home->prev;
  }

  if (app_stack == home)
    return; // Already home

  // 切换回主界面
  if (home->root_obj) {
    lv_scr_load_anim(home->root_obj, LV_SCR_LOAD_ANIM_OUT_TOP, 300, 0, true);
  }

  // 释放除栈底的所有实例
  while (app_stack != home) {
    app_t *temp = app_stack;
    app_stack = app_stack->prev;

    if (temp->def->destroy)
      temp->def->destroy(temp);
    lv_mem_free(temp);
  }

  if (home->def->resume)
    home->def->resume(home);
}
