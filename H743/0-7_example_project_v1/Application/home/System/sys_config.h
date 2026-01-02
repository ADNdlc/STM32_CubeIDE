#ifndef APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_
#define APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_

#include <stdbool.h>
#include <stdint.h>

// Internal storage constants (Bank 2, Sector 7 for STM32H743)
#define SYS_CFG_INTERNAL_ADDR 0x081E0000
#define SYS_CFG_MAGIC 0x53434647 // "SCFG" in ASCII

/**
 * @brief Network Configuration
 */
typedef struct {
  char ssid[33];
  char password[64];
  bool auto_connect;
} net_config_t;

/**
 * @brief Cloud Platform Types
 */
typedef enum {
  CLOUD_PLATFORM_ONENET = 0,
  CLOUD_PLATFORM_ALIYUN,
  CLOUD_PLATFORM_CUSTOM
} cloud_platform_t;

/**
 * @brief Cloud Configuration
 */
typedef struct {
  cloud_platform_t platform;
  char device_id[33];
  char product_id[33];
  char device_secret[64];
} cloud_config_t;

/**
 * @brief Global System Configuration
 */
typedef struct {
  net_config_t net;
  cloud_config_t cloud;
} sys_config_t;

/**
 * @brief Initialize system configuration, load from flash
 * @return 0 on success
 */
int sys_config_init(void);

/**
 * @brief Get global config handle (read-only)
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
 * @brief Save current configuration to flash
 * @return 0 on success
 */
int sys_config_save(void);

#endif /* APPLICATION_HOME_SYSTEM_SYS_CONFIG_H_ */
