#include "sys_config.h"
#include "project_cfg.h"
#include <stdio.h>
#include <string.h>

#if !USE_Simulator
#include "Sys.h"
#include "cJSON.h"
#include "vfs_manager.h"

#endif
#include "app_manager.h"

#define SYS_CONFIG_MEM_SOURCE SYS_MEM_INTERNAL
#define LOG_TAG "SYS_CFG"
#include "elog.h"

// 组合系统配置文件完整路径
#define SYS_CONFIG_FILE_PATH                                                   \
  SYS_STORAGE_MOUNT_POINT SYS_CONFIG_DIR "system.json"

static app_settings_t sys_config_settings = {
    .configs = NULL,
    .hash = 0x1234, // 随便给个固定的 hash 用于验证
    .count = SYS_CONFIG_KEY_MAX,
    .attr = {0, 0},
};

static app_def_t sys_app_def = {
    .name = "system",
    .settings = &sys_config_settings,
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
  sys_config_settings.configs[WIFI_SSID].key = WIFI_SSID;
  sys_config_settings.configs[WIFI_SSID].type = APP_CONFIG_TYPE_STRING;
  sys_config_settings.configs[WIFI_SSID].s_val =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_WIFI_SSID));
  memcpy(sys_config_settings.configs[WIFI_SSID].s_val, DEFAULT_WIFI_SSID,
         sizeof(DEFAULT_WIFI_SSID));
  // wifi password
  sys_config_settings.configs[WIFI_PASSWORD].key = WIFI_PASSWORD;
  sys_config_settings.configs[WIFI_PASSWORD].type = APP_CONFIG_TYPE_STRING;
  sys_config_settings.configs[WIFI_PASSWORD].s_val =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_WIFI_PASSWORD));
  memcpy(sys_config_settings.configs[WIFI_PASSWORD].s_val,
         DEFAULT_WIFI_PASSWORD, sizeof(DEFAULT_WIFI_PASSWORD));
  // cloud platform
  sys_config_settings.configs[CLOUD_PLATFORM].key = CLOUD_PLATFORM;
  sys_config_settings.configs[CLOUD_PLATFORM].type = APP_CONFIG_TYPE_INT;
  sys_config_settings.configs[CLOUD_PLATFORM].i_val = DEFAULT_CLOUD_PLATFORM;
  // cloud product id
  sys_config_settings.configs[CLOUD_PRODUCT_ID].key = CLOUD_PRODUCT_ID;
  sys_config_settings.configs[CLOUD_PRODUCT_ID].type = APP_CONFIG_TYPE_STRING;
  sys_config_settings.configs[CLOUD_PRODUCT_ID].s_val =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_CLOUD_PRODUCT_ID));
  memcpy(sys_config_settings.configs[CLOUD_PRODUCT_ID].s_val,
         DEFAULT_CLOUD_PRODUCT_ID, sizeof(DEFAULT_CLOUD_PRODUCT_ID));
  // cloud device id
  sys_config_settings.configs[CLOUD_DEVICE_ID].key = CLOUD_DEVICE_ID;
  sys_config_settings.configs[CLOUD_DEVICE_ID].type = APP_CONFIG_TYPE_STRING;
  sys_config_settings.configs[CLOUD_DEVICE_ID].s_val =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_CLOUD_DEVICE_ID));
  memcpy(sys_config_settings.configs[CLOUD_DEVICE_ID].s_val,
         DEFAULT_CLOUD_DEVICE_ID, sizeof(DEFAULT_CLOUD_DEVICE_ID));
  // cloud device secret
  sys_config_settings.configs[CLOUD_DEVICE_SECRET].key = CLOUD_DEVICE_SECRET;
  sys_config_settings.configs[CLOUD_DEVICE_SECRET].type =
      APP_CONFIG_TYPE_STRING;
  sys_config_settings.configs[CLOUD_DEVICE_SECRET].s_val =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_CLOUD_DEVICE_SECRET));
  memcpy(sys_config_settings.configs[CLOUD_DEVICE_SECRET].s_val,
         DEFAULT_CLOUD_DEVICE_SECRET, sizeof(DEFAULT_CLOUD_DEVICE_SECRET));
  sys_config_settings.attr.is_loaded = 1;
}

int sys_config_init(void) {
  log_i("Initializing system configuration...");

  // 1. 确保目录存在
  vfs_mkdir("/sys/config");

  // 2. 分配基础配置内存并加载默认值
  sys_config_settings.configs = sys_malloc(
      SYS_CONFIG_MEM_SOURCE, sizeof(app_config_t) * SYS_CONFIG_KEY_MAX);
  if (sys_config_settings.configs == NULL) {
    log_e("Failed to allocate memory for sys_config");
    return -1;
  }
  sys_config_set_defaults();

  // 3. 注册 dummy app 到管理器，内部会自动尝试加载，加载失败时会自动保存这些默认值
  app_manager_register(&sys_app_def, 0);

  if (sys_config_settings.attr.is_loaded) {
    log_i("System config loaded/created successfully (handled by app_manager).");
  } else {
    log_w("System config initialization issue.");
  }

  return 0;
}

int sys_config_save(void) {
  log_i("Saving system configuration...");
  return app_settings_save("system", "system");
}

/**
 * @brief 获取系统配置句柄
 *
 * @return app_settings_t*
 */
app_settings_t *sys_config_get(void) {
  if (!sys_config_settings.attr.is_loaded) {
    return NULL;
  }
  return &sys_config_settings;
}

char *sys_config_get_wifi_ssid(void) {
  if (!sys_config_settings.attr.is_loaded) {
    return NULL;
  }
  return sys_config_settings.configs[WIFI_SSID].s_val;
}
char *sys_config_get_wifi_password(void) {
  if (!sys_config_settings.attr.is_loaded) {
    return NULL;
  }
  return sys_config_settings.configs[WIFI_PASSWORD].s_val;
}

void sys_config_set_wifi_ssid(const char *ssid) {
  if (!sys_config_settings.attr.is_loaded || !ssid) {
    return;
  }
  if (sys_config_settings.configs[WIFI_SSID].s_val) {
    sys_free(SYS_CONFIG_MEM_SOURCE,
             sys_config_settings.configs[WIFI_SSID].s_val);
  }
  size_t len = strlen(ssid) + 1;
  sys_config_settings.configs[WIFI_SSID].s_val =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, len);
  if (sys_config_settings.configs[WIFI_SSID].s_val) {
    memcpy(sys_config_settings.configs[WIFI_SSID].s_val, ssid, len);
  }
}
void sys_config_set_wifi_password(const char *password) {
  if (!sys_config_settings.attr.is_loaded || !password) {
    return;
  }
  if (sys_config_settings.configs[WIFI_PASSWORD].s_val) {
    sys_free(SYS_CONFIG_MEM_SOURCE,
             sys_config_settings.configs[WIFI_PASSWORD].s_val);
  }
  size_t len = strlen(password) + 1;
  sys_config_settings.configs[WIFI_PASSWORD].s_val =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, len);
  if (sys_config_settings.configs[WIFI_PASSWORD].s_val) {
    memcpy(sys_config_settings.configs[WIFI_PASSWORD].s_val, password, len);
  }
}

int sys_config_get_cloud_platform(void) {
  if (!sys_config_settings.attr.is_loaded) {
    return -1;
  }
  return sys_config_settings.configs[CLOUD_PLATFORM].i_val;
}
char *sys_config_get_cloud_product_id(void) {
  if (!sys_config_settings.attr.is_loaded) {
    return NULL;
  }
  return sys_config_settings.configs[CLOUD_PRODUCT_ID].s_val;
}
char *sys_config_get_cloud_device_id(void) {
  if (!sys_config_settings.attr.is_loaded) {
    return NULL;
  }
  return sys_config_settings.configs[CLOUD_DEVICE_ID].s_val;
}
char *sys_config_get_cloud_device_secret(void) {
  if (!sys_config_settings.attr.is_loaded) {
    return NULL;
  }
  return sys_config_settings.configs[CLOUD_DEVICE_SECRET].s_val;
}
