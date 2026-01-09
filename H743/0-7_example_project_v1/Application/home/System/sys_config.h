#ifndef APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_
#define APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief wifi连接配置
 */
typedef struct {
  char ssid[33];
  char password[64];
  bool auto_connect;
} net_config_t;

/**
 * @brief 云平台类型
 */
typedef enum {
  CLOUD_PLATFORM_ONENET = 0,
  CLOUD_PLATFORM_ALIYUN,
  CLOUD_PLATFORM_CUSTOM
} cloud_platform_t;

/**
 * @brief 云服务配置
 */
typedef struct {
  cloud_platform_t platform;
  char device_id[65];
  char product_id[65];
  char device_secret[256];
} cloud_config_t;

/**
 * @brief 全局系统配置
 */
typedef struct {
  net_config_t net;
  cloud_config_t cloud;
} sys_config_t;

/**
 * @brief 初始化系统配置
 * @return 0 on success
 */
int sys_config_init(void);

/**
 * @brief 获取系统配置
 */
const sys_config_t *sys_config_get(void);

/**
 * @brief Update network configuration
 */
void sys_config_set_net(const net_config_t *net);

/**
 * @brief Update cloud configuration
 */
void sys_config_set_cloud(const cloud_config_t *cloud);

/**
 * @brief 保存当前配置到flash
 * @return 0 on success
 */
int sys_config_save(void);

#endif /* APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_ */
