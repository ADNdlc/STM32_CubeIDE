#include "dev_map.h"
#include "dev_map_config.h"

#if (STM32F103_BOARD_V1 == TARGET_BOARD)
#include "factory_inc.h"
#include "interface_inc.h"
#include "device_inc.h"
#include "stm32_inc.h"

#include "usart.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"

/*************
 * 总线配置表
 *************/

// GPIO 设备配置
static const stm32_gpio_config_t all_gpio_configs[GPIO_MAX_DEVICES] = {
    [GPIO_ID_LED0] = {.pin = GPIO_PIN_13, .port = GPIOC},
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
    [I2C_BUS_OLED] = {.is_soft = 0, .resource.hi2c = &hi2c1},
};

// I2C 逻辑号映射表
const i2c_mapping_t i2c_mappings[I2C_MAX_DEVICES] = {
    [I2C_BUS_OLED] = {.resource = (void *)&all_i2c_configs[I2C_BUS_OLED]},
};

// Timer 逻辑号映射表
const timer_mapping_t timer_mappings[TIMER_ID_MAX] = {
    [TIMER_ID_1] = {.resource = (void *)&htim1},
};

// SPI 逻辑号映射表
const spi_mapping_t spi_mappings[SPI_MAX_DEVICES] = {
    [SPI_ID_1] = {.resource = (void *)&hspi2},
};

/*************
 * 设备配置表
 *************/
uint8_t oled_buffer[(64 / 8) * 128];
// "OLED"设备配置表
oled_config_t oled_config = {
    .ops = &ssd1306_ops,
    .bus_type = OLED_BUS_I2C,
    .oled_i2c_config ={
            .i2c_id = I2C_BUS_OLED,
            .dev_addr = 0x78, // SSD1306 常用地址
        },
    .width = 128,
    .height = 64,
    .col_offset = 0,
    .buffer = oled_buffer,
};

// "OLED"逻辑号映射表
const oled_mapping_t oled_mappings[OLED_ID_MAX] = {
    [OLED_ID_MAIN] = {.resource = (void *)&oled_config},
};

#endif // STM32F103_BOARD_V1
