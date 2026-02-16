#ifndef BSP_INIT_H_
#define BSP_INIT_H_

#include "uart_queue/uart_queue.h"
#include "usart_driver.h"


// 全局调试串口句柄和调试队列
extern usart_driver_t *g_debug_uart;
extern uart_queue_t *g_debug_queue;

// BSP 统一初始化接口
void bsp_init(void);

#endif /* BSP_INIT_H_ */
