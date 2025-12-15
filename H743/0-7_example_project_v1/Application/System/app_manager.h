#ifndef _APP_MANAGER_H
#define _APP_MANAGER_H

#include "core_app.h"

#ifdef __cplusplus
extern "C" {
#endif

// Init
void app_manager_init(void);

// Registration
void app_manager_register(const app_def_t *app_def);
int app_manager_get_app_count(void);
const app_def_t *app_manager_get_app_by_index(int index);

// Navigation
void app_manager_start_app(const char *name);
void app_manager_go_back(void);
void app_manager_go_home(void);

// Helper
const app_def_t *app_manager_find_by_name(const char *name);

#ifdef __cplusplus
}
#endif

#endif
