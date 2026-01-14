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
  const char *test_path = "0:/stm32_test.txt";
  const char *test_str =
      "Hello from STM32H7 FatFS Integration with Heap Buffers!";
  size_t data_len = strlen(test_str);

  // 为 DMA 使用堆内存 (AXI SRAM)，避免 DTCM/栈问题
  uint8_t *test_buf = (uint8_t *)sys_malloc(SYS_MEM_INTERNAL, 512);
  uint8_t *read_buf = (uint8_t *)sys_malloc(SYS_MEM_INTERNAL, 512);

  if (!test_buf || !read_buf) {
    log_e("Failed to allocate test buffers on heap");
    if (test_buf)
      sys_free(SYS_MEM_INTERNAL, test_buf);
    if (read_buf)
      sys_free(SYS_MEM_INTERNAL, read_buf);
    FLASH_STRATEGY_UNMOUNT(strategy);
    fatfs_strategy_destroy(strategy);
    return;
  }

  memset(test_buf, 0, 512);
  memcpy(test_buf, test_str, data_len);
  memset(read_buf, 0, 512);

  log_i("Writing test data to file: %s", test_path);
  if (FLASH_STRATEGY_WRITE(strategy, test_path, 0, test_buf, data_len) != 0) {
    log_e("File write failed");
  } else {
    log_i("File write successful! Reading back test data...");
    if (FLASH_STRATEGY_READ(strategy, test_path, 0, read_buf, data_len) != 0) {
      log_e("File read failed");
    } else {
      log_i("Read data: %s", (char *)read_buf);
      if (memcmp(test_buf, read_buf, data_len) == 0) {
        log_i("FatFS Integration Test PASSED!");
      } else {
        log_e("Data mismatch!");
      }
    }
  }

  // 释放资源
  sys_free(SYS_MEM_INTERNAL, test_buf);
  sys_free(SYS_MEM_INTERNAL, read_buf);

  // 5. 卸载和清理
  FLASH_STRATEGY_UNMOUNT(strategy);
  fatfs_strategy_destroy(strategy);

  log_i("FatFS Test Completed.");
}

#endif
