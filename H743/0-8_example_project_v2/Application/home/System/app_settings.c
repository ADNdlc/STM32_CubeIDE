#include "app_settings.h"
#include "app_manager.h"
#include "project_cfg.h"

#if !USE_Simulator
#include "vfs_manager.h"
#include "Sys.h"
#endif

#include "cJSON.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "APP_SET"
#include "elog.h"

// 【修改】路径宏优化，确保中间有 '/' 连接
#if !USE_Simulator
#define APP_SETTINGS_CONFIG_PATH "/sys/config/"  // 根据实际 VFS 挂载点调整
#else
#define APP_SETTINGS_CONFIG_PATH "C:\\Users\\12114\\Desktop\\temp\\app_settings\\"
#endif

// 内部辅助释放配置内存函数
static void free_configs(app_config_t *configs, uint8_t count) {
  if (!configs) return;
  for (int i = 0; i < count; i++) {
    if (configs[i].type == APP_CONFIG_TYPE_STRING && configs[i].s_val) {
      sys_free(SYS_MEM_INTERNAL, (void *)configs[i].s_val);
    }
  }
  sys_free(SYS_MEM_INTERNAL, configs);
}

/**
 * @brief 从文件系统加载应用配置
 */
int app_settings_load(const char *app_name, const char *config_file_name) {
  app_def_t *app_def = (app_def_t *)app_manager_find_by_name(app_name);
  if (!app_def || !app_def->settings) {
    log_e("App or app settings struct not found for %s", app_name);
    return -1;
  }

  char path[256];
  snprintf(path, sizeof(path), "%s%s.json", APP_SETTINGS_CONFIG_PATH,
           config_file_name);

  char *json_buffer = NULL;

#if !USE_Simulator
  // 使用 VFS 动态读取机制
  vfs_stat_t st;
  if (vfs_stat(path, &st) != VFS_OK || st.size == 0) {
    log_w("Settings file %s not found or empty", path);
    return -1;
  }

  json_buffer = sys_malloc(SYS_MEM_INTERNAL, st.size + 1);
  if (!json_buffer) {
    log_e("OOM allocating %lu bytes for JSON read", st.size);
    return -1;
  }

  int fd = vfs_open(path, VFS_O_RDONLY);
  if (fd < 0) {
    sys_free(SYS_MEM_INTERNAL, json_buffer);
    return -1;
  }

  int read_len = vfs_read(fd, json_buffer, st.size);
  vfs_close(fd);

  if (read_len != st.size) {
    log_e("Failed to read full settings file");
    sys_free(SYS_MEM_INTERNAL, json_buffer);
    return -1;
  }
  json_buffer[st.size] = '\0'; // 确保字符串结束
#else
  FILE *fp = fopen(path, "rb");
  if (!fp)
    return -1;
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  json_buffer = sys_malloc(SYS_MEM_INTERNAL, size + 1);
  if (!json_buffer) {
    fclose(fp);
    return -1;
  }
  fread(json_buffer, 1, size, fp);
  json_buffer[size] = '\0';
  fclose(fp);
#endif

  // 解析JSON数据
  cJSON *root = cJSON_Parse(json_buffer);
  sys_free(SYS_MEM_INTERNAL, json_buffer); // 解析完立即释放字符串内存

  if (!root) {
    log_e("Failed to parse JSON for %s", app_name);
    return -1;
  }

  // 检查 hash
  cJSON *hash_item = cJSON_GetObjectItem(root, "hash");
  if (!hash_item || !cJSON_IsNumber(hash_item) ||
      (uint32_t)hash_item->valuedouble != app_def->settings->hash) {
    log_e("Hash invalid or mismatch for %s", app_name);
    cJSON_Delete(root);
    return -1;
  }

  // 读取配置数组
  cJSON *configs_array = cJSON_GetObjectItem(root, "configs");
  if (!configs_array || !cJSON_IsArray(configs_array)) {
    cJSON_Delete(root);
    return -1;
  }

  int config_count = cJSON_GetArraySize(configs_array);
  if (config_count == 0) {
    cJSON_Delete(root);
    return 0; // 空配置
  }

  // 分配临时数组
  app_config_t *new_configs = (app_config_t *)sys_malloc(
      SYS_MEM_INTERNAL, config_count * sizeof(app_config_t));
  if (!new_configs) {
    cJSON_Delete(root);
    return -1;
  }
  memset(new_configs, 0, config_count * sizeof(app_config_t));

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
      new_configs[i].key = (uint16_t)key_item->valueint;
      new_configs[i].type = (uint8_t)type_item->valueint;

      switch (new_configs[i].type) {
      case APP_CONFIG_TYPE_INT:
        new_configs[i].i_val = (int32_t)value_item->valuedouble;
        break;
      case APP_CONFIG_TYPE_STRING:
        if (cJSON_IsString(value_item)) {
          size_t str_len = strlen(value_item->valuestring) + 1;
          new_configs[i].s_val = (char *)sys_malloc(SYS_MEM_INTERNAL, str_len);
          if (new_configs[i].s_val) {
            strcpy(new_configs[i].s_val, value_item->valuestring);
          }
        }
        break;
      case APP_CONFIG_TYPE_BOOL:
        new_configs[i].b_val = cJSON_IsTrue(value_item) ||
                               (cJSON_IsNumber(value_item) &&
                                value_item->valueint != 0);
        break;
      case APP_CONFIG_TYPE_DOUBLE:
        new_configs[i].d_val = value_item->valuedouble;
        break;
      }
    }
  }

  // 释放旧内存并挂载新内存 (事务操作)
  free_configs(app_def->settings->configs, app_def->settings->count);
  app_def->settings->configs = new_configs;
  app_def->settings->count = (uint8_t)config_count;
  app_def->settings->attr.is_loaded = 1;
  app_def->settings->attr.is_dirty = 0;

  cJSON_Delete(root);
  log_i("Loaded %d configs for %s", config_count, app_name);
  return 0;
}

/**
 * @brief 保存应用配置到文件系统
 */
int app_settings_save(const char *app_name, const char *config_file_name) {
  app_def_t *app_def = (app_def_t *)app_manager_find_by_name(app_name);
  if (!app_def || !app_def->settings) return -1;

  cJSON *root = cJSON_CreateObject();

  cJSON_AddNumberToObject(root, "hash", app_def->settings->hash);

  cJSON *configs_array = cJSON_CreateArray();
  for (int i = 0; i < app_def->settings->count; i++) {
    cJSON *config_item = cJSON_CreateObject();
    cJSON_AddNumberToObject(config_item, "key", app_def->settings->configs[i].key);
    cJSON_AddNumberToObject(config_item, "type", app_def->settings->configs[i].type);

    switch (app_def->settings->configs[i].type) {
    case APP_CONFIG_TYPE_INT:
      cJSON_AddNumberToObject(config_item, "value", app_def->settings->configs[i].i_val);
      break;
    case APP_CONFIG_TYPE_STRING:
      if (app_def->settings->configs[i].s_val) {
        cJSON_AddStringToObject(config_item, "value", app_def->settings->configs[i].s_val);
      } else {
        cJSON_AddNullToObject(config_item, "value");
      }
      break;
    case APP_CONFIG_TYPE_BOOL:
      cJSON_AddBoolToObject(config_item, "value", app_def->settings->configs[i].b_val);
      break;
    case APP_CONFIG_TYPE_DOUBLE:
      cJSON_AddNumberToObject(config_item, "value", app_def->settings->configs[i].d_val);
      break;
    default:
      cJSON_AddNullToObject(config_item, "value");
      break;
    }
    cJSON_AddItemToArray(configs_array, config_item);
  }
  cJSON_AddItemToObject(root, "configs", configs_array);

  char *json_string = cJSON_PrintUnformatted(root); // 【修改】改用无格式化，节省超多存储空间
  cJSON_Delete(root);

  if (!json_string) return -1;

  char path[256];
  snprintf(path, sizeof(path), "%s%s.json", APP_SETTINGS_CONFIG_PATH, config_file_name);

#if !USE_Simulator
  // 【修改】使用 VFS 写入，并带上 VFS_O_TRUNC，确保不会残留旧文件内容
  int fd = vfs_open(path, VFS_O_WRONLY | VFS_O_CREAT | VFS_O_TRUNC);
  if (fd < 0) {
    log_e("Failed to open %s for write", path);
    cJSON_free(json_string);
    return -1;
  }

  int res = vfs_write(fd, json_string, strlen(json_string));
  vfs_close(fd);

  if (res < 0) {
    log_e("Failed to write json to %s", path);
    cJSON_free(json_string);
    return -1;
  }
#else
  FILE *fp = fopen(path, "w");
  if (!fp) {
    cJSON_free(json_string);
    return -1;
  }
  fputs(json_string, fp);
  fclose(fp);
#endif

  log_i("Settings saved to %s", path);
  cJSON_free(json_string);
  app_def->settings->attr.is_dirty = 0;
  return 0;
}

/**
 * @brief 更新应用配置
 */
int app_settings_update(const char *app_name, const app_settings_t *settings) {
  app_def_t *app_def = (app_def_t *)app_manager_find_by_name(app_name);
  if (!app_def || !app_def->settings || !settings) return -1;

  app_config_t *new_configs = NULL;
  uint8_t new_count = 0;

  if (settings->configs && settings->count > 0) {
    new_configs = (app_config_t *)sys_malloc(SYS_MEM_INTERNAL, settings->count * sizeof(app_config_t));
    if (!new_configs) return -1;

    memset(new_configs, 0, settings->count * sizeof(app_config_t));
    new_count = settings->count;

    for (int i = 0; i < settings->count; i++) {
      new_configs[i].key = settings->configs[i].key;
      new_configs[i].type = settings->configs[i].type;

      switch (settings->configs[i].type) {
      case APP_CONFIG_TYPE_INT:
        new_configs[i].i_val = settings->configs[i].i_val;
        break;
      case APP_CONFIG_TYPE_BOOL:
        new_configs[i].b_val = settings->configs[i].b_val;
        break;
      case APP_CONFIG_TYPE_DOUBLE:
        new_configs[i].d_val = settings->configs[i].d_val;
        break;
      case APP_CONFIG_TYPE_STRING:
        if (settings->configs[i].s_val) {
          size_t str_len = strlen(settings->configs[i].s_val) + 1;
          new_configs[i].s_val = (char *)sys_malloc(SYS_MEM_INTERNAL, str_len);
          if (new_configs[i].s_val) {
            strcpy(new_configs[i].s_val, settings->configs[i].s_val);
          }
        }
        break;
      default:
        break;
      }
    }
  }

  // 使用辅助函数统一释放
  free_configs(app_def->settings->configs, app_def->settings->count);

  app_def->settings->configs = new_configs;
  app_def->settings->count = new_count;
  app_def->settings->attr.is_dirty = 1;

  log_d("Updated RAM settings for %s", app_name);
  return 0;
}
