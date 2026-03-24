#include "all_tests_config.h"
#if _fatfs_test_
#include "elog.h"
#include "factory/flash_factory.h"
#include "project_cfg.h"
#include "strategy/fatfs_strategy.h"
#include "strategy/lfs_strategy.h"
#include "sys.h"
#include <string.h>

#define LOG_TAG "FatFSTest"

#include "flash_handler.h"
#define LOG_TAG "FatFSTest"

static block_device_t *g_sd_dev = NULL;
static flash_strategy_t *g_fat_strat = NULL;

static void base_test(void) {
  log_i("Starting FatFS Integration Test...");

  // 1. 获取存储设备
  g_sd_dev = flash_factory_get(SYS_FLASH_DEV);
  if (!g_sd_dev) {
    log_e("Failed to get storage device (%d) from factory", SYS_FLASH_DEV);
    return;
  }

  // 2. 创建对应策略
#if (SYS_FLASH_DEV == FLASH_EXT_SDCARD)
  fatfs_strategy_config_t fat_cfg = {.pdrv = 0};
  flash_strategy_t *strategy = fatfs_strategy_create(&fat_cfg);
#else
  lfs_strategy_config_t lfs_cfg = {
      .read_size = 16,
      .prog_size = 16,
      .cache_size = 256,
      .lookahead_size = 32,
      .block_cycles = 200,
  };
  flash_strategy_t *strategy = lfs_strategy_create(&lfs_cfg);
#endif

  if (!strategy) {
    log_e("Failed to create storage strategy");
    return;
  }

  // 3. 挂载
  log_i("Mounting storage...");
  if (FLASH_STRATEGY_MOUNT(strategy, g_sd_dev, SYS_STORAGE_MOUNT_POINT) != 0) {
    log_e("Mount failed");
    // TODO: Need a generic way to destroy strategy
    return;
  }

  // 4. 测试文件读写
  const char *test_path = SYS_STORAGE_MOUNT_POINT "/stm32_test.txt";
  const char *test_str =
      "STM32H7 FatFS 测试!";
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

static void _fatfs_event_cb(const char *prefix, flash_event_t event) {
  if (event == FLASH_EVENT_INSERTED) {
    log_i("Card Inserted Event: %s", prefix);
    // PB0 (LED1) 闪烁几下
    for (int i = 0; i < 6; i++) {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
      HAL_Delay(100);
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    base_test();
  } else if (event == FLASH_EVENT_REMOVED) {
    log_w("Card Removed Event: %s", prefix);
  }
}

void fatfs_test_run(void) {
  base_test();
  return;

  log_i("Starting FatFS Hot-plug & Stability Test...");
  // 1. 设置回调
  flash_handler_set_callback(_fatfs_event_cb);

  // 2. 初始化核心并获取设备
  flash_handler_init();

  // 3. 获取设备并注册挂载点
  g_sd_dev = flash_factory_get(SYS_FLASH_DEV);
  if (!g_sd_dev) {
    log_e("Failed to get device (%d)", SYS_FLASH_DEV);
    return;
  }

#if (SYS_FLASH_DEV == FLASH_EXT_SDCARD)
  fatfs_strategy_config_t fat_cfg = {.pdrv = 0};
  g_fat_strat = fatfs_strategy_create(&fat_cfg);
#else
  lfs_strategy_config_t lfs_cfg = {
      .read_size = 16,
      .prog_size = 16,
      .cache_size = 256,
      .lookahead_size = 32,
      .block_cycles = 200,
  };
  g_fat_strat = lfs_strategy_create(&lfs_cfg);
#endif

  if (!g_fat_strat) {
    log_e("Failed to create strategy");
    return;
  }

  // 注册路径。由于初始可能没插卡，状态会是 DISCONNECTED
  if (flash_handler_register(SYS_STORAGE_MOUNT_POINT, g_sd_dev, g_fat_strat) !=
      0) {
    log_e("Failed to register storage in flash_handler");
    return;
  }

  log_i("Entering Hot-plug polling loop for /sd... Press Ctrl+C to exit.");
  // The original line had a duplicate "to exit."
  // log_i("Entering Hot-plug polling loop for /sd... Press Ctrl+C/Stop "
  //       "to exit.");

  while (1) {
    // 状态轮询
    flash_handler_poll();

    // 翻转 LED (PB1 = LED0)
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);

    HAL_Delay(500);
  }
}

#endif
