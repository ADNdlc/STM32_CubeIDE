#ifndef _CORE_APP_H
#define _CORE_APP_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_t; // Forward declaration

// Callbacks
typedef lv_obj_t *(*app_create_cb)(void);
typedef void (*app_destroy_cb)(struct app_t *app);
typedef void (*app_pause_cb)(struct app_t *app);
typedef void (*app_resume_cb)(struct app_t *app);

// The immutable definition of an app (registered at startup)
typedef struct {
  const char *name;
  const void *icon;
  app_create_cb create;
  app_destroy_cb destroy;
  app_pause_cb pause;
  app_resume_cb resume;
} app_def_t;

// The runtime instance of an app
typedef struct app_t {
  const app_def_t *def;
  lv_obj_t *root_obj;
  void *user_data;
  struct app_t *prev; // Linked list for stack
} app_t;

#ifdef __cplusplus
}
#endif

#endif
