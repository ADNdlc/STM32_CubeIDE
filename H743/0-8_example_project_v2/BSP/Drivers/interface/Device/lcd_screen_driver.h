/*
 * screen_driver.h
 *
 *  Created on: Feb 26, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_INTERFACE_DEVICE_LCD_SCREEN_DRIVER_H_
#define DRIVERS_INTERFACE_DEVICE_LCD_SCREEN_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

#define COLOR_DEPTH_8	uint8_t
#define COLOR_DEPTH_16	uint16_t
#define COLOR_DEPTH_24	uint32_t

#define COLOR_DEPTH COLOR_DEPTH_16	// 定义驱动使用的颜色宽度

typedef enum {
	HORIZONTAL = 0,
	VERTICAL = 1
} disp_direction_t;

/**
 * @brief LCD 信息结构体
 */
typedef struct lcd_screen_info_t {
  uint16_t width;
  uint16_t height;
} lcd_screen_info_t;


/**
 * @brief LCD 驱动操作接口
 */
#define scr_color_t COLOR_DEPTH
typedef struct {
	int lcd_init(lcd_driver_t* self, lcd_screen_info_t *info, uint8_t* buffer);	// 设置信息和缓冲区并初始化

	int lcd_set_buffer(lcd_driver_t* self, uint8_t* buffer);			// 重新设置缓冲区
	int lcd_set_dir(lcd_driver_t* self, disp_direction_t direction);	// 设置显示方向
	int lcd_display_on(void);											// 开启显示
	int lcd_display_off(void);											// 关闭显示

	scr_color_t* lcd_read_point(uint16_t x, uint16_t y); 										// 读取一个像素
	int lcd_draw_point(uint16_t x, uint16_t y, scr_color_t color);								// 单点绘制
	int lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, scr_color_t color);		// 纯色填充
	int lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, scr_color_t *color); // 画面(数据)填充
}lcd_driver_ops_t;

/**
 * @brief LCD 驱动接口
 */
typedef struct {
	lcd_driver_ops_t *ops;
	lcd_screen_info_t *info;
}lcd_driver_t;

#endif /* DRIVERS_INTERFACE_DEVICE_LCD_SCREEN_DRIVER_H_ */
