#include "dev_map_config.h"

#if (STM32H743_BOARD_V1 == TARGET_BOARD)

#include "dev_map.h"
#include "usart.h" // 仅在Board层包含底层的usart.h
#include "fmc.h"

// USART 设备映射表定义
const usart_mapping_t usart_mappings[USART_MAX_DEVICES] = {
    [USART_ID_DEBUG] = {.resource = (void *)&huart1},
    [USART_ID_ESP8266] = {.resource = (void *)&huart2},
};

// SDRAM 设备映射表定义
const sdram_mapping_t sdram_mappings[SDRAM_MAX_DEVICES] = {
    [SDRAM_MAIN] = {.resource = (void *)&hsdram1}};

#endif // STM32H743_BOARD_V1
