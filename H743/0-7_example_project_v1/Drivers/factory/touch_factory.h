/*
 * touch_factory.h
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  触摸屏驱动工厂
 *  根据配置创建对应的触摸屏驱动实例
 */

#ifndef FACTORY_TOUCH_FACTORY_H_
#define FACTORY_TOUCH_FACTORY_H_

#include "device_mapping.h"
#include "touch_driver.h"


/**
 * @brief 获取触摸屏驱动实例
 * @param id 触摸屏设备 ID
 * @return 触摸屏驱动实例指针，失败返回 NULL
 */
touch_driver_t *touch_driver_get(touch_device_id_t id);

#endif /* FACTORY_TOUCH_FACTORY_H_ */
