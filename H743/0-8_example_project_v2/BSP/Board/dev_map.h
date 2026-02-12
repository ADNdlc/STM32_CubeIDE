/*
 * dev_map.h
 *
 *  Created on: Feb 6, 2026
 *      Author: 12114
 */

#ifndef BOARD_DEV_MAP_H_
#define BOARD_DEV_MAP_H_

#include "dev_map_config.h"

/* ----- GPIO ----- */
typedef enum {
  TOUCH_RST = 0,
  TOUCH_INT = 1,
  GPIO_ID_LED0 = 2,
  GPIO_ID_LED1 = 3,
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

/* ----- SDRAM ----- */
typedef enum {
  SDRAM_MAIN = 0,
  //...
  SDRAM_MAX_DEVICES
} sdram_device_id_t;
typedef struct {
  void *resource;
} sdram_mapping_t;
extern const sdram_mapping_t sdram_mappings[SDRAM_MAX_DEVICES];

/* ----- one_wire ----- */
typedef enum {
  ONE_WIRE_DHT11 = 0,
  //...
  ONE_WIRE_MAX_DEVICES
} one_wire_device_id_t;
typedef struct {
  void *resource;
} one_wire_mapping_t;
extern const one_wire_mapping_t one_wire_mappings[ONE_WIRE_MAX_DEVICES];

/* ----- 温湿度传感器 ----- */
// 温湿度传感器逻辑号
typedef enum {
  TH_SENSOR_ID_AMBIENT = 0, // 温湿度传感器
  //...
  TH_SENSOR_MAX
} th_sensor_id_t;
// 设备资源映射结构
typedef struct {
  void *resource; // 关联的总线资源 (如 one_wire_driver_t* 或 i2c_driver_t*)
} th_sensor_mapping_t;
extern const th_sensor_mapping_t th_sensor_mappings[TH_SENSOR_MAX];

/* ----- i2c ----- */
typedef enum {
  I2C_BUS_SENSOR = 0, // 传感器I2C总线
  I2C_BUS_TOUCH = 1,
  //...
  I2C_MAX_DEVICES
} i2c_device_id_t;
typedef struct {
  void *resource; // 硬件总线资源
} i2c_mapping_t;
extern const i2c_mapping_t i2c_mappings[I2C_MAX_DEVICES];

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

/* ----- 触摸屏 ----- */
typedef enum { 
  TOUCH_ID_UI = 0, 
  TOUCH_MAX 
} touch_id_t;
typedef struct {
  void *resource; // 逻辑资源映射
} touch_mapping_t;
extern const touch_mapping_t touch_mappings[TOUCH_MAX];

#endif /* BOARD_DEV_MAP_H_ */
