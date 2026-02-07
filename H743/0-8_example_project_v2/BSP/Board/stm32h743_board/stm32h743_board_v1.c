#include "dev_map_config.h"

#if (STM32H743_BOARD_V1 == TARGET_BOARD)

#include "dev_map.h"
#include "usart.h" // 仅在Board层包含底层的usart.h
// 统一硬件资源映射
const usart_mapping_t usart_mappings[USART_MAX_DEVICES] = {
    [USART_ID_DEBUG] = {.resource = (void *)&huart1},
    [USART_ID_ESP8266] = {.resource = (void *)&huart2},
};

#endif // STM32H743_BOARD_V1
