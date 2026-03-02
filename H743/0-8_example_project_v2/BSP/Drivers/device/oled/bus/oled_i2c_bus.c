/*
 * oled_i2c_bus.c
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */

#include "oled_driver.h"
#include "i2c_driver.h"

// I2C 适配器私有配置结构体
typedef struct {
    i2c_driver_t *i2c_drv; // I2C 驱动实例
    uint16_t     dev_addr; // OLED 的 I2C 地址 (通常为 0x78 或 0x7A)
} oled_i2c_config_t;

// 实现总线的 write 方法
static int oled_i2c_bus_write(oled_bus_t *self, oled_data_type_t type, const uint8_t *data, uint32_t len) {
    oled_i2c_config_t *config = (oled_i2c_config_t *)self->priv_config;

    // I2C OLED 的 Control Byte:
    // 0x00: 接下来发的是 Command
    // 0x40: 接下来发的是 Data
    uint16_t mem_addr = (type == OLED_CMD) ? 0x00 : 0x40;

    // 调用你的 I2C 接口宏 (注意强转去掉 const 警告)
    return I2C_MEM_WRITE(config->i2c_drv,
                         config->dev_addr,
                         mem_addr,
                         I2C_MEMADD_SIZE_8BIT,
                         (uint8_t *)data,
                         len,
                         100); // 100ms timeout
}

// 暴露一个创建 I2C 总线对象的函数
oled_bus_t* OLED_I2C_Bus_Create(i2c_driver_t *i2c_drv, uint16_t addr) {
    // 实际项目中建议用 sys_malloc 分配
    static oled_i2c_config_t config;
    static oled_bus_t bus;

    config.i2c_drv = i2c_drv;
    config.dev_addr = addr;

    bus.write = oled_i2c_bus_write;
    bus.hard_reset = NULL; // I2C 通常无硬复位
    bus.priv_config = &config;

    return &bus;
}
