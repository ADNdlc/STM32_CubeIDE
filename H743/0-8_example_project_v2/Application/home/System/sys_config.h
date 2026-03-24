#ifndef APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_
#define APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_

#include "app_settings.h"
#include <stdbool.h>
#include <stdint.h>


/**
 * @brief System Config
 *
 * 此模块负责维护系统运行时的全局状态，如音量、亮度等。并提供状态改变时的观察者通知机制(如UI更新)。
 *
 */

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
  CLOUD_PLATFORM,      // 平台类型
  CLOUD_PRODUCT_ID,    // 产品ID
  CLOUD_DEVICE_ID,     // 设备ID
  CLOUD_DEVICE_SECRET, // 密钥

  SYS_CONFIG_KEY_MAX
} sys_config_key_t;

int sys_config_init(void);

int sys_config_save(void); // 保存系统配置到文件系统

void sys_config_set_defaults(void); // 设置为程序默认参数(调试)

/**
 * @brief 获取系统配置(外部调用)
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
