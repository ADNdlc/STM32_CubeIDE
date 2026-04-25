#ifndef BSP_DRIVERS_COMMPONENT_INC_H_
#define BSP_DRIVERS_COMMPONENT_INC_H_

#ifdef STM32H743xx
#include "stm32h7xx_hal.h"
#endif
#ifdef STM32F103xB
#include "stm32f1xx_hal.h"
#endif

#include "utils\ring_buffer\ring_buffer.h"
#include "uart_queue\uart_queue.h"
#include "logger\elog_init.h"


#endif /* BSP_DRIVERS_COMMPONENT_INC_H_ */
