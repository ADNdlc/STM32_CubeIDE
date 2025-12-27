#include "flash_test.h"
#include "flash_factory.h"
#include "flash_handler.h"
#include "raw_strategy.h"
#include "sys.h" // for logging/printf if available
#include "elog.h"
#include <stdio.h>
#include <string.h>

#define LOG_TAG "Flash Test"

void flash_integration_test(void)
{
  log_d("Starting flash integration test");
  
  // 1. Initialize Handler
  log_d("Initializing flash handler");
  flash_handler_init();
  log_d("Flash handler initialized");

  // 2. Create Device via Factory (using ID directly)
  // Was: "FLASH_SPI_EXT" -> Now: FLASH_EXT_SPI (enum)
  log_d("Getting flash device via factory with ID: FLASH_EXT_QSPI");
  block_device_t *dev = flash_factory_get(FLASH_EXT_QSPI);
  if (!dev)
  {
    log_e("Flash Factory Failed - returned NULL device");
    return;
  }
  log_i("Flash device successfully created from factory");

  // 3. Create Strategy (Raw)
  log_d("Creating raw strategy");
  flash_strategy_t *strat = raw_strategy_create();
  if (!strat)
  {
    log_e("Strategy Create Failed");
    return;
  }
  log_i("Raw strategy created successfully");

  // 4. Register Device
  log_d("Registering device with handler");
  if (flash_handler_register("/ext", dev, strat) != 0)
  {
    log_e("Handler Register Failed");
    return;
  }
  log_i("Device registered successfully");

  // 5. Test Operations
  uint8_t write_buf[16] = "Hello Flash 123";
  uint8_t read_buf[16] = {0};

  log_d("Starting test operations...");
  // Erase
  log_d("Erasing sector at address 0");
  BLOCK_DEV_ERASE(dev, 0, 4096);

  // Write
  log_d("Writing test data to address 0");
  if (flash_handler_write("/ext", 0, write_buf, 16) != 0)
  {
    log_e("Write Failed");
  }
  else
  {
    log_i("Write Success");
  }

  // Read
  log_d("Reading data from address 0");
  if (flash_handler_read("/ext", 0, read_buf, 16) != 0)
  {
    log_e("Read Failed");
  }
  else
  {
    log_i("Read Success");
    if (memcmp(write_buf, read_buf, 16) == 0)
    {
      log_i("Data Match!");
    }
    else
    {
      log_e("Data Mismatch: %s vs %s", write_buf, read_buf);
    }
  }
  log_d("Flash integration test completed");
}
