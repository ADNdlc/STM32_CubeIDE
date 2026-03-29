/*
 * pwm_factory.h
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#ifndef FACTORY_PWM_FACTORY_H_
#define FACTORY_PWM_FACTORY_H_

#include "dev_map.h"
#include "pwm_driver.h"

// PWM工厂函数，通过逻辑标识符获取对应的驱动实例
pwm_driver_t *pwm_driver_get(pwm_device_id_t id);

#endif /* FACTORY_PWM_FACTORY_H_ */
