/*
 * sep_uart.h
 *
 *  Created on: Aug 9, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_AT_UART_H_
#define BSP_ESP_AT_AT_UART_H_

#include "usart.h"

#define	LOOP_BUF_SIZE	1024	//模块回复消息的缓冲区大小

void ATuart_driver_init(UART_HandleTypeDef* uart_port);

size_t ATuart_get_readable_bytes(void);
size_t ATuart_read(uint8_t *buffer, size_t len);
void ATuart_send_string(const char* str);

void ATuart_RxCpltHandle(UART_HandleTypeDef *ituart);

#endif
