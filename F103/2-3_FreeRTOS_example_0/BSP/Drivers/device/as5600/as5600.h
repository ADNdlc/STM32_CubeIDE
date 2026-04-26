#ifndef BSP_DRIVERS_DEVICE_AS5600_AS5600_H_
#define BSP_DRIVERS_DEVICE_AS5600_AS5600_H_

#include "interface_inc.h"
#include "absolute_encoder_driver.h"

// AS5600 驱动结构体
typedef struct {
    absolute_encoder_driver_t base; // 继承基类
    i2c_driver_t *i2c_drv;         // 依赖的 I2C 驱动
    uint16_t dev_addr;             // I2C 设备地址
} as5600_driver_t;

/**
 * @brief 创建 AS5600 驱动实例
 * @param i2c_drv I2C 驱动实例
 * @return 绝对值编码器驱动指针
 */
absolute_encoder_driver_t *as5600_driver_create(i2c_driver_t *i2c_drv);

#endif /* BSP_DRIVERS_DEVICE_AS5600_AS5600_H_ */
