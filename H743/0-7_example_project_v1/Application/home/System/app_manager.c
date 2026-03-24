#include "app_manager.h"
#include "app_settings.h"
#include "elog.h"
#include <string.h>

#define LOG_TAG "APP_MGR"
#include "elog.h"

#define MAX_APPS 10 // 最大注册app数量

// @brief app注册表项
typedef struct {
  const app_def_t *def;
  int page_index;
} registered_app_t;
static registered_app_t registry[MAX_APPS]; // app注册表
static int registry_count = 0;              // 注册数量
static app_t *app_stack = NULL;             // 活动app实例栈顶(当前运行的app)

void app_manager_init(void) {
  registry_count = 0;
  app_stack = NULL;
  log_i("Initialized");
}

static uint32_t djb2_hash(const char *str) {
  if (str == NULL) {
    return 0;
  }
  uint32_t hash = 5381;
  int c;
  // 遍历字符串（自动处理'\0'结束符）
  while ((c = (unsigned char)*str++)) {
    // 等价于 hash = hash * 33 + c（位运算优化，速度更快）
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

/**
 * @brief 注册app
 *
 * @param app_def    app定义
 * @param page_index app在启动时加载的页面索引
 */
void app_manager_register(app_def_t *app_def, int page_index) {
  if (registry_count < MAX_APPS && app_def && app_def->name) {
    // 检查是否有重复的应用名称
    for (int i = 0; i < registry_count; i++) {
      if (strcmp(registry[i].def->name, app_def->name) == 0) {
        log_w("App with name '%s' already registered, skipping registration",
              app_def->name);
        return;
      }
    }
    // 计算app名称的hash值
    uint32_t app_name_hash = djb2_hash(app_def->name);

    // 添加app_def到注册表
    registry[registry_count].def = app_def;
    registry[registry_count].page_index = page_index;
    registry_count++;

    if (app_def->settings) { // 有配置选项
      // 设置app_def中的settings的hash值
      ((app_def_t *)app_def)->settings->hash = app_name_hash;
      int res = app_settings_load(app_def->name,
                                  app_def->name); // 直接使用app_name作为文件名
      if (res == 0) {
        app_def->settings->attr.is_loaded = true;
      } else {
        app_def->settings->attr.is_loaded = false;
        log_w("Failed to load settings for app: %s", app_def->name);
      }
      app_def->settings->attr.is_dirty = 0;
    }

    log_i("Registered app: %s (hash: %u) at page: %d", app_def->name,
          app_name_hash, page_index);
  } else {
    log_w("Failed to register app, registry full or invalid app");
  }
}

/**
 * @brief 获取已注册的app数量
 *
 * @return int app数量
 */
int app_manager_get_app_count(void) {
  log_d("App count: %d", registry_count);
  return registry_count;
}

/**
 * @brief 通过索引获取已注册的app定义
 *
 * @param index app索引
 * @return const app_def_t* app定义
 */
const app_def_t *app_manager_get_app_by_index(int index) {
  if (index >= 0 && index < registry_count)
    return registry[index].def;
  log_w("Invalid index %d", index);
  return NULL;
}

/**
 * @brief 通过名称获取已注册的app定义
 *
 * @param name app名称
 * @return const app_def_t* app定义
 */
const app_def_t *app_manager_find_by_name(const char *name) {
  for (int i = 0; i < registry_count; i++) {
    if (strcmp(registry[i].def->name, name) == 0) {
      log_d("Found app by name: %s", name);
      return registry[i].def;
    }
  }
  log_w("App not found by name: %s", name);
  return NULL;
}

/**
 * @brief 通过app定义获取其启动时加载的页面索引
 *
 * @param app_def app定义
 * @return int 页面索引
 */
int app_manager_get_page_index(const app_def_t *app_def) {
  if (!app_def)
    return -1;
  for (int i = 0; i < registry_count; i++) {
    if (registry[i].def == app_def) {
      return registry[i].page_index;
    }
  }
  return -1;
}

/**
 * @brief 更新已注册的app的启动时加载的页面索引
 *
 * @param name app名称
 * @param page_index 新的页面索引
 */
void app_manager_set_page_index(const char *name, int page_index) {
  if (!name)
    return;
  for (int i = 0; i < registry_count; i++) {
    if (strcmp(registry[i].def->name, name) == 0) {
      registry[i].page_index = page_index;
      log_i("App %s page index updated to %d", name, page_index);
      return;
    }
  }
}

/**
 * @brief 释放app实例的屏幕栈
 *
 * @param app app实例
 * @param skip_obj 需要跳过不删除的对象(通常是正在进行切换动画的活跃窗口)
 */
static void free_screen_stack(app_t *app, lv_obj_t *skip_obj) {
  if (!app)
    return;
  screen_node_t *node = app->screen_stack;
  while (node) {
    screen_node_t *tmp = node;
    node = node->next;
    if (tmp->obj) {
      if (tmp->obj != skip_obj) {
        lv_obj_del(tmp->obj);
      } else {
        // 如果某处正在使用该对象(如动画)，则设置为自动删除，由由底层清除
        // 或者由于 lv_scr_load_anim(..., true)
        // 已经处理了清理，这里只需要确保链表节点被释放
      }
    }
    lv_mem_free(tmp);
  }
  app->screen_stack = NULL;
}

/**
 * @brief 释放app实例
 *
 * @param app app实例
 */
static void free_app_instance(app_t *app, lv_obj_t *skip_obj) {
  if (!app)
    return;
  // 1. 检查并保存设置
  if (app->def->settings && app->def->settings->attr.is_dirty &&
      app->def->settings->attr.is_loaded) {
    app_settings_save(app->def->name, app->def->name); // 保存配置
    app->def->settings->attr.is_dirty = false;
  }
  // 2. 销毁应用实例
  if (app->def->destroy)
    app->def->destroy(app); // 调用app的destroy回调

  // 3. 释放屏幕栈
  free_screen_stack(app, skip_obj);

  // 4. 释放 app 对象内存
  lv_mem_free(app);
}

/**
 * @brief 通过名称启动app
 *
 * @param name app名称
 */
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

/**
 * @brief 将页面压入当前app实例的屏幕栈
 *
 */
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
  lv_scr_load_anim(obj, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
}

/**
 * @brief 将页面从当前app实例的屏幕栈弹出
 *
 */
void app_manager_pop_screen(void) {
  if (!app_stack || !app_stack->screen_stack)
    return;

  // 如果内部栈还有超过一个页面
  if (app_stack->screen_stack->next != NULL) {
    screen_node_t *popped = app_stack->screen_stack;
    app_stack->screen_stack = popped->next;

    log_d("Pop screen from app: %s", app_stack->def->name);
    lv_scr_load_anim(app_stack->screen_stack->obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                     200, 0, true);

    // 触发 resume 回调以便应用刷新内容
    if (app_stack->def->resume) {
      app_stack->def->resume(app_stack);
    }

    // Note: lv_scr_load_anim with auto_del=true will delete the old screen
    // (popped->obj)
    lv_mem_free(popped);
  } else {
    // 只有一个页面，执行应用级返回
    app_manager_go_back();
  }
}

/**
 * @brief 返回到上一个app实例
 *
 */
void app_manager_go_back(void) {
  if (!app_stack || !app_stack->prev)
    return;

  app_t *current = app_stack;
  app_stack = current->prev;

  log_i("App back: %s -> %s", current->def->name, app_stack->def->name);

  if (app_stack->def->resume) {
    app_stack->def->resume(app_stack);
  }

  lv_obj_t *act_scr = lv_scr_act();

  // 加载上一个app的栈顶页面
  if (app_stack->screen_stack) {
    lv_scr_load_anim(app_stack->screen_stack->obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                     300, 0, true);
  }

  // 销毁当前应用及其所有内部屏幕，包含自动保存设置
  free_app_instance(current, act_scr);
}

/**
 * @brief 返回到主app实例(弹出除主页的所有app实例)
 *
 */
void app_manager_go_home(void) {
  if (!app_stack)
    return;

  app_t *home = app_stack;
  while (home->prev)
    home = home->prev;

  if (app_stack == home)
    return;

  lv_obj_t *act_scr = lv_scr_act();

  // 寻找 Home App 的最底层页面（初始页面）
  screen_node_t *bottom = home->screen_stack;
  while (bottom && bottom->next) {
    bottom = bottom->next;
  }

  if (bottom) {
    lv_scr_load_anim(bottom->obj, LV_SCR_LOAD_ANIM_OUT_TOP, 300, 0, true);

    // 清理 Home App 内部多余的页面栈（除了最底层那个）
    screen_node_t *n = home->screen_stack;
    while (n != bottom) {
      screen_node_t *t = n;
      n = n->next;
      if (t->obj != act_scr)
        lv_obj_del(t->obj); // 如果由于某种原因首页不是活跃页，正常删
      lv_mem_free(t);
    }
    home->screen_stack = bottom;
  }

  while (app_stack != home) {
    app_t *temp = app_stack;
    app_stack = app_stack->prev;
    free_app_instance(temp, act_scr);
  }

  if (home->def->resume)
    home->def->resume(home);
}
