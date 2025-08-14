/*
 * esp_sys.h
 *
 *  Created on: Aug 13, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_ESP_APP_ESP_SYS_ESP_SYS_H_
#define BSP_ESP_AT_ESP_APP_ESP_SYS_ESP_SYS_H_

#include "usart.h"
#include "esp_at/at_uart.h"
#include "esp_at/at_parser.h"
#include "esp_at/at_dispatcher.h"
#include "esp_at/at_controller.h"

void ESP_AT_sys_init(UART_HandleTypeDef* uart_port);
void ESP_AT_sys_handle(void);

#endif /* BSP_ESP_AT_ESP_APP_ESP_SYS_ESP_SYS_H_ */
