#include "flash_test.h"
#include "elog.h"
#include "flash_factory.h"
#include "flash_handler.h"
#include "raw_strategy.h"
#include "sys.h" // for logging/printf if available
#include <stdio.h>
#include <string.h>


#define LOG_TAG "Flash Test"

static int test_at_address(block_device_t *dev, uint32_t addr,
                           const char *msg) {
  uint8_t write_buf[32];
  uint8_t read_buf[32];

  snprintf((char *)write_buf, sizeof(write_buf), "Data at 0x%08X: %s",
           (unsigned int)addr, msg);
  memset(read_buf, 0, sizeof(read_buf));

  log_i("--- Testing Address 0x%08X ---", (unsigned int)addr);

  // Erase sector
  log_d("Erasing sector...");
  if (BLOCK_DEV_ERASE(dev, addr, 4096) != 0) {
    log_e("Erase failed at 0x%08X", (unsigned int)addr);
    return -1;
  }

  // Write
  log_d("Writing: %s", (char *)write_buf);
  if (flash_handler_write("/ext", addr, write_buf, sizeof(write_buf)) != 0) {
    log_e("Write failed at 0x%08X", (unsigned int)addr);
    return -1;
  }

  // Read
  if (flash_handler_read("/ext", addr, read_buf, sizeof(read_buf)) != 0) {
    log_e("Read failed at 0x%08X", (unsigned int)addr);
    return -1;
  }
  log_d("Read: %s", (char *)read_buf);

  // Verify
  if (memcmp(write_buf, read_buf, sizeof(write_buf)) == 0) {
    log_i("Verification SUCCESS at 0x%08X", (unsigned int)addr);
    return 0;
  } else {
    log_e("Verification FAILED at 0x%08X", (unsigned int)addr);
    log_e("Expected: %s", (char *)write_buf);
    log_e("Got:      %s", (char *)read_buf);
    return -1;
  }
}

void flash_integration_test(void) {
  log_i("Starting enhanced flash integration test");

  // 1. Initialize Handler
  flash_handler_init();

  // 2. Create Device via Factory
  block_device_t *dev = flash_factory_get(FLASH_EXT_QSPI);
  if (!dev) {
    log_e("Flash Factory Failed");
    return;
  }

  // 3. Create Strategy (Raw)
  flash_strategy_t *strat = raw_strategy_create();
  if (!strat) {
    log_e("Strategy Create Failed");
    return;
  }

  // 4. Register Device
  if (flash_handler_register("/ext", dev, strat) != 0) {
    log_e("Handler Register Failed");
    return;
  }

  // 5. Run tests at various addresses
  log_i("Starting multi-address verification...");

  int failures = 0;
  // Test Case 1: Start of flash
  failures += test_at_address(dev, 0, "Origin");

  // Test Case 2: Some offset
  failures += test_at_address(dev, 4096, "First Sector Offset");

  // Test Case 3: Near 16MB boundary (24-bit limit)
  failures += test_at_address(dev, 0xFFF000, "Near 16MB Boundary");

  // Test Case 4: Across 16MB boundary (Requires 4-byte address)
  failures += test_at_address(dev, 0x1000000, "First Byte of 4-byte Range");
  failures += test_at_address(dev, 0x1100000, "Into 17MB Range");

  // Test Case 5: Near end of 32MB range
  failures += test_at_address(dev, 31 * 1024 * 1024, "Near End of 32MB");

  if (failures == 0) {
    log_i("All multi-address tests PASSED!");
  } else {
    log_e("%d test cases FAILED!", failures);
  }

  log_i("Flash integration test completed");
}
