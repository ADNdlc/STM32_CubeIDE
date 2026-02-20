#include "dev_map.h"
#include "dev_map_config.h"

#if (STM32H743_BOARD_V1 == TARGET_BOARD)
#include "fmc.h"
#include "gpio/stm32_gpio_driver.h"
#include "gt9xxx_touch/gt9xxx_touch_driver.h"
#include "i2c/stm32_i2c_driver.h"
#include "main.h"
#include "one_wire/stm32_one_wire_driver.h"
#include "qspi/stm32_qspi_driver.h"
#include "quadspi.h"
#include "rtc/stm32_rtc_driver.h"
#include "tim.h"
#include "spi.h"
#include "spi/stm32_spi_driver.h"
#include "usart.h"

/*************
 * 总线配置表
 *************/

// GPIO 设备配置(所有)
static const stm32_gpio_config_t all_gpio_configs[GPIO_MAX_DEVICES] = {
    [TOUCH_RST] = {.pin = touch_RST_Pin, .port = touch_RST_GPIO_Port},
    [TOUCH_INT] = {.pin = touch_INT_Pin, .port = touch_INT_GPIO_Port},
    [GPIO_ID_LED0] = {.pin = LED0_Pin, .port = LED0_GPIO_Port},
    [GPIO_ID_LED1] = {.pin = LED1_Pin, .port = LED1_GPIO_Port},
};
// GPIO 逻辑号映射表
const gpio_mapping_t gpio_mappings[GPIO_MAX_DEVICES] = {
    [TOUCH_RST] = {.resource = (void *)&all_gpio_configs[TOUCH_RST]},
    [TOUCH_INT] = {.resource = (void *)&all_gpio_configs[TOUCH_INT]},
    [GPIO_ID_LED0] = {.resource = (void *)&all_gpio_configs[GPIO_ID_LED0]},
    [GPIO_ID_LED1] = {.resource = (void *)&all_gpio_configs[GPIO_ID_LED1]},
};

// USART 逻辑号映射表
const usart_mapping_t usart_mappings[USART_MAX_DEVICES] = {
    [USART_ID_DEBUG] = {.resource = (void *)&huart1},
    [USART_ID_ESP8266] = {.resource = (void *)&huart2},
};

// I2C 设备配置
static const stm32_i2c_soft_config_t touch_soft_i2c_bus = {
    // 软件i2c配置
    .scl_port = touch_SCL_GPIO_Port,
    .scl_pin = touch_SCL_Pin,
    .sda_port = touch_SDA_GPIO_Port,
    .sda_pin = touch_SDA_Pin,
    .delay_us = 0,
};
static const stm32_i2c_config_t all_i2c_configs[I2C_MAX_DEVICES] = {
    // 硬件i2c配置
    [I2C_BUS_SENSOR] = {.is_soft = 0, .resource.hi2c = &hi2c2},
    [I2C_BUS_TOUCH] = {.is_soft = 1,
                       .resource.soft_config =
                           (stm32_i2c_soft_config_t *)&touch_soft_i2c_bus},
};
// I2C 逻辑号映射表
const i2c_mapping_t i2c_mappings[I2C_MAX_DEVICES] = {
    [I2C_BUS_SENSOR] = {.resource = (void *)&all_i2c_configs[I2C_BUS_SENSOR]},
    [I2C_BUS_TOUCH] = {.resource = (void *)&all_i2c_configs[I2C_BUS_TOUCH]},
};

// SDRAM 逻辑号映射表
const sdram_mapping_t sdram_mappings[SDRAM_MAX_DEVICES] = {
    [SDRAM_MAIN] = {.resource = (void *)&hsdram1}};

// RTC 逻辑号映射表
const rtc_mapping_t rtc_mappings[RTC_MAX] = {
    [RTC_ID_INTERNAL] = {.resource = (void *)&hrtc},
};

// Timer 逻辑号映射表
const timer_mapping_t timer_mappings[TIMER_ID_MAX] = {
    [TIMER_ID_1] = {.resource = (void *)&htim6},
};

// One-Wire 设备配置
static const stm32_one_wire_config_t dht11_congfig = {
    .port = dht11_D_GPIO_Port, // GPIOB
    .pin = dht11_D_Pin,        // GPIO_PIN_11
};
// One-Wire 逻辑号映射表
const one_wire_mapping_t one_wire_mappings[ONE_WIRE_MAX_DEVICES] = {
    [ONE_WIRE_DHT11] = {.resource = (void *)&dht11_congfig},
};

// QSPI 逻辑号映射表
const qspi_mapping_t qspi_mappings[QSPI_MAX_DEVICES] = {
    [QSPI_ID_FLASH] = {.resource = (void *)&hqspi},
};

// SPI 逻辑号映射表
const spi_mapping_t spi_mappings[SPI_MAX_DEVICES] = {
    [SPI_ID_1] = {.resource = (void *)&hspi1},
};

/*************
 * 设备配置表
 *************/

// "温湿度传感器"逻辑号映射表
const th_sensor_mapping_t th_sensor_mappings[TH_SENSOR_MAX] = {
    [TH_SENSOR_ID_AMBIENT] = {.resource = (void *)ONE_WIRE_DHT11},
};

// "光照传感器"逻辑号映射表
const light_sensor_mapping_t light_sensor_mappings[LIGHT_SENSOR_MAX] = {
    [LIGHT_SENSOR_ID_AMBIENT] = {.resource = (void *)I2C_BUS_SENSOR},
};

// "触摸屏"总线配置
static const gt9xxx_config_t gt9xxx_config = {
    .i2c_id = I2C_BUS_TOUCH,
    .rst_gpio_id = TOUCH_RST,
    .int_gpio_id = TOUCH_INT,
    .addr_mode = GT9XXX_ADDR_0x14,
};
// "触摸屏"逻辑号映射表
const touch_mapping_t touch_mappings[TOUCH_MAX] = {
    [TOUCH_ID_UI] = {.resource = (void *)&gt9xxx_config},
};

// "NORflash设备表"
const norflash_mapping_t norflash_mappings[NOR_FLASH_MAX] = {
    [NOR_FLASH_SYS] =
        {
            .resource = (void *)QSPI_ID_FLASH,
            .manual_info = NULL,
        },
};

#endif // STM32H743_BOARD_V1
