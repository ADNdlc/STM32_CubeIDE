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
#include "Sys.h"
#endif

// 数据类型
enum app_config_type_t {
  APP_CONFIG_TYPE_INT,
  APP_CONFIG_TYPE_STRING,
  APP_CONFIG_TYPE_BOOL,
  APP_CONFIG_TYPE_DOUBLE,
};
// 数据状态
typedef struct attribute {
  unsigned is_dirty : 1; // 已更新
  unsigned is_loaded : 1;// 已加载
} attribute;
// 配置项
typedef struct app_config_t {
  uint16_t key;	// 配置号
  uint8_t type; // 数据类型
  union {
    uint32_t Int;
    char *string;
    bool Bool;
    float Float;
  };
} app_config_t;

// 应用配置
typedef struct app_settings_t {
  app_config_t *configs; // 配置表
  uint32_t hash;		 // 校验
  uint8_t count;	     // 表项数量
  attribute attr;		 // 状态
} app_settings_t;

int app_settings_load(const char *app_name, const char *config_file_name);
int app_settings_save(const char *app_name, const char *config_file_name);
int app_settings_update(const char *app_name, const app_settings_t *settings);

#endif /* HOME_SYSTEM_APP_SETTINGS_H_ */
