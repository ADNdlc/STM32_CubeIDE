#include "flash_factory.h"
#include "device_mapping.h"
#include "elog.h"
#include "mt29f4g08/mt29f4g08.h"
#include "fmc/mt29f_fmc_adapter.h"
#include "sdmmc/stm32_sdmmc_adapter.h"
#include "qspi_driver.h"
#include "qspi_factory.h"
#include "sdcard/sdcard.h"
#include "spi_driver.h"
#include "spi_factory.h"
#include "w25qxx/transport/w25q_qspi_adapter.h"
#include "w25qxx/transport/w25q_spi_adapter.h"
#include "w25qxx/w25qxx.h"
#include <stddef.h>


#define LOG_TAG "Flash_Factory"

// Static storage for devices using ID
static block_device_t *flash_devices[FLASH_MAX_DEVICES] = {NULL};

block_device_t *flash_factory_get(flash_device_id_t id) {
  log_d("Flash factory get called with ID: %d", id);

  if (id >= FLASH_MAX_DEVICES) {
    log_e("Invalid flash device ID: %d", id);
    return NULL;
  }

  if (flash_devices[id] == NULL) {
    log_d("Creating new flash device for ID: %d", id);
    const flash_mapping_t *mapping = &flash_mappings[id];

    if (mapping->type == FLASH_TYPE_SPI) {
      log_d("Creating SPI flash device, SPI ID: %d", mapping->spi_id);
      // 1. Get SPI Driver from Factory (Dependency Injection)
      spi_driver_t *spi_drv = spi_driver_get(mapping->spi_id);
      if (!spi_drv) {
        log_e("Failed to get SPI driver for ID: %d", mapping->spi_id);
        return NULL;
      }
      log_i("Successfully got SPI driver");

      // 2. Create adapter directly with SPI driver (no HAL needed)
      w25q_adapter_t *adapter = w25q_spi_adapter_create(spi_drv);
      if (!adapter) {
        log_e("Failed to create SPI adapter");
        return NULL;
      }
      log_i("Successfully created SPI adapter");

      // 3. Create device with adapter
      flash_devices[id] = w25qxx_create(adapter);
      if (!flash_devices[id]) {
        log_e("Failed to create W25QXX device with SPI adapter");
        w25q_spi_adapter_destroy(adapter);
        return NULL;
      }
      log_i("Successfully created W25QXX device with SPI adapter");

    } else if (mapping->type == FLASH_TYPE_QSPI) {
      log_d("Creating QSPI flash device, QSPI ID: %d", mapping->qspi_id);
      qspi_driver_t *qspi_drv = qspi_driver_get(mapping->qspi_id);
      if (!qspi_drv) {
        log_e("Failed to get QSPI driver for ID: %d", mapping->qspi_id);
        return NULL;
      }
      log_i("Successfully got QSPI driver");

      // Create adapter directly with QSPI driver (no HAL needed)
      w25q_adapter_t *adapter = w25q_qspi_adapter_create(qspi_drv);
      if (!adapter) {
        log_e("Failed to create QSPI adapter");
        return NULL;
      }
      log_i("Successfully created QSPI adapter");

      flash_devices[id] = w25qxx_create(adapter);
      if (!flash_devices[id]) {
        log_e("Failed to create W25QXX device with QSPI adapter");
        w25q_qspi_adapter_destroy(adapter);
        return NULL;
      }
      log_i("Successfully created W25QXX device with QSPI adapter");

    } else if (mapping->type == FLASH_TYPE_NAND) {
      log_d("Creating NAND flash device, NAND ID: %d", mapping->hnand);

      // 1. Get NAND Driver handle directly from mapping
      NAND_HandleTypeDef *nand_drv = mapping->hnand;
      if (!nand_drv) {
        log_e("Failed to get NAND driver for ID: %d", mapping->hnand);
        return NULL;
      }

      // 2. Create adapter directly with NAND driver (no HAL needed)
      mt29f_adapter_t *adapter = mt29f_fmc_adapter_create(nand_drv);
      if (!adapter) {
        log_e("Failed to create NAND adapter");
        return NULL;
      }
      log_i("Successfully created NAND adapter");

      // 3. Create device with adapter
      flash_devices[id] = mt29f4g08_create(adapter);
      if (!flash_devices[id]) {
        log_e("Failed to create MT29F4G08 device with NAND adapter");
        // Note: Currently there's no destroy function for mt29f_fmc_adapter,
        // assuming it's not needed
        return NULL;
      }
      log_i("Successfully created MT29F4G08 device with NAND adapter");
    } else if (mapping->type == FLASH_TYPE_SD) {
      log_d("Creating SD card device");

      SD_HandleTypeDef *hsd = mapping->hsd;
      if (!hsd) {
        log_e("Failed to get SD handle for ID: %d", id);
        return NULL;
      }

      sdcard_adapter_t *adapter = stm32_sdmmc_adapter_create(hsd);
      if (!adapter) {
        log_e("Failed to create SD adapter");
        return NULL;
      }
      log_i("Successfully created SD adapter");

      flash_devices[id] = sdcard_create(adapter);
      if (!flash_devices[id]) {
        log_e("Failed to create SD card device");
        stm32_sdmmc_adapter_destroy(adapter);
        return NULL;
      }
      log_i("Successfully created SD card device");
    } else {
      log_e("Unknown flash type: %d", mapping->type);
      return NULL;
    }
  } else {
    log_d("Returning existing flash device for ID: %d", id);
  }

  log_d("Flash factory get returning device: %p", flash_devices[id]);
  return flash_devices[id];
}
