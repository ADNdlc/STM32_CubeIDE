#ifndef APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_
#define APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_

#include <stdbool.h>
#include <stdint.h>
#include "app_settings.h"

/**
 * @brief 云平台类型
 */
typedef enum {
  CLOUD_PLATFORM_ONENET = 0,
  CLOUD_PLATFORM_ALIYUN,
  CLOUD_PLATFORM_CUSTOM
} cloud_platform_t;

typedef enum {
  // wifi
  WIFI_SSID,
  WIFI_PASSWORD,
  // mqtt cloud
  CLOUD_PLATFORM,
  CLOUD_PRODUCT_ID,
  CLOUD_DEVICE_ID,
  CLOUD_DEVICE_SECRET,

  SYS_CONFIG_KEY_MAX
} sys_config_key_t;

/**
 * @brief 初始化系统配置
 * @return 0 on success
 */
int sys_config_init(void);

void sys_config_set_defaults(void);

/**
 * @brief 获取系统配置
 */
app_settings_t *sys_config_get(void);

char *sys_config_get_wifi_ssid(void);
char *sys_config_get_wifi_password(void);
void sys_config_set_wifi_ssid(const char *ssid);
void sys_config_set_wifi_password(const char *password);
int sys_config_get_cloud_platform(void);
char *sys_config_get_cloud_product_id(void);
char *sys_config_get_cloud_device_id(void);
char *sys_config_get_cloud_device_secret(void);



#endif /* APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_ */
