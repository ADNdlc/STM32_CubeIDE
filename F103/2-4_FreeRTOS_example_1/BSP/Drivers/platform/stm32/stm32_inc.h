#ifndef BSP_DRIVERS_STM32_INC_H_
#define BSP_DRIVERS_STM32_INC_H_

#ifdef STM32H743xx
#include "stm32h7xx_hal.h"
#endif
#ifdef STM32F103xB
#include "stm32f1xx_hal.h"
#endif

#include "System\Sys\stm32_sys.h"

#include "Connectivity\gpio\stm32_gpio_driver.h"
#include "Connectivity\spi\stm32_spi_driver.h"
#include "Connectivity\i2c\stm32_i2c_driver.h"
#include "Connectivity\usart\stm32_usart_driver.h"
#include "Connectivity\timer\stm32_timer_driver.h"
#include "Connectivity\pwm\stm32_pwm_driver.h"


#endif /* BSP_DRIVERS_STM32_INC_H_ */
