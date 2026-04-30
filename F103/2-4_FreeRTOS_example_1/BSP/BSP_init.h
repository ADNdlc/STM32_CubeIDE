#ifndef BSP_INIT_H_
#define BSP_INIT_H_



#include "uart_queue/uart_queue.h"
#include "motor_control\motor_control.h"

extern usart_driver_t *g_debug_uart;
extern uart_queue_t *g_debug_queue;
extern motor_control_t *Motor_1_control;
extern motor_control_t *Motor_2_control;
extern absolute_encoder_driver_t *g_encoder_m0;
extern absolute_encoder_driver_t *g_encoder_m1;

void bsp_init(void);

#endif /* BSP_INIT_H_ */
