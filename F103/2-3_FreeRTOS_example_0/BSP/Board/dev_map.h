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
//  GPIO_ID_OLED_DC,
//  GPIO_ID_OLED_RES,
  //...
  GPIO_MAX_DEVICES
} gpio_device_id_t;
typedef struct {
  void *resource;
} gpio_mapping_t;
extern const gpio_mapping_t gpio_mappings[GPIO_MAX_DEVICES];

/* ----- USART ----- */
typedef enum {
  USART_ID_DEBUG = 0,
  // ...
  USART_MAX_DEVICES
} usart_device_id_t;
typedef struct {
  void *resource;
} usart_mapping_t;
extern const usart_mapping_t usart_mappings[USART_MAX_DEVICES];

/* ----- i2c ----- */
typedef enum {
  I2C_BUS_OLED = 0,
  //...
  I2C_MAX_DEVICES
} i2c_device_id_t;
typedef struct {
  void *resource;
} i2c_mapping_t;
extern const i2c_mapping_t i2c_mappings[I2C_MAX_DEVICES];

/* ----- spi ----- */
typedef enum {
  SPI_ID_1 = 0,
  //...
  SPI_MAX_DEVICES
} spi_device_id_t;
typedef struct {
  void *resource;
} spi_mapping_t;
extern const spi_mapping_t spi_mappings[SPI_MAX_DEVICES];

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


/* ----- PWM ----- */
typedef enum {
  PWM_ID_R = 0,
  PWM_ID_G = 1,
  PWM_ID_B = 2,
  PWM_ID_MAX
} pwm_device_id_t;
typedef struct {
  void *resource;
} pwm_mapping_t;
extern const pwm_mapping_t pwm_mappings[PWM_ID_MAX];

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

/* ----- 电源监测 ----- */
typedef enum {
  POWER_MONITOR_ID_MAIN = 0,
  //...
  POWER_MONITOR_MAX
} power_monitor_id_t;
typedef struct {
  void *resource;
} power_monitor_mapping_t;
extern const power_monitor_mapping_t power_monitor_mappings[POWER_MONITOR_MAX];

#define CALL_RESOURE(config, type, res_name)                                   \
  ((type)(((config *)mapping->resource)->res_name))

#endif /* BOARD_DEV_MAP_H_ */
