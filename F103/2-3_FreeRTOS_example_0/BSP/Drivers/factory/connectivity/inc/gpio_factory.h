/*
 * gpio_factory.h
 *
 *  Created on: Nov 28, 2025
 *      Author: 12114
 */

#ifndef FACTORY_GPIO_FACTORY_H_
#define FACTORY_GPIO_FACTORY_H_

#include "dev_map.h"
#include "interface_inc.h"


// GPIO工厂函数，通过逻辑标识符获取对应的驱动实例
gpio_driver_t *gpio_driver_get(gpio_device_id_t id);

#endif /* FACTORY_GPIO_FACTORY_H_ */
