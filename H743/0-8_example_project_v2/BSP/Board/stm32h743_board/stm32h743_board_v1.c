#include "dev_map_config.h"
#include "dev_map.h"
#if (STM32H743_BOARD_V1 == TARGET_BOARD)
#include "fmc.h"
#include "usart.h" // 仅在Board层包含底层的usart.h
#include "one_wire/stm32_one_wire_driver.h"

// USART 设备映射表定义
const usart_mapping_t usart_mappings[USART_MAX_DEVICES] = {
    [USART_ID_DEBUG] = {.resource = (void *)&huart1},
    [USART_ID_ESP8266] = {.resource = (void *)&huart2},
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
    [TH_SENSOR_ID_AMBIENT] = {},
};

#endif // STM32H743_BOARD_V1
