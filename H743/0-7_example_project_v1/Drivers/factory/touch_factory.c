/*
 * touch_factory.c
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  触摸屏驱动工厂实现
 */

#include "touch_factory.h"
#include "device_mapping.h"
#include "factory_config.h"
#include "gpio_factory.h"
#include "gt9xxx_touch/gt9xxx_touch_driver.h"
#include "i2c_factory.h"


// 存储已创建的触摸屏驱动实例指针数组
static touch_driver_t *touch_drivers[TOUCH_MAX_DEVICES] = {NULL};

touch_driver_t *touch_driver_get(touch_device_id_t id) {
  // 检查ID是否有效
  if (id >= TOUCH_MAX_DEVICES) {
    return NULL;
  }

  // 如果该驱动尚未创建，则创建它
  if (touch_drivers[id] == NULL) {
    const touch_mapping_t *mapping = &touch_mappings[id];

    // 获取依赖的驱动实例(使用其他工厂函数获取)
    i2c_driver_t *i2c = i2c_soft_driver_get(mapping->i2c_id);
    gpio_driver_t *rst_gpio = gpio_driver_get(mapping->rst_gpio_id);
    gpio_driver_t *int_gpio = gpio_driver_get(mapping->int_gpio_id);

    if (i2c == NULL || rst_gpio == NULL || int_gpio == NULL) {
      return NULL;
    }

    // 创建 GT9xxx 触摸屏驱动配置
    gt9xxx_config_t config = {
        .i2c = i2c,
        .rst_gpio = rst_gpio,
        .int_gpio = int_gpio,
        .addr_mode = (gt9xxx_addr_mode_t)mapping->i2c_addr_mode,
    };

    // 创建驱动实例
    gt9xxx_touch_driver_t *drv = gt9xxx_touch_create(&config);
    if (drv) {
      touch_drivers[id] = (touch_driver_t *)drv;
    }
  }

  return touch_drivers[id];
}
