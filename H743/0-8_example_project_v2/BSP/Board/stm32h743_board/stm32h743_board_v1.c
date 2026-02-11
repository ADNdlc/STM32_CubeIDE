#include "dev_map.h"
#include "dev_map_config.h"

#if (STM32H743_BOARD_V1 == TARGET_BOARD)
#include "fmc.h"
#include "i2c.h"
#include "usart.h"
#include "one_wire/stm32_one_wire_driver.h"

// USART 设备映射表定义
const usart_mapping_t usart_mappings[USART_MAX_DEVICES] = {
    [USART_ID_DEBUG] = {.resource = (void *)&huart1},
    [USART_ID_ESP8266] = {.resource = (void *)&huart2},
};

// I2C 设备映射表定义
const i2c_mapping_t i2c_mappings[I2C_MAX_DEVICES] = {
    [I2C_BUS_SENSOR] = {.resource = (void *)&hi2c2},
};

// SDRAM 设备映射表定义
const sdram_mapping_t sdram_mappings[SDRAM_MAX_DEVICES] = {
    [SDRAM_MAIN] = {.resource = (void *)&hsdram1}};

// One-Wire 资源
static const stm32_one_wire_config_t dht11_congfig = {
    .port = dht11_D_GPIO_Port, // GPIOB
    .pin = dht11_D_Pin,        // GPIO_PIN_11
};
const one_wire_mapping_t one_wire_mappings[ONE_WIRE_MAX_DEVICES] = {
    [ONE_WIRE_DHT11] = {.resource = (void *)&dht11_congfig},
};

// 温湿度传感器驱动映射表
const th_sensor_mapping_t th_sensor_mappings[TH_SENSOR_MAX] = {
    [TH_SENSOR_ID_AMBIENT] = {.resource = (void *)ONE_WIRE_DHT11},
};

// 光照传感器驱动映射表
const light_sensor_mapping_t light_sensor_mappings[LIGHT_SENSOR_MAX] = {
    [LIGHT_SENSOR_ID_AMBIENT] = {.resource = (void *)I2C_BUS_SENSOR},
};

#endif // STM32H743_BOARD_V1
