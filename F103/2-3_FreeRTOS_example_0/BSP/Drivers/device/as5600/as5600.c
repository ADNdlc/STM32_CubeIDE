#include "as5600.h"
#include <stdlib.h>
#include <math.h>

// AS5600 寄存器定义
#define AS5600_ADDR             (0x36 << 1)
#define AS5600_REG_STATUS       0x0B
#define AS5600_REG_RAW_ANGLE    0x0C
#define AS5600_REG_ANGLE        0x0E

// PI 常量
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static uint16_t _as5600_get_raw_angle(absolute_encoder_driver_t *base) {
    as5600_driver_t *self = (as5600_driver_t *)base;
    uint8_t buf[2];
    
    // 读取原始角度 (0x0C, 0x0D)
    if (I2C_MEM_READ(self->i2c_drv, self->dev_addr, AS5600_REG_RAW_ANGLE, I2C_MEMADD_SIZE_8BIT, buf, 2, 100) == 0) {
        return ((uint16_t)buf[0] << 8) | buf[1];
    }
    return 0;
}

static float _as5600_get_angle(absolute_encoder_driver_t *base) {
    uint16_t raw = _as5600_get_raw_angle(base);
    // 12 位原始值转弧度: raw * 2PI / 4096
    return (float)raw * (2.0f * M_PI / 4096.0f);
}

static uint8_t _as5600_get_status(absolute_encoder_driver_t *base) {
    as5600_driver_t *self = (as5600_driver_t *)base;
    uint8_t status = 0;
    
    if (I2C_MEM_READ(self->i2c_drv, self->dev_addr, AS5600_REG_STATUS, I2C_MEMADD_SIZE_8BIT, &status, 1, 100) == 0) {
        // bit 5 (MD): Magnet Detected
        return (status & 0x20) ? 1 : 0;
    }
    return 0;
}

static const absolute_encoder_driver_ops_t as5600_ops = {
    .get_raw_angle = _as5600_get_raw_angle,
    .get_angle = _as5600_get_angle,
    .get_status = _as5600_get_status,
};

absolute_encoder_driver_t *as5600_driver_create(i2c_driver_t *i2c_drv) {
    as5600_driver_t *self = (as5600_driver_t *)malloc(sizeof(as5600_driver_t));
    if (self) {
        self->base.ops = &as5600_ops;
        self->i2c_drv = i2c_drv;
        self->dev_addr = AS5600_ADDR;
    }
    return &self->base;
}
