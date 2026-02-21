#include "dev_map.h"
#include "dev_map_config.h"

#if (STM32H743_BOARD_V1 == TARGET_BOARD)
#include "usart.h"
#include "gpio/stm32_gpio_driver.h"
#include "i2c/stm32_i2c_driver.h"
#include "main.h"
#include "rtc/stm32_rtc_driver.h"
#include "spi.h"
#include "spi/stm32_spi_driver.h"
#include "tim.h"


/*************
 * 总线配置表
 *************/

// GPIO 设备配置(所有)
static const stm32_gpio_config_t all_gpio_configs[GPIO_MAX_DEVICES] = {
    [GPIO_ID_LED0] = {.pin = LED0_Pin, .port = LED0_GPIO_Port},
};
// GPIO 逻辑号映射表
const gpio_mapping_t gpio_mappings[GPIO_MAX_DEVICES] = {
    [GPIO_ID_LED0] = {.resource = (void *)&all_gpio_configs[GPIO_ID_LED0]},
};

// USART 逻辑号映射表
const usart_mapping_t usart_mappings[USART_MAX_DEVICES] = {
    [USART_ID_DEBUG] = {.resource = (void *)&huart1},
};


static const stm32_i2c_config_t all_i2c_configs[I2C_MAX_DEVICES] = {
    // 硬件i2c配置
    [I2C_BUS_SENSOR] = {.is_soft = 0, .resource.hi2c = &hi2c1},
};
// I2C 逻辑号映射表
const i2c_mapping_t i2c_mappings[I2C_MAX_DEVICES] = {
    [I2C_BUS_SENSOR] = {.resource = (void *)&all_i2c_configs[I2C_BUS_SENSOR]},
};

// RTC 逻辑号映射表
const rtc_mapping_t rtc_mappings[RTC_MAX] = {
    [RTC_ID_INTERNAL] = {.resource = (void *)&hrtc},
};

// Timer 逻辑号映射表
const timer_mapping_t timer_mappings[TIMER_ID_MAX] = {
    [TIMER_ID_1] = {.resource = (void *)&htim1},
};

// SPI 逻辑号映射表
const spi_mapping_t spi_mappings[SPI_MAX_DEVICES] = {
    [SPI_ID_1] = {.resource = (void *)&hspi1},
};

/*************
 * 设备配置表
 *************/
// "光照传感器"逻辑号映射表
const light_sensor_mapping_t light_sensor_mappings[LIGHT_SENSOR_MAX] = {
    [LIGHT_SENSOR_ID_AMBIENT] = {.resource = (void *)I2C_BUS_SENSOR},
};


#endif // STM32H743_BOARD_V1
