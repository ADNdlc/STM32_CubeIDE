/*
 * lcd_factory.h
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_FACTORY_LCD_FACTORY_H_
#define BSP_DEVICE_DRIVER_FACTORY_LCD_FACTORY_H_

#include "device_mapping.h"
#include "lcd_driver.h"

lcd_driver_t *lcd_driver_get(lcd_device_id_t id, uint16_t width, uint16_t height);

#endif /* BSP_DEVICE_DRIVER_FACTORY_LCD_FACTORY_H_ */
