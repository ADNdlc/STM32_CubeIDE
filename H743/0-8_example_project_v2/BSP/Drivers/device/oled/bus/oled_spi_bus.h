/*
 * oled_spi_bus.h
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_DEVICE_OLED_OLED_SPI_BUS_H_
#define DRIVERS_DEVICE_OLED_OLED_SPI_BUS_H_

#include "dev_map.h"
#include "oled_driver.h"
#include "spi_driver.h"
#include "gpio_driver.h"

// SPI 适配器配置
typedef struct {
  spi_device_id_t spi_id;
  gpio_device_id_t dc_pin;  // Data/Command 切换引脚
  gpio_device_id_t res_pin; // 复位引脚 (可选)
} oled_spi_config_t;

typedef struct {
  spi_driver_t *spi_dev;
  gpio_driver_t *dc_pin;
  gpio_driver_t *res_pin;
} bus_drivers_t;

typedef struct {
  oled_bus_t base;
  bus_drivers_t bus_drivers;
} oled_spi_bus_t;

oled_bus_t *OLED_SPI_Bus_Create(bus_drivers_t *bus_drivers,
                                const oled_spi_config_t *config);

#endif /* DRIVERS_DEVICE_OLED_OLED_SPI_BUS_H_ */
