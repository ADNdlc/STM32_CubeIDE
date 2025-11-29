/*
 * pwm_factory.c
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#include "pwm_factory.h"
#include "stm32_pwm_driver.h"
#include "device_mapping.h"

// 存储已创建的PWM驱动实例指针数组
static pwm_driver_t* pwm_drivers[PWM_MAX_DEVICES] = {NULL};

pwm_driver_t* pwm_driver_get(pwm_device_id_t id) {
    // 检查ID是否有效
    if (id >= PWM_MAX_DEVICES) {
        return NULL;
    }
    // 如果该驱动尚未创建，则创建它
    if (pwm_drivers[id] == NULL) {
        const pwm_mapping_t* mapping = &pwm_mappings[id];
        // 创建具体的STM32 PWM驱动实例
        pwm_drivers[id] = (pwm_driver_t*)stm32_pwm_driver_create(mapping->htim, mapping->channel);
    }

    // 返回驱动实例
    return pwm_drivers[id];
}
