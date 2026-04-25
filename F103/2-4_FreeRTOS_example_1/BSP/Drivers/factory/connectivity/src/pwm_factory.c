/*
 * pwm_factory.c
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#include "..\inc\pwm_factory.h"
#include "dev_map.h"

#if (TARGET_PLATFORM == PLATFORM_STM32)
#include "stm32_inc.h"
#endif

// 存储已创建的PWM驱动实例指针数组
static pwm_driver_t *pwm_drivers[PWM_ID_MAX] = {NULL};

pwm_driver_t *pwm_driver_get(pwm_device_id_t id) {
  // 检查ID是否有效
  if (id >= PWM_ID_MAX) {
    return NULL;
  }
  // 如果该驱动尚未创建，则创建它
  if (pwm_drivers[id] == NULL) {
    const pwm_mapping_t *mapping = &pwm_mappings[id];

// 根据平台配置创建相应的PWM驱动实例
#if (TARGET_PLATFORM == PLATFORM_STM32)
    // 创建具体的STM32 PWM驱动实例
    pwm_drivers[id] = (pwm_driver_t *)stm32_pwm_driver_create(
        (stm32_pwm_config_t *)mapping->resource);
#else
#error "未定义PWM_DRIVER_PLATFORM或平台不支持"
#endif
  }

  // 返回驱动实例
  return pwm_drivers[id];
}
