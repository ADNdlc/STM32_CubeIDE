/*
 * oled_driver.h
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_INTERFACE_DEVICE_OLED_OLED_DRIVER_H_
#define DRIVERS_INTERFACE_DEVICE_OLED_OLED_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>

/* 前向声明 */
typedef struct oled_device_t oled_device_t;
typedef struct oled_bus_t oled_bus_t;

/* ========================================= */
/* 1. 总线适配器接口 (Transport Layer)       */
/* ========================================= */
typedef enum {
    OLED_CMD  = 0,
    OLED_DATA = 1
} oled_data_type_t;

struct oled_bus_t {
    // 基础发送接口
    int (*write)(oled_bus_t *self, oled_data_type_t type, const uint8_t *data, uint32_t len);
    // 可选复位接口 (I2C通常不用，预留给SPI)
    void (*hard_reset)(oled_bus_t *self);
    // 存放 I2C/SPI 具体驱动句柄及其它配置
    void *priv_config;
};

/* ========================================= */
/* 2. 芯片操作接口 (Chip Ops Layer)          */
/* ========================================= */
typedef struct {
    int (*init)(oled_device_t *dev);
    int (*set_cursor)(oled_device_t *dev, uint8_t page, uint8_t col);
    int (*set_contrast)(oled_device_t *dev, uint8_t contrast);
    int (*display_on)(oled_device_t *dev, bool on);
} oled_chip_ops_t;

/* ========================================= */
/* 3. 设备实体定义 (Device Layer)            */
/* ========================================= */
struct oled_device_t {
    const oled_chip_ops_t *ops;  // 绑定的芯片驱动 (如 SSD1306)
    oled_bus_t            *bus;  // 绑定的通信总线 (如 I2C)

    uint16_t width;              // 宽度 (如 128)
    uint16_t height;             // 高度 (如 64 或 32)
    uint8_t  col_offset;         // 列偏移 (SSD1306通常为0，SH1106为2)

    uint8_t *frame_buffer;       // MCU 本地显存指针
};

/* ========================================= */
/* 4. 图形与刷新 API                         */
/* ========================================= */
void OLED_Init(oled_device_t *dev);
void OLED_Flush(oled_device_t *dev);
void OLED_Clear(oled_device_t *dev);
void OLED_DrawPoint(oled_device_t *dev, int x, int y, uint8_t color);

#endif /* DRIVERS_INTERFACE_DEVICE_OLED_OLED_DRIVER_H_ */
