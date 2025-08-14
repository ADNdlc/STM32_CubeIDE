/*
 * esp_sys.c
 *
 *  Created on: Aug 13, 2025
 *      Author: 12114
 */
#include "esp_sys.h"


uint8_t temp_read_buffer[128]; // 从驱动读取的临时缓冲区

void ESP_AT_sys_init(UART_HandleTypeDef* uart_port){
	ATuart_driver_init(uart_port);//绑定AT串口
	AT_parser_init();		// 初始化行解析
	AT_controller_init(); // 初始化控制器
}


void ESP_AT_sys_handle(void){
    if (ATuart_get_readable_bytes() > 0){
        size_t len = ATuart_read(temp_read_buffer, sizeof(temp_read_buffer));
        if (len > 0)
        {
            // 数据流向: uart_driver(接收) -> parser(分行) -> dispatcher(调用) -> controller(处理)
            AT_parser_input(temp_read_buffer, len);
        }
    }
    AT_controller_process();
}


