/*
 * lcd_screen_factory.h
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_FACTORY_DEVICE_INC_LCD_SCREEN_FACTORY_H_
#define DRIVERS_FACTORY_DEVICE_INC_LCD_SCREEN_FACTORY_H_

#include "lcd_screen_driver.h"
#include "dev_map.h"

lcd_driver_t *lcd_screen_factory_creat(lcd_id_t id);

#endif /* DRIVERS_FACTORY_DEVICE_INC_LCD_SCREEN_FACTORY_H_ */
