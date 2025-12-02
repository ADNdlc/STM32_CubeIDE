/*
 * usart_factory.c
 *
 *  Created on: Dec 1, 2025
 *      Author: 12114
 */

#include "usart_factory.h"
#include "device_mapping.h"
#include "stm32_usart_driver.h"
#include <stddef.h>


static usart_driver_t *usart_drivers[USART_MAX_DEVICES] = {NULL};

usart_driver_t *usart_driver_get(usart_device_id_t id) {
  if (id >= USART_MAX_DEVICES) {
    return NULL;
  }
  if (usart_drivers[id] == NULL) {
    const usart_mapping_t *mapping = &usart_mappings[id];
    usart_drivers[id] =
        (usart_driver_t *)stm32_usart_driver_create(mapping->huart);
  }
  return usart_drivers[id];
}
