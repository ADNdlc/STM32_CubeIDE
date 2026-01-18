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
  sys_config.configs[WIFI_SSID].type = APP_CONFIG_TYPE_STRING;
  sys_config.configs[WIFI_SSID].string = sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_WIFI_SSID));
  memcpy(sys_config.configs[WIFI_SSID].string, DEFAULT_WIFI_SSID, sizeof(DEFAULT_WIFI_SSID));
  // wifi password
  sys_config.configs[WIFI_PASSWORD].type = APP_CONFIG_TYPE_STRING;
  sys_config.configs[WIFI_PASSWORD].string = sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_WIFI_PASSWORD));
  memcpy(sys_config.configs[WIFI_PASSWORD].string, DEFAULT_WIFI_PASSWORD, sizeof(DEFAULT_WIFI_PASSWORD));
  // cloud platform
  sys_config.configs[CLOUD_PLATFORM].type = APP_CONFIG_TYPE_INT;
  sys_config.configs[CLOUD_PLATFORM].Int = DEFAULT_CLOUD_PLATFORM;
  // cloud product id
  sys_config.configs[CLOUD_PRODUCT_ID].type = APP_CONFIG_TYPE_STRING;
  sys_config.configs[CLOUD_PRODUCT_ID].string = sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_CLOUD_PRODUCT_ID));
  memcpy(sys_config.configs[CLOUD_PRODUCT_ID].string, DEFAULT_CLOUD_PRODUCT_ID, sizeof(DEFAULT_CLOUD_PRODUCT_ID));
  // cloud device id
  sys_config.configs[CLOUD_DEVICE_ID].type = APP_CONFIG_TYPE_STRING;
  sys_config.configs[CLOUD_DEVICE_ID].string = sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_CLOUD_DEVICE_ID));
  memcpy(sys_config.configs[CLOUD_DEVICE_ID].string, DEFAULT_CLOUD_DEVICE_ID, sizeof(DEFAULT_CLOUD_DEVICE_ID));
  // cloud device secret
  sys_config.configs[CLOUD_DEVICE_SECRET].type = APP_CONFIG_TYPE_STRING;
  sys_config.configs[CLOUD_DEVICE_SECRET].string = sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(DEFAULT_CLOUD_DEVICE_SECRET));
  memcpy(sys_config.configs[CLOUD_DEVICE_SECRET].string, DEFAULT_CLOUD_DEVICE_SECRET, sizeof(DEFAULT_CLOUD_DEVICE_SECRET));
  sys_config.attr.is_loaded = 1;
}

int sys_config_init(void) {
  log_i("Loading system configuration...");
  sys_config.configs =
      sys_malloc(SYS_CONFIG_MEM_SOURCE, sizeof(app_config_t) * SYS_CONFIG_KEY_MAX);
  if (sys_config.configs == NULL) {
    log_e("Failed to allocate memory for sys_config");
    return -1;
  }
  sys_config_set_defaults();
  return 0;
}

/**
 * @brief 获取系统配置句柄
 *
 * @return app_settings_t*
 */
app_settings_t *sys_config_get(void) { 
  if(!sys_config.attr.is_loaded){
    return NULL;
  }
  return &sys_config; 
}

char *sys_config_get_wifi_ssid(void) { 
  if(!sys_config.attr.is_loaded){
    return NULL;
  }
  return sys_config.configs[WIFI_SSID].string; 
}
char *sys_config_get_wifi_password(void) { 
  if(!sys_config.attr.is_loaded){
    return NULL;
  }
  return sys_config.configs[WIFI_PASSWORD].string; 
}
int sys_config_get_cloud_platform(void) { 
  if(!sys_config.attr.is_loaded){
    return -1;
  }
  return sys_config.configs[CLOUD_PLATFORM].Int; 
}
char *sys_config_get_cloud_product_id(void) { 
  if(!sys_config.attr.is_loaded){
    return NULL;
  }
  return sys_config.configs[CLOUD_PRODUCT_ID].string; 
}
char *sys_config_get_cloud_device_id(void) { 
  if(!sys_config.attr.is_loaded){
    return NULL;
  }
  return sys_config.configs[CLOUD_DEVICE_ID].string; 
}
char *sys_config_get_cloud_device_secret(void) { 
  if(!sys_config.attr.is_loaded){
    return NULL;
  }
  return sys_config.configs[CLOUD_DEVICE_SECRET].string; 
}


