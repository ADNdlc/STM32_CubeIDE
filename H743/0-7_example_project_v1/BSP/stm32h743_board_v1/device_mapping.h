#ifndef BSP_STM32H743_BOARD_V1_DEVICE_MAPPING_H_
#define BSP_STM32H743_BOARD_V1_DEVICE_MAPPING_H_

#include "stm32h7xx_hal.h"

typedef enum {
  FLASH_TYPE_NONE = 0,
  FLASH_TYPE_SPI,
  FLASH_TYPE_QSPI
} flash_type_t;

typedef struct {
  flash_type_t type;
  void *handle; // SPI_HandleTypeDef* or QSPI_HandleTypeDef*
  GPIO_TypeDef *cs_port;
  uint16_t cs_pin;
} flash_config_t;

/**
 * @brief Get Flash Configuration by TAG
 * @param tag String identifier (e.g. "FLASH_EXT")
 * @param config Pointer to config struct to fill
 * @return 0 on success, -1 if not found
 */
int device_mapping_get_flash_config(const char *tag, flash_config_t *config);

#endif /* BSP_STM32H743_BOARD_V1_DEVICE_MAPPING_H_ */
