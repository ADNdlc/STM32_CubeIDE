#include "device_mapping.h"
#include "spi.h"
#include <string.h>
// #include "quadspi.h" // Assuming this will exist or is manually added

// Extern handles if not in headers
extern SPI_HandleTypeDef hspi1;
// extern QSPI_HandleTypeDef hqspi;

int device_mapping_get_flash_config(const char *tag, flash_config_t *config) {
  if (!tag || !config)
    return -1;

  if (strcmp(tag, "FLASH_SPI_EXT") == 0) {
    config->type = FLASH_TYPE_SPI;
    config->handle = &hspi1;
    config->cs_port = GPIOA;     // Example, check schematic
    config->cs_pin = GPIO_PIN_4; // Example
    return 0;
  }
  /*
      if (strcmp(tag, "FLASH_QSPI_INT") == 0) {
          config->type = FLASH_TYPE_QSPI;
          // config->handle = &hqspi;
          config->handle = NULL; // Warning: No QSPI handle yet
          return 0;
      }
  */
  return -1;
}
