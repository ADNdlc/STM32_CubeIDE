/*
 * usart_factory.h
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_FACTORY_USART_FACTORY_H_
#define BSP_DEVICE_DRIVER_FACTORY_USART_FACTORY_H_

#include "dev_map.h"
#include "Connectivity\usart_driver.h"

usart_driver_t *usart_driver_get(usart_device_id_t id);

#endif /* BSP_DEVICE_DRIVER_FACTORY_USART_FACTORY_H_ */
