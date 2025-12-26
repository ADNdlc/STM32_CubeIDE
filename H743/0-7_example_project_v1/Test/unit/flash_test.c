#include "flash_test.h"
#include "flash_factory.h"
#include "flash_handler.h"
#include "raw_strategy.h"
#include "sys.h" // for logging/printf if available
#include <stdio.h>
#include <string.h>


void flash_integration_test(void) {
  // 1. Initialize Handler
  flash_handler_init();

  // 2. Create Device via Factory (using ID directly)
  // Was: "FLASH_SPI_EXT" -> Now: FLASH_EXT_SPI (enum)
  block_device_t *dev = flash_factory_get(FLASH_EXT_QSPI);
  if (!dev) {
    printf("Flash Factory Failed\n");
    return;
  }

  // 3. Create Strategy (Raw)
  flash_strategy_t *strat = raw_strategy_create();
  if (!strat) {
    printf("Strategy Create Failed\n");
    return;
  }

  // 4. Register Device
  if (flash_handler_register("/ext", dev, strat) != 0) {
    printf("Handler Register Failed\n");
    return;
  }

  // 5. Test Operations
  uint8_t write_buf[16] = "Hello Flash 123";
  uint8_t read_buf[16] = {0};

  // Erase
  BLOCK_DEV_ERASE(dev, 0, 4096);

  // Write
  if (flash_handler_write("/ext", 0, write_buf, 16) != 0) {
    printf("Write Failed\n");
  } else {
    printf("Write Success\n");
  }

  // Read
  if (flash_handler_read("/ext", 0, read_buf, 16) != 0) {
    printf("Read Failed\n");
  } else {
    printf("Read Success\n");
    if (memcmp(write_buf, read_buf, 16) == 0) {
      printf("Data Match!\n");
    } else {
      printf("Data Mismatch: %s vs %s\n", write_buf, read_buf);
    }
  }
}
