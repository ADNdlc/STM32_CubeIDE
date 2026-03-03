/*
 * lcd_screen_factory.h
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_FACTORY_DEVICE_INC_LCD_SCREEN_FACTORY_H_
#define DRIVERS_FACTORY_DEVICE_INC_LCD_SCREEN_FACTORY_H_

#include "dev_map.h"
#include "lcd_screen_driver.h"


lcd_driver_t *lcd_screen_factory_create(lcd_id_t id);

#endif /* DRIVERS_FACTORY_DEVICE_INC_LCD_SCREEN_FACTORY_H_ */
