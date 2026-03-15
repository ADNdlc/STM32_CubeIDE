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
#include "lcd_screen_driver.h"
#include "nor_flash_driver.h"

/* ----- GPIO ----- */
typedef enum {
  TOUCH_RST = 0,
  TOUCH_INT,
  GPIO_ID_LED0,
  GPIO_ID_LED1,
  LCD_BL,
  ESP_RST,
  GPIO_ID_KEY0,
  GPIO_ID_KEY1,
  GPIO_ID_KEY2,
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

/* ----- i2c ----- */
typedef enum {
  I2C_BUS_SENSOR = 0, // 传感器I2C总线
  I2C_BUS_PWR,        // 电源监测(高占用)
  I2C_BUS_TOUCH,
  //...
  I2C_MAX_DEVICES
} i2c_device_id_t;
typedef struct {
  void *resource; // 硬件总线资源
} i2c_mapping_t;
extern const i2c_mapping_t i2c_mappings[I2C_MAX_DEVICES];

/* ----- qspi ----- */
typedef enum {
  QSPI_ID_FLASH = 0, // QSPI Flash
  //...
  QSPI_MAX_DEVICES
} qspi_device_id_t;
typedef struct {
  void *resource; // 硬件总线资源
} qspi_mapping_t;
extern const qspi_mapping_t qspi_mappings[QSPI_MAX_DEVICES];

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

/* ----- RTC ----- */
typedef enum { RTC_ID_INTERNAL = 0, RTC_MAX } rtc_device_id_t;
typedef struct {
  void *resource;
} rtc_mapping_t;
extern const rtc_mapping_t rtc_mappings[RTC_MAX];

/* ----- Timer ----- */
typedef enum {
  TIMER_ID_LV = 0,
  TIMER_ID_2,

  TIMER_ID_MAX
} timer_device_id_t;
typedef struct {
  void *resource;
} timer_mapping_t;
extern const timer_mapping_t timer_mappings[TIMER_ID_MAX];

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
typedef enum { TOUCH_ID_UI = 0, TOUCH_MAX } touch_id_t;
typedef struct {
  void *resource;
} touch_mapping_t;
extern const touch_mapping_t touch_mappings[TOUCH_MAX];

/* ----- LCD屏 ----- */
typedef enum { LCD_ID_UI = 0, LCD_ID_MAX } lcd_id_t;
typedef struct {
  void *resource;
  lcd_screen_info_t info;
} lcd_mapping_t;
extern const lcd_mapping_t lcd_mappings[LCD_ID_MAX];

/* ----- OLED ----- */
typedef enum {
  OLED_ID_MAIN = 0,
  //...
  OLED_ID_MAX
} oled_id_t;
typedef struct {
  void *resource;
} oled_mapping_t;
extern const oled_mapping_t oled_mappings[OLED_ID_MAX];

/* ----- norflash ----- */
typedef enum { NOR_FLASH_SYS = 0, NOR_FLASH_MAX } norflash_id_t;
typedef struct {
  nor_flash_info_t *manual_info; // 手动配置项(可缺省)
  void *resource;                // 硬件总线资源
} norflash_mapping_t;
extern const norflash_mapping_t norflash_mappings[NOR_FLASH_MAX];

/* ----- 电源监测 ----- */
typedef enum {
  POWER_MONITOR_ID_MAIN = 0, // 主电源监测
  //...
  POWER_MONITOR_MAX
} power_monitor_id_t;
typedef struct {
  void *resource; // 关联的总线资源 (如 i2c_driver_t*)
} power_monitor_mapping_t;
extern const power_monitor_mapping_t power_monitor_mappings[POWER_MONITOR_MAX];

// 辅助宏
#define CALL_RESOURE(config, type, res_name)                                   \
  ((type)(((config *)mapping->resource)->res_name))

#endif /* BOARD_DEV_MAP_H_ */
