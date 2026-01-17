#include "app_manager.h"
#include "elog.h"
#include <string.h>

#define LOG_TAG "APP_MGR"
#include "elog.h"

#define MAX_APPS 10 // 最大注册app数量

typedef struct {
  const app_def_t *def;
  int page_index;
} registered_app_t;

static registered_app_t registry[MAX_APPS]; // app注册表
static int registry_count = 0;              // 注册数量

static app_t *app_stack = NULL; // 活动app实例栈顶(当前运行的app)

void app_manager_init(void) {
  registry_count = 0;
  app_stack = NULL;
  log_i("Initialized");
}

// ... (getters/setters remain same)

// 内部：释放屏幕栈
static void free_screen_stack(app_t *app) {
  if (!app)
    return;
  screen_node_t *node = app->screen_stack;
  while (node) {
    screen_node_t *tmp = node;
    node = node->next;
    if (tmp->obj) {
      lv_obj_del(tmp->obj);
    }
    lv_mem_free(tmp);
  }
  app->screen_stack = NULL;
}

// 创建app的实例并运行
void app_manager_start_app(const char *name) {
  const app_def_t *def = app_manager_find_by_name(name);
  if (!def)
    return;

  // 挂起当前app
  if (app_stack && app_stack->def->pause) {
    app_stack->def->pause(app_stack);
  }

  app_t *new_app = lv_mem_alloc(sizeof(app_t));
  if (!new_app) {
    log_e("Failed to allocate app: %s", name);
    return;
  }

  new_app->def = def;
  new_app->screen_stack = NULL;
  new_app->user_data = NULL;
  new_app->prev = app_stack;
  app_stack = new_app;

  // 调用首次创建
  lv_obj_t *root = def->create();
  if (root) {
    app_manager_push_screen(root);
  } else {
    log_w("App %s created with NULL screen", name);
  }
}

void app_manager_push_screen(lv_obj_t *obj) {
  if (!app_stack || !obj)
    return;

  screen_node_t *node = lv_mem_alloc(sizeof(screen_node_t));
  if (!node)
    return;

  node->obj = obj;
  node->next = app_stack->screen_stack;
  app_stack->screen_stack = node;

  log_d("Push screen to app: %s", app_stack->def->name);
  lv_scr_load_anim(obj, LV_SCR_LOAD_ANIM_FADE_IN, 300, 0, false);
}

void app_manager_pop_screen(void) {
  if (!app_stack || !app_stack->screen_stack)
    return;

  // 如果内部栈还有超过一个页面，或者我们决定总是优先内部弹栈
  if (app_stack->screen_stack->next != NULL) {
    screen_node_t *popped = app_stack->screen_stack;
    app_stack->screen_stack = popped->next;

    log_d("Pop screen from app: %s", app_stack->def->name);
    lv_scr_load_anim(app_stack->screen_stack->obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                     300, 0, true);

    // Note: lv_scr_load_anim with auto_del=true will delete the old screen
    // (popped->obj)
    lv_mem_free(popped);
  } else {
    // 只有一个页面，执行应用级返回
    app_manager_go_back();
  }
}

void app_manager_go_back(void) {
  if (!app_stack || !app_stack->prev)
    return;

  app_t *current = app_stack;
  app_stack = current->prev;

  log_i("App back: %s -> %s", current->def->name, app_stack->def->name);

  if (app_stack->def->resume) {
    app_stack->def->resume(app_stack);
  }

  // 加载上一个app的栈顶页面
  if (app_stack->screen_stack) {
    lv_scr_load_anim(app_stack->screen_stack->obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                     300, 0, true);
  }

  // 销毁当前应用及其所有内部屏幕
  if (current->def->destroy) {
    current->def->destroy(current);
  }
  free_screen_stack(current);
  lv_mem_free(current);
}

void app_manager_go_home(void) {
  if (!app_stack)
    return;

  app_t *home = app_stack;
  while (home->prev)
    home = home->prev;

  if (app_stack == home)
    return;

  if (home->screen_stack) {
    lv_scr_load_anim(home->screen_stack->obj, LV_SCR_LOAD_ANIM_OUT_TOP, 300, 0,
                     true);
  }

  while (app_stack != home) {
    app_t *temp = app_stack;
    app_stack = app_stack->prev;
    if (temp->def->destroy)
      temp->def->destroy(temp);
    free_screen_stack(temp);
    lv_mem_free(temp);
  }

  if (home->def->resume)
    home->def->resume(home);
}
