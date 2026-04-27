#include "as5600.h"
#include <stdlib.h>
#include <math.h>

#include "elog.h"
#define LOG_TAG "AS5600"

// AS5600 寄存器定义
#define AS5600_ADDR 0x36 << 1    // AS5600的I2C从机地址（左移1位）
#define AS5600_REG_ZPOS_H 0x01      // 起始位置11-8
#define AS5600_REG_ZPOS_L 0x02      // 起始位置7-0
#define AS5600_REG_MPOS_H 0x03      // 终止位置11-8
#define AS5600_REG_MPOS_L 0x04      // 终止位置7-0
#define AS5600_REG_MANG_H 0x05      // 最大角度11-8
#define AS5600_REG_MANG_L 0x06      // 最大角度7-0
#define AS5600_REG_CFG_1 0x07       // CONF高位寄存器
#define AS5600_REG_CFG_2 0x08       // CONF低位寄存器
#define AS5600_REG_RAW_ANGLE 0x0C	// 原始角度
#define AS5600_REG_ANGLE 0x0E		// 角度
#define AS5600_REG_STATUS 0x0B      // 状态寄存器(读)
#define AS5600_REG_AGC 0x1A         // AGC寄存器(读)
#define AS5600_REG_MAGNITUDE_1 0x1B // MAGNITUDE寄存器(读)
#define AS5600_REG_MAGNITUDE_2 0x1C // MAGNITUDE寄存器(读)

// PI 常量
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static uint32_t _as5600_get_raw_angle(absolute_encoder_driver_t *base)
{
    as5600_driver_t *self = (as5600_driver_t *)base;
    uint8_t buf[2];

    // 读取原始角度 (0x0C, 0x0D)
    self->last_i2c_status = I2C_MEM_READ(self->i2c_drv, self->dev_addr, AS5600_REG_RAW_ANGLE, I2C_MEMADD_SIZE_8BIT, buf, 2, 100);

    if (self->last_i2c_status == 0)
    {
        return ((uint32_t)buf[0] << 8) | buf[1];
    }
    else
    {
        self->error_count++;
        // 每隔 20 次打印一次错误
        if (self->error_count % 20 == 1)
        {
            log_e("I2C Read Angle Failed! Status: %d, Addr: 0x%02X", self->last_i2c_status, self->dev_addr);
        }
        return 0;
    }
}

static float _as5600_get_angle(absolute_encoder_driver_t *base)
{
    uint16_t raw = _as5600_get_raw_angle(base);
    // 12 位原始值转弧度: raw * 2PI / 4096
    return (float)raw * (2.0f * M_PI / 4096.0f);
}

static uint8_t _as5600_get_status(absolute_encoder_driver_t *base)
{
    as5600_driver_t *self = (as5600_driver_t *)base;
    uint8_t status = 0;

    self->last_i2c_status = I2C_MEM_READ(self->i2c_drv, self->dev_addr, AS5600_REG_STATUS, I2C_MEMADD_SIZE_8BIT, &status, 1, 100);

    if (self->last_i2c_status == 0)
    {
        // bit 5 (MD): Magnet Detected
        return (status & 0x20) ? 1 : 0;
    }
    return 0;
}

static uint8_t _as5600_get_agc(absolute_encoder_driver_t *base)
{
    as5600_driver_t *self = (as5600_driver_t *)base;
    uint8_t agc = 0;
    self->last_i2c_status = I2C_MEM_READ(self->i2c_drv, self->dev_addr, AS5600_REG_AGC, I2C_MEMADD_SIZE_8BIT, &agc, 1, 100);
    if (self->last_i2c_status == 0)
    {
        return agc;
    }
    return 0;
}

static const absolute_encoder_driver_ops_t as5600_ops = {
    .get_raw_angle = _as5600_get_raw_angle,
    .get_angle = _as5600_get_angle,
    .get_acg = _as5600_get_agc,
    .get_status = _as5600_get_status,
};

absolute_encoder_driver_t *as5600_driver_create(i2c_driver_t *i2c_drv)
{
    as5600_driver_t *self = (as5600_driver_t *)malloc(sizeof(as5600_driver_t));
    if (self){
        self->base.ops = &as5600_ops;
        self->i2c_drv = i2c_drv;
        self->dev_addr = AS5600_ADDR;
        self->error_count = 0;
        self->last_i2c_status = 0;
        log_i("AS5600 Driver Initializing. Scanning I2C bus...");

        // 扫描总线，寻找响应的设备
        int found_count = 0;
        for (uint16_t i = 0; i < 128; i++){
            // 使用HAL_I2C_IsDeviceReady的底层对应操作（通常是发送地址看有无 ACK）
            // 这里通过 i2c_drv 的 master_transmit 发送 0 字节来测试
            uint8_t dummy = 0;
            if (I2C_MASTER_TRANSMIT(self->i2c_drv, (i << 1), &dummy, 0, 10) == 0)
            {
                log_i("  Found I2C device at 7-bit addr: 0x%02X (8-bit: 0x%02X)", i, (i << 1));
                found_count++;
            }
        }
        if (found_count == 0){
            log_e("  No I2C devices found on this bus! Please check wiring and pull-up resistors.");
        }
        else{
            log_i("  Scan complete. Found %d device(s).", found_count);
        }
        log_i("AS5600 Target Addr: 0x%02X", self->dev_addr);
    }
    return self ? &self->base : NULL;
}
