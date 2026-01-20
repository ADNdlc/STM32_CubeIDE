#include "sys_config.h"
#include "project_cfg.h"
#include <stdio.h>
#include <string.h>

#if !USE_Simulator
#include "cJSON.h"
#include "flash_handler.h"
#include "sys.h"
#endif

#define SYS_CONFIG_MEM_SOURCE SYS_MEM_INTERNAL
#define LOG_TAG "SYS_CFG"
#include "elog.h"

// 组合系统配置文件完整路径
#define SYS_CONFIG_FILE_PATH                                                   \
  SYS_STORAGE_MOUNT_POINT SYS_CONFIG_DIR "system.json"

static app_settings_t sys_config = {
    .configs = NULL,
    .hash = 0,
    .count = SYS_CONFIG_KEY_MAX,
    .attr = {0, 0},
};

// 默认wifi配置
#define DEFAULT_WIFI_SSID "test1"
#define DEFAULT_WIFI_PASSWORD "yu12345678"
// 默认云平台配置
#define DEFAULT_CLOUD_PLATFORM CLOUD_PLATFORM_ONENET
#define DEFAULT_CLOUD_PRODUCT_ID "SQKg9n0Ii0"
#define DEFAULT_CLOUD_DEVICE_ID "test2"
#define DEFAULT_CLOUD_DEVICE_SECRET                                            \
  "version=2018-10-31&res=products%2FSQKg9n0Ii0%2Fdevices%2Ftest2&et="         \
  "1855499668539&method=md5&sign=%2FHVmg4Xz2RfTRWEu44mApQ%3D%3D"

/**
 * @brief 将g_sys_config配置为默认值
 *
 */
void sys_config_set_defaults(void) {
  log_i("Setting system configuration to defaults...");
  // wifi ssid
  sys_config.configs[WIFI_SSID].key = WIFI_SSID;
  sys_config.configs[WIFI_SSID].type = APP_CONFIG_TYPE_STRING;
  sys_config.configs[WIFI_SSID].string =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_WIFI_SSID));
  memcpy(sys_config.configs[WIFI_SSID].string, DEFAULT_WIFI_SSID,
         sizeof(DEFAULT_WIFI_SSID));
  // wifi password
  sys_config.configs[WIFI_PASSWORD].key = WIFI_PASSWORD;
  sys_config.configs[WIFI_PASSWORD].type = APP_CONFIG_TYPE_STRING;
  sys_config.configs[WIFI_PASSWORD].string =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_WIFI_PASSWORD));
  memcpy(sys_config.configs[WIFI_PASSWORD].string, DEFAULT_WIFI_PASSWORD,
         sizeof(DEFAULT_WIFI_PASSWORD));
  // cloud platform
  sys_config.configs[CLOUD_PLATFORM].key = CLOUD_PLATFORM;
  sys_config.configs[CLOUD_PLATFORM].type = APP_CONFIG_TYPE_INT;
  sys_config.configs[CLOUD_PLATFORM].Int = DEFAULT_CLOUD_PLATFORM;
  // cloud product id
  sys_config.configs[CLOUD_PRODUCT_ID].key = CLOUD_PRODUCT_ID;
  sys_config.configs[CLOUD_PRODUCT_ID].type = APP_CONFIG_TYPE_STRING;
  sys_config.configs[CLOUD_PRODUCT_ID].string =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_CLOUD_PRODUCT_ID));
  memcpy(sys_config.configs[CLOUD_PRODUCT_ID].string, DEFAULT_CLOUD_PRODUCT_ID,
         sizeof(DEFAULT_CLOUD_PRODUCT_ID));
  // cloud device id
  sys_config.configs[CLOUD_DEVICE_ID].key = CLOUD_DEVICE_ID;
  sys_config.configs[CLOUD_DEVICE_ID].type = APP_CONFIG_TYPE_STRING;
  sys_config.configs[CLOUD_DEVICE_ID].string =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_CLOUD_DEVICE_ID));
  memcpy(sys_config.configs[CLOUD_DEVICE_ID].string, DEFAULT_CLOUD_DEVICE_ID,
         sizeof(DEFAULT_CLOUD_DEVICE_ID));
  // cloud device secret
  sys_config.configs[CLOUD_DEVICE_SECRET].key = CLOUD_DEVICE_SECRET;
  sys_config.configs[CLOUD_DEVICE_SECRET].type = APP_CONFIG_TYPE_STRING;
  sys_config.configs[CLOUD_DEVICE_SECRET].string =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_CLOUD_DEVICE_SECRET));
  memcpy(sys_config.configs[CLOUD_DEVICE_SECRET].string,
         DEFAULT_CLOUD_DEVICE_SECRET, sizeof(DEFAULT_CLOUD_DEVICE_SECRET));
  sys_config.attr.is_loaded = 1;
}

int sys_config_init(void) {
  log_i("Loading system configuration...");
  sys_config.configs = sys_malloc(SYS_CONFIG_MEM_SOURCE,
                                  sizeof(app_config_t) * SYS_CONFIG_KEY_MAX);
  if (sys_config.configs == NULL) {
    log_e("Failed to allocate memory for sys_config");
    return -1;
  }
  sys_config_set_defaults();

#if !USE_Simulator
  // 尝试从文件系统加载
  app_settings_t *settings = &sys_config;
  char *buffer = sys_malloc(SYS_CONFIG_MEM_SOURCE, 1024);
  if (buffer) {
    int res =
        flash_handler_read(SYS_CONFIG_FILE_PATH, 0, (uint8_t *)buffer, 1023);
    if (res > 0) {
      buffer[res] = '\0';
      log_i("Loaded system config from %s", SYS_CONFIG_FILE_PATH);
      // TODO: 解析 JSON 并更新 sys_config.configs
      // 这里简化处理，实际应解析 cJSON
    }
    sys_free(SYS_CONFIG_MEM_SOURCE, buffer);
  }
#endif

  return 0;
}

int sys_config_save(void) {
  log_i("Saving system configuration to %s...", SYS_CONFIG_FILE_PATH);
#if !USE_Simulator
  // TODO: 将 sys_config.configs 序列化为 JSON
  // 这里简化演示写入逻辑
  const char *test_json = "{\"wifi_ssid\":\"test\"}";
  return flash_handler_write(SYS_CONFIG_FILE_PATH, 0,
                             (const uint8_t *)test_json, strlen(test_json));
#else
  return 0;
#endif
}

/**
 * @brief 获取系统配置句柄
 *
 * @return app_settings_t*
 */
app_settings_t *sys_config_get(void) {
  if (!sys_config.attr.is_loaded) {
    return NULL;
  }
  return &sys_config;
}

char *sys_config_get_wifi_ssid(void) {
  if (!sys_config.attr.is_loaded) {
    return NULL;
  }
  return sys_config.configs[WIFI_SSID].string;
}
char *sys_config_get_wifi_password(void) {
  if (!sys_config.attr.is_loaded) {
    return NULL;
  }
  return sys_config.configs[WIFI_PASSWORD].string;
}

void sys_config_set_wifi_ssid(const char *ssid) {
  if (!sys_config.attr.is_loaded || !ssid) {
    return;
  }
  if (sys_config.configs[WIFI_SSID].string) {
    sys_free(SYS_CONFIG_MEM_SOURCE, sys_config.configs[WIFI_SSID].string);
  }
  size_t len = strlen(ssid) + 1;
  sys_config.configs[WIFI_SSID].string = sys_malloc(SYS_CONFIG_MEM_SOURCE, len);
  if (sys_config.configs[WIFI_SSID].string) {
    memcpy(sys_config.configs[WIFI_SSID].string, ssid, len);
  }
}
void sys_config_set_wifi_password(const char *password) {
  if (!sys_config.attr.is_loaded || !password) {
    return;
  }
  if (sys_config.configs[WIFI_PASSWORD].string) {
    sys_free(SYS_CONFIG_MEM_SOURCE, sys_config.configs[WIFI_PASSWORD].string);
  }
  size_t len = strlen(password) + 1;
  sys_config.configs[WIFI_PASSWORD].string =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, len);
  if (sys_config.configs[WIFI_PASSWORD].string) {
    memcpy(sys_config.configs[WIFI_PASSWORD].string, password, len);
  }
}

int sys_config_get_cloud_platform(void) {
  if (!sys_config.attr.is_loaded) {
    return -1;
  }
  return sys_config.configs[CLOUD_PLATFORM].Int;
}
char *sys_config_get_cloud_product_id(void) {
  if (!sys_config.attr.is_loaded) {
    return NULL;
  }
  return sys_config.configs[CLOUD_PRODUCT_ID].string;
}
char *sys_config_get_cloud_device_id(void) {
  if (!sys_config.attr.is_loaded) {
    return NULL;
  }
  return sys_config.configs[CLOUD_DEVICE_ID].string;
}
char *sys_config_get_cloud_device_secret(void) {
  if (!sys_config.attr.is_loaded) {
    return NULL;
  }
  return sys_config.configs[CLOUD_DEVICE_SECRET].string;
}
