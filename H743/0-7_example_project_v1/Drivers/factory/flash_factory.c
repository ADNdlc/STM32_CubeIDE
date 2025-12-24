#include "flash_factory.h"
#include "device_mapping.h"
#include "qspi_hal.h"
#include "spi_hal.h"
#include "stm32_qspi.h"
#include "stm32_spi.h"
#include "w25q_qspi_adapter.h"
#include "w25q_spi_adapter.h"
#include "w25qxx.h"
#include <stddef.h>


block_device_t *flash_factory_create(const char *tag) {
  flash_config_t config;
  if (device_mapping_get_flash_config(tag, &config) != 0) {
    return NULL;
  }

  if (config.type == FLASH_TYPE_SPI) {
    // 1. Create Platform Driver
    spi_driver_t *stm_spi = stm32_spi_create((SPI_HandleTypeDef *)config.handle,
                                             config.cs_port, config.cs_pin);
    if (!stm_spi)
      return NULL;

    // 2. Create HAL
    spi_hal_t *hal = spi_hal_create(stm_spi);
    if (!hal) {
      // cleanup stm_spi
      stm32_spi_destroy(stm_spi);
      return NULL;
    }

    // 3. Create Adapter
    w25q_adapter_t *adapter = w25q_spi_adapter_create(hal);
    if (!adapter) {
      // cleanup hal
      spi_hal_destroy(hal);
      stm32_spi_destroy(stm_spi);
      return NULL;
    }

    // 4. Create Device
    block_device_t *dev = w25qxx_create(adapter);
    if (!dev) {
      // cleanup adapter
      w25q_spi_adapter_destroy(adapter);
      spi_hal_destroy(hal);
      stm32_spi_destroy(stm_spi);
      return NULL;
    }

    return dev;

  } else if (config.type == FLASH_TYPE_QSPI) {
    // 1. Create Platform Driver
    qspi_driver_t *stm_qspi =
        stm32_qspi_create((QSPI_HandleTypeDef *)config.handle);
    if (!stm_qspi)
      return NULL;

    // 2. Create HAL
    qspi_hal_t *hal = qspi_hal_create(stm_qspi);
    if (!hal) {
      stm32_qspi_destroy(stm_qspi);
      return NULL;
    }

    // 3. Create Adapter
    w25q_adapter_t *adapter = w25q_qspi_adapter_create(hal);
    if (!adapter) {
      qspi_hal_destroy(hal);
      stm32_qspi_destroy(stm_qspi);
      return NULL;
    }

    // 4. Create Device
    block_device_t *dev = w25qxx_create(adapter);
    if (!dev) {
      w25q_qspi_adapter_destroy(adapter);
      qspi_hal_destroy(hal);
      stm32_qspi_destroy(stm_qspi);
      return NULL;
    }

    return dev;
  }

  return NULL;
}
