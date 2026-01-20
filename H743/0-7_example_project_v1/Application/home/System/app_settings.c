/*
 * app_config.c
 *
 *  Created on: Jan 18, 2026
 *      Author: 12114
 */
#include "app_settings.h"
#include "app_manager.h"
#include "project_cfg.h"

#if !USE_Simulator
#include "flash_handler.h"
#include "sys.h"
#endif

#include "cJSON.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define LOG_TAG "app_settings"
#include "elog.h"

#if !USE_Simulator
#define APP_SETTINGS_CONFIG_PATH SYS_STORAGE_MOUNT_POINT SYS_CONFIG_DIR
#else
#define APP_SETTINGS_CONFIG_PATH                                               \
  "C:\\Users\\12114\\Desktop\\temp\\app_settings\\"
#endif

#define MAX_SETTINGS_BUFFER_SIZE 1024 // 文件系统操作缓冲
static char settings_buffer[MAX_SETTINGS_BUFFER_SIZE];

/**
 * @brief 从文件系统加载应用配置
 *
 * @param app_name 应用名称
 * @param config_file_name 配置文件
 * @return int 0成功，-1失败
 */
int app_settings_load(const char *app_name, const char *config_file_name) {
  app_def_t *app_def = (app_def_t *)app_manager_find_by_name(app_name);
  if (!app_def) {
    log_e("app not found when load settings");
    return -1;
  }
#if !USE_Simulator
  memset(settings_buffer, 0, MAX_SETTINGS_BUFFER_SIZE);

  char path[256] = {0};
  snprintf(path, sizeof(path), APP_SETTINGS_CONFIG_PATH "%s", config_file_name);

  int res =
      flash_handler_read(path, 0, settings_buffer, sizeof(settings_buffer) - 1);
  if (res <= 0) {
    log_w("%s settings load failed from %s", app_name, path);
    return -1;
  }

  log_d("%s settings load success from %s", app_name, path);
#else
  char full_path[512];
  snprintf(full_path, sizeof(full_path), APP_SETTINGS_CONFIG_PATH "%s.json",
           config_file_name);
  FILE *fp = fopen(full_path, "r");
  if (!fp) {
    log_w("%s settings load failed from %s", app_name, full_path);
    return -1;
  }
  memset(settings_buffer, 0, MAX_SETTINGS_BUFFER_SIZE);
  fread(settings_buffer, 1, sizeof(settings_buffer) - 1, fp);
  fclose(fp);
  log_d("%s settings load success from %s", app_name, full_path);
#endif
  // 解析JSON数据
  cJSON *root = cJSON_Parse(settings_buffer);
  if (!root) {
    log_e("Failed to parse JSON for %s", app_name);
    return -1;
  }

  // 检查hash值是否与应用注册时计算的值相同
  cJSON *hash_item = cJSON_GetObjectItem(root, "hash");
  if (hash_item && cJSON_IsNumber(hash_item)) {
    uint32_t loaded_hash = (uint32_t)hash_item->valuedouble;
    if (loaded_hash != app_def->settings->hash) {
      log_e("Hash mismatch for %s: loaded=%u, expected=%u", app_name,
            loaded_hash, app_def->settings->hash);
      cJSON_Delete(root);
      return -1;
    }
    log_d("Hash verified for %s: %u", app_name, loaded_hash);
  } else {
    log_e("Hash not found or invalid in settings file for %s", app_name);
    cJSON_Delete(root);
    return -1;
  }

  // 读取配置数组
  cJSON *configs_array = cJSON_GetObjectItem(root, "configs");
  if (configs_array && cJSON_IsArray(configs_array)) {
    int config_count = cJSON_GetArraySize(configs_array);

    // 释放旧的配置内存
    if (app_def->settings->configs) {
      // 释放字符串类型的配置项所占的内存
      for (int i = 0; i < app_def->settings->count; i++) {
        if (app_def->settings->configs[i].type == APP_CONFIG_TYPE_STRING &&
            app_def->settings->configs[i].string) {
          sys_free(SYS_MEM_INTERNAL,
                   (void *)app_def->settings->configs[i].string);
        }
      }
      sys_free(SYS_MEM_INTERNAL, app_def->settings->configs);
    }

    // 分配新的配置内存
    app_config_t *configs = (app_config_t *)sys_malloc(
        SYS_MEM_INTERNAL, config_count * sizeof(app_config_t));
    if (!configs) {
      log_e("Failed to allocate memory for configs");
      cJSON_Delete(root);
      return -1;
    }
    memset(configs, 0, config_count * sizeof(app_config_t));

    // 解析每个配置项
    for (int i = 0; i < config_count; i++) {
      cJSON *config_item = cJSON_GetArrayItem(configs_array, i);
      if (!config_item)
        continue;

      cJSON *key_item = cJSON_GetObjectItem(config_item, "key");
      cJSON *type_item = cJSON_GetObjectItem(config_item, "type");
      cJSON *value_item = cJSON_GetObjectItem(config_item, "value");

      if (key_item && cJSON_IsNumber(key_item) && type_item &&
          cJSON_IsNumber(type_item) && value_item) {
        configs[i].key = (uint16_t)key_item->valueint;
        configs[i].type = (uint8_t)type_item->valueint;

        // 根据类型解析值
        switch (configs[i].type) {
        case APP_CONFIG_TYPE_INT:
          if (cJSON_IsNumber(value_item)) {
            configs[i].Int = (uint32_t)value_item->valueint;
          }
          break;
        case APP_CONFIG_TYPE_STRING:
          if (cJSON_IsString(value_item)) {
            // 为字符串分配内存并复制
            size_t str_len = strlen(value_item->valuestring) + 1;
            configs[i].string = (char *)sys_malloc(SYS_MEM_INTERNAL, str_len);
            if (configs[i].string) {
              strcpy(configs[i].string, value_item->valuestring);
            } else {
              log_e("Failed to allocate memory for string config");
            }
          }
          break;
        case APP_CONFIG_TYPE_BOOL:
          if (cJSON_IsBool(value_item)) {
            configs[i].Bool = cJSON_IsTrue(value_item);
          } else if (cJSON_IsNumber(value_item)) {
            configs[i].Bool = (bool)value_item->valueint != 0;
          }
          break;
        case APP_CONFIG_TYPE_DOUBLE:
          if (cJSON_IsNumber(value_item)) {
            configs[i].Double = value_item->valuedouble;
          }
          break;
        default:
          log_w("Unknown config type: %d", configs[i].type);
          break;
        }
      }
    }

    app_def->settings->configs = configs;
    app_def->settings->count = config_count;
  }

  cJSON_Delete(root);
  return 0;
}

/**
 * @brief 保存应用配置到文件系统
 *
 * @param app_name 应用名称
 * @param config_file_name 配置文件
 * @return int 0成功，-1失败
 */
int app_settings_save(const char *app_name, const char *config_file_name) {
  cJSON *root = cJSON_CreateObject(); // json根对象
  app_def_t *app_def = app_manager_find_by_name(app_name);
  if (!app_def) {
    log_e("app not found when save settings");
    cJSON_Delete(root);
    return -1;
  }

  // 添加hash值
  cJSON_AddNumberToObject(root, "hash", app_def->settings->hash);

  // 添加配置数组
  cJSON *configs_array = cJSON_CreateArray();
  for (int i = 0; i < app_def->settings->count; i++) {
    cJSON *config_item = cJSON_CreateObject();
    cJSON_AddNumberToObject(config_item, "key",
                            (double)app_def->settings->configs[i].key);
    cJSON_AddNumberToObject(config_item, "type",
                            (double)app_def->settings->configs[i].type);

    // 根据类型添加值
    switch (app_def->settings->configs[i].type) {
    case APP_CONFIG_TYPE_INT:
      cJSON_AddNumberToObject(config_item, "value",
                              (double)app_def->settings->configs[i].Int);
      break;
    case APP_CONFIG_TYPE_STRING:
      if (app_def->settings->configs[i].string) {
        cJSON_AddStringToObject(config_item, "value",
                                app_def->settings->configs[i].string);
      } else {
        cJSON_AddNullToObject(config_item, "value");
      }
      break;
    case APP_CONFIG_TYPE_BOOL:
      cJSON_AddBoolToObject(config_item, "value",
                            app_def->settings->configs[i].Bool);
      break;
    case APP_CONFIG_TYPE_DOUBLE:
      cJSON_AddNumberToObject(config_item, "value",
                              app_def->settings->configs[i].Double);
      break;
    default:
      log_w("Unknown config type: %d", app_def->settings->configs[i].type);
      cJSON_AddNullToObject(config_item, "value");
      break;
    }

    cJSON_AddItemToArray(configs_array, config_item);
  }

  cJSON_AddItemToObject(root, "configs", configs_array);

  // 转换为字符串
  char *json_string = cJSON_Print(root);
  if (!json_string) {
    log_e("Failed to convert settings to JSON for %s", app_name);
    cJSON_Delete(root);
    return -1;
  }

#if !USE_Simulator
  // 创建路径
  static char path[256] = {0};
  snprintf(path, sizeof(path), APP_SETTINGS_CONFIG_PATH "%s", config_file_name);

  // 写入文件
  int res = flash_handler_write(path, 0, json_string, strlen(json_string));
  if (res < 0) {
    log_e("%s settings save failed to %s", app_name, path);
    cJSON_free(json_string);
    cJSON_Delete(root);
    return -1;
  }
#else
  char full_path[512];
  snprintf(full_path, sizeof(full_path), APP_SETTINGS_CONFIG_PATH "%s.json",
           config_file_name);
  FILE *fp = fopen(full_path, "w");
  if (!fp) {
    log_e("%s settings save failed to %s", app_name, full_path);
    cJSON_free(json_string);
    cJSON_Delete(root);
    return -1;
  }
  if (fputs(json_string, fp) < 0) {
    log_e("%s settings write failed", app_name);
    fclose(fp);
    cJSON_free(json_string);
    cJSON_Delete(root);
    return -1;
  }
  fclose(fp);
#endif

#if !USE_Simulator
  log_i("%s settings save success to %s", app_name, path);
#else
  log_i("%s settings save success to %s", app_name, full_path);
#endif
  cJSON_free(json_string);
  cJSON_Delete(root);
  app_def->settings->attr.is_dirty = 0;
  return 0;
}

/**
 * @brief 更新应用配置
 *
 * @param app_name 应用名称
 * @param settings 配置
 * @return int 0成功，-1失败
 */
int app_settings_update(const char *app_name, const app_settings_t *settings) {
  app_def_t *app_def = app_manager_find_by_name(app_name);
  if (!app_def || !settings) {
    log_e("Invalid app or settings for update");
    return -1;
  }

  app_config_t *new_configs = NULL;
  uint8_t new_count = 0;

  // 1. 先准备新的配置内存并复制数据（避免自赋值引起的即放即读）
  if (settings->configs && settings->count > 0) {
    new_configs = (app_config_t *)sys_malloc(
        SYS_MEM_INTERNAL, settings->count * sizeof(app_config_t));
    if (!new_configs) {
      log_e("Failed to allocate memory for new configs");
      return -1;
    }
    memset(new_configs, 0, settings->count * sizeof(app_config_t));
    new_count = settings->count;

    for (int i = 0; i < settings->count; i++) {
      new_configs[i].key = settings->configs[i].key;
      new_configs[i].type = settings->configs[i].type;

      switch (settings->configs[i].type) {
      case APP_CONFIG_TYPE_INT:
        new_configs[i].Int = settings->configs[i].Int;
        break;
      case APP_CONFIG_TYPE_BOOL:
        new_configs[i].Bool = settings->configs[i].Bool;
        break;
      case APP_CONFIG_TYPE_DOUBLE:
        new_configs[i].Double = settings->configs[i].Double;
        break;
      case APP_CONFIG_TYPE_STRING:
        if (settings->configs[i].string) {
          size_t str_len = strlen(settings->configs[i].string) + 1;
          new_configs[i].string = (char *)sys_malloc(SYS_MEM_INTERNAL, str_len);
          if (new_configs[i].string) {
            strcpy(new_configs[i].string, settings->configs[i].string);
          }
        }
        break;
      default:
        break;
      }
    }
  }

  // 2. 释放旧配置
  if (app_def->settings->configs) {
    for (int i = 0; i < app_def->settings->count; i++) {
      if (app_def->settings->configs[i].type == APP_CONFIG_TYPE_STRING &&
          app_def->settings->configs[i].string) {
        sys_free(SYS_MEM_INTERNAL,
                 (void *)app_def->settings->configs[i].string);
      }
    }
    sys_free(SYS_MEM_INTERNAL, app_def->settings->configs);
  }

  // 3. 更新指针
  app_def->settings->configs = new_configs;
  app_def->settings->count = new_count;
  app_def->settings->attr.is_dirty = 1;

  log_d("Updated settings for %s", app_name);
  return 0;
}
