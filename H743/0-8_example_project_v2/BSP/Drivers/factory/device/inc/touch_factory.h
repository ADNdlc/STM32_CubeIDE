/*
 * touch_factory.h
 *
 *  Created on: Feb 12, 2026
 *      Author: Antigravity
 *
 *  触摸屏驱动工厂
 */

#ifndef BSP_DRIVERS_FACTORY_DEVICE_TOUCH_FACTORY_H_
#define BSP_DRIVERS_FACTORY_DEVICE_TOUCH_FACTORY_H_

#include "dev_map.h"
#include "touch_driver.h"


/**
 * @brief 获取触摸屏驱动实例
 * @param id 触摸屏逻辑 ID
 * @return 驱动指针，失败返回 NULL
 */
touch_driver_t *touch_driver_get(touch_id_t id);

#endif /* BSP_DRIVERS_FACTORY_DEVICE_TOUCH_FACTORY_H_ */
