#include "dev_map.h"
#include "dev_map_config.h"

#if (STM32F103_BOARD_V1 == TARGET_BOARD)
#include "device_inc.h"
#include "factory_inc.h"
#include "interface_inc.h"
#include "stm32_inc.h"

#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

/*************
 * 总线配置表
 *************/

// GPIO 设备配置
static const stm32_gpio_config_t all_gpio_configs[GPIO_MAX_DEVICES] = {
    [GPIO_ID_LED0] = {.pin = LED_0_Pin, .port = LED_0_GPIO_Port},
};

// GPIO 逻辑号映射表
const gpio_mapping_t gpio_mappings[GPIO_MAX_DEVICES] = {
    [GPIO_ID_LED0] = {.resource = (void *)&all_gpio_configs[GPIO_ID_LED0]},
};

// USART 逻辑号映射表
const usart_mapping_t usart_mappings[USART_MAX_DEVICES] = {
    [USART_ID_DEBUG] = {.resource = (void *)&huart1},
};

// 硬件i2c配置
static const stm32_i2c_config_t all_i2c_configs[I2C_MAX_DEVICES] = {
    [I2C_M0_OUT] = {.is_soft = 0, .resource.hi2c = &hi2c2},
    [I2C_M1_OUT] = {.is_soft = 0, .resource.hi2c = &hi2c1},
};

// I2C 逻辑号映射表
const i2c_mapping_t i2c_mappings[I2C_MAX_DEVICES] = {
    [I2C_M0_OUT] = {.resource = (void *)&all_i2c_configs[I2C_M0_OUT]},
    [I2C_M1_OUT] = {.resource = (void *)&all_i2c_configs[I2C_M1_OUT]},
};

// Timer 逻辑号映射表
const timer_mapping_t timer_mappings[TIMER_ID_MAX] = {
    [TIMER_ID_1] = {.resource = (void *)&htim1},
};

// PWM 逻辑号映射
static const stm32_pwm_config_t pwm_configs[PWM_ID_MAX] = {
    [M0_IN_1] = {&htim2, TIM_CHANNEL_1},
    [M0_IN_2] = {&htim2, TIM_CHANNEL_2},
    [M0_IN_3] = {&htim2, TIM_CHANNEL_3},
    [M1_IN_1] = {&htim2, TIM_CHANNEL_4},
    [M1_IN_2] = {&htim3, TIM_CHANNEL_1},
    [M1_IN_3] = {&htim3, TIM_CHANNEL_2},
};
const pwm_mapping_t pwm_mappings[PWM_ID_MAX] = {
    [M0_IN_1] = {.resource = (void *)&pwm_configs[M0_IN_1]},
    [M0_IN_2] = {.resource = (void *)&pwm_configs[M0_IN_2]},
    [M0_IN_3] = {.resource = (void *)&pwm_configs[M0_IN_3]},
    [M1_IN_1] = {.resource = (void *)&pwm_configs[M1_IN_1]},
    [M1_IN_2] = {.resource = (void *)&pwm_configs[M1_IN_2]},
    [M1_IN_3] = {.resource = (void *)&pwm_configs[M1_IN_3]},
};

// SPI 逻辑号映射表
const spi_mapping_t spi_mappings[SPI_MAX_DEVICES] = {
    [SPI_ID_1] = {.resource = (void *)&hspi2},
};

/*************
 * 设备配置表
 *************/

// 绝对值编码器逻辑号映射表
const absolute_encoder_mapping_t absolute_encoder_mappings[ENCODER_ID_MAX] = {
    [ENCODER_ID_M0] = {.resource = (void *)I2C_M0_OUT},
    [ENCODER_ID_M0] = {.resource = (void *)I2C_M1_OUT},
};

#endif // STM32F103_BOARD_V1
