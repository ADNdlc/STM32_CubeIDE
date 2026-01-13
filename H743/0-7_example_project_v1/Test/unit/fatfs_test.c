#include "all_tests_config.h"
#if _fatfs_test_
#include "elog.h"
#include "factory/flash_factory.h"
#include "strategy/fatfs_strategy.h"
#include "sys.h"
#include <string.h>

#define LOG_TAG "FatFSTest"

void fatfs_test_run(void) {
  log_i("Starting FatFS Integration Test...");

  // 1. 获取 SD 卡设备
  block_device_t *sd_dev = flash_factory_get(FLASH_EXT_SDCARD);
  if (!sd_dev) {
    log_e("Failed to get SD card device from factory");
    return;
  }

  // 2. 创建 FatFS 策略 (使用驱动器 0)
  fatfs_strategy_config_t config = {.pdrv = 0};
  flash_strategy_t *strategy = fatfs_strategy_create(&config);
  if (!strategy) {
    log_e("Failed to create FatFS strategy");
    return;
  }

  // 3. 挂载
  log_i("Mounting FatFS on SD card...");
  if (FLASH_STRATEGY_MOUNT(strategy, sd_dev) != 0) {
    log_e("FatFS mount failed");
    fatfs_strategy_destroy(strategy);
    return;
  }

  // 4. 测试文件读写
  const char *test_path = "0:/hello_stm32.txt";
  const char *test_data = "Hello from STM32H7 FatFS Integration!";
  size_t data_len = strlen(test_data);

  log_i("Writing test data to file: %s", test_path);
  if (FLASH_STRATEGY_WRITE(strategy, test_path, 0, (const uint8_t *)test_data,
                           data_len) != 0) {
    log_e("File write failed");
  } else {
    log_i("Reading back test data...");
    uint8_t read_buf[64] = {0};
    if (FLASH_STRATEGY_READ(strategy, test_path, 0, read_buf, data_len) != 0) {
      log_e("File read failed");
    } else {
      log_i("Read data: %s", (char *)read_buf);
      if (memcmp(test_data, read_buf, data_len) == 0) {
        log_i("FatFS Integration Test PASSED!");
      } else {
        log_e("Data mismatch! Expected: %s, Got: %s", test_data,
              (char *)read_buf);
      }
    }
  }

  // 5. 卸载和清理
  FLASH_STRATEGY_UNMOUNT(strategy);
  fatfs_strategy_destroy(strategy);

  log_i("FatFS Test Completed.");
}

#endif
