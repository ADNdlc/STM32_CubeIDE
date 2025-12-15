#include "app_manager.h"
#include <stdio.h>
#include <string.h>

#define MAX_APPS 20

static const app_def_t *registry[MAX_APPS];
static int registry_count = 0;

static app_t *app_stack = NULL; // Top of stack

void app_manager_init(void) {
  registry_count = 0;
  app_stack = NULL;
}

void app_manager_register(const app_def_t *app_def) {
  if (registry_count < MAX_APPS) {
    registry[registry_count++] = app_def;
  }
}

int app_manager_get_app_count(void) { return registry_count; }

const app_def_t *app_manager_get_app_by_index(int index) {
  if (index >= 0 && index < registry_count)
    return registry[index];
  return NULL;
}

const app_def_t *app_manager_find_by_name(const char *name) {
  for (int i = 0; i < registry_count; i++) {
    if (strcmp(registry[i]->name, name) == 0)
      return registry[i];
  }
  return NULL;
}

// Internal: Free app instance
static void free_app_instance(app_t *app) {
  if (!app)
    return;
  if (app->def->destroy)
    app->def->destroy(app);
  // Note: LVGL objects are usually deleted by screen transition or parent
  // deletion but if the app created a screen, it should be deleted. If the
  // transition deleted it, fine. If not, we might need manual cleanup. Assuming
  // standard transition deletes old screen if properly configured.
  lv_mem_free(app);
}

void app_manager_start_app(const char *name) {
  const app_def_t *def = app_manager_find_by_name(name);
  if (!def)
    return;

  // Pause current
  if (app_stack && app_stack->def->pause) {
    app_stack->def->pause(app_stack);
  }

  // Create new
  app_t *new_app = lv_mem_alloc(sizeof(app_t));
  new_app->def = def;
  new_app->prev = app_stack;
  new_app->root_obj = def->create();
  new_app->user_data = NULL;

  app_stack = new_app; // Push

  // Animate
  if (new_app->root_obj) {
    lv_scr_load_anim(new_app->root_obj, LV_SCR_LOAD_ANIM_FADE_IN, 300, 0,
                     false);
  }
}

void app_manager_go_back(void) {
  if (!app_stack || !app_stack->prev)
    return; // Cannot go back from root (Home)

  app_t *current = app_stack;
  app_t *prev = current->prev;

  app_stack = prev; // Pop

  // Resume prev
  if (prev->def->resume) {
    prev->def->resume(prev);
  }

  // Animate back
  if (prev->root_obj) {
    // "auto_del: true" causes the current screen (current->root_obj) to be
    // deleted
    lv_scr_load_anim(prev->root_obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, true);
  }

  // Free current instance Logic
  // Since auto_del=true deletes the lv_obj, we just need to free the struct and
  // call destroy cb
  if (current->def->destroy) {
    current->def->destroy(current);
  }
  lv_mem_free(current);
}

void app_manager_go_home(void) {
  if (!app_stack)
    return;

  // Find Home (assuming bottom of stack)
  app_t *home = app_stack;
  while (home->prev) {
    home = home->prev;
  }

  if (app_stack == home)
    return; // Already home

  // Load Home Screen
  if (home->root_obj) {
    lv_scr_load_anim(home->root_obj, LV_SCR_LOAD_ANIM_OUT_TOP, 300, 0, true);
  }

  // Clean up everything in between
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
