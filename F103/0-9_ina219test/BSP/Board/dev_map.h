/*
 * dev_map.h
 *
 *  Created on: Feb 6, 2026
 *      Author: 12114
 */

#ifndef BOARD_DEV_MAP_H_
#define BOARD_DEV_MAP_H_
#include <stdbool.h>
#include <stdint.h>

#include "dev_map_config.h"

/* ----- GPIO ----- */
typedef enum {
  GPIO_ID_LED0 = 0,
  //...
  GPIO_MAX_DEVICES
} gpio_device_id_t;
typedef struct {
  void *resource;
} gpio_mapping_t;
extern const gpio_mapping_t gpio_mappings[GPIO_MAX_DEVICES];

/* ----- USART ----- */
// 定义串口逻辑ID，供上层应用和Factory层使用
typedef enum {
  USART_ID_DEBUG = 0, // 日志和调试
  USART_ID_ESP8266,   // 网络通信模块
  // ...
  USART_MAX_DEVICES
} usart_device_id_t;
// 定义映射项结构（屏蔽平台差异）
typedef struct {
  void *resource; // 物理资源句柄
} usart_mapping_t;
// 声明映射表
extern const usart_mapping_t usart_mappings[USART_MAX_DEVICES];

/* ----- i2c ----- */
typedef enum {
  I2C_BUS_SENSOR = 0, // 传感器I2C总线
  //...
  I2C_MAX_DEVICES
} i2c_device_id_t;
typedef struct {
  void *resource; // 硬件总线资源
} i2c_mapping_t;
extern const i2c_mapping_t i2c_mappings[I2C_MAX_DEVICES];

/* ----- spi ----- */
typedef enum {
  SPI_ID_1 = 0,
  SPI_ID_2 = 1,
  //...
  SPI_MAX_DEVICES
} spi_device_id_t;
typedef struct {
  void *resource; // 硬件总线资源
} spi_mapping_t;
extern const spi_mapping_t spi_mappings[SPI_MAX_DEVICES];

/* ----- 光照传感器 ----- */
typedef enum {
  LIGHT_SENSOR_ID_AMBIENT = 0, // 环境光传感器
  //...
  LIGHT_SENSOR_MAX
} light_sensor_id_t;
typedef struct {
  void *resource; // 关联的总线资源 (如 i2c_driver_t*)
} light_sensor_mapping_t;
extern const light_sensor_mapping_t light_sensor_mappings[LIGHT_SENSOR_MAX];

/* ----- RTC ----- */
typedef enum { RTC_ID_INTERNAL = 0, RTC_MAX } rtc_device_id_t;
typedef struct {
  void *resource;
} rtc_mapping_t;
extern const rtc_mapping_t rtc_mappings[RTC_MAX];

/* ----- Timer ----- */
typedef enum {
	TIMER_ID_1 = 0,
	TIMER_ID_2,

	TIMER_ID_MAX
} timer_device_id_t;
typedef struct {
  void *resource;
} timer_mapping_t;
extern const timer_mapping_t timer_mappings[TIMER_ID_MAX];

// 辅助宏
#define CALL_RESOURE(config, type, res_name)                                   \
  ((type)(((config *)mapping->resource)->res_name))

#endif /* BOARD_DEV_MAP_H_ */
