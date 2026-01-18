/*
 * app_config.h
 *
 *  Created on: Jan 18, 2026
 *      Author: 12114
 */

#ifndef HOME_SYSTEM_APP_SETTINGS_H_
#define HOME_SYSTEM_APP_SETTINGS_H_

#include "project_cfg.h"
#include "stdbool.h"
#include "stdint.h"


#if USE_Simulator
#include <stdlib.h>
#include <string.h>
#define sys_malloc(tag, size) malloc(size)
#define sys_free(tag, ptr) free(ptr)
#define SYS_MEM_INTERNAL 0
#else
#include "sys.h"
#endif
enum app_config_type_t {
  APP_CONFIG_TYPE_INT,
  APP_CONFIG_TYPE_STRING,
  APP_CONFIG_TYPE_BOOL,
  APP_CONFIG_TYPE_DOUBLE,
};

typedef struct attribute {
  unsigned is_dirty : 1;
  unsigned is_loaded : 1;
} attribute;

typedef struct app_config_t {
  uint16_t key;
  uint8_t type;
  union {
    uint32_t Int;
    char *string;
    bool Bool;
    double Double;
  };
} app_config_t;

typedef struct app_settings_t {
  app_config_t *configs;
  uint32_t hash;
  uint8_t count;
  attribute attr;
} app_settings_t;

int app_settings_load(const char *app_name, const char *config_file_name);
int app_settings_save(const char *app_name, const char *config_file_name);
int app_settings_update(const char *app_name, const app_settings_t *settings);

#endif /* HOME_SYSTEM_APP_SETTINGS_H_ */
