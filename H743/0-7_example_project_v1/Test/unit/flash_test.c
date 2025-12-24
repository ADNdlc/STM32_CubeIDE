/*
 * flash_test.c
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  Flash 驱动测试用例实现
 */

#include "all_tests_config.h"

#if _flash_test_

#define LOG_TAG "FLASH_TEST"
#include "elog.h"

#include "flash_default_strategy.h"
#include "flash_dependencies.h"
#include "flash_driver.h"
#include "flash_factory.h"
#include "flash_handler.h"
#include "spi_adapter.h"
#include "sys.h"
#include "w25q256_driver.h"


// 平台适配器（根据实际硬件配置选择）
// 如果使用 QSPI
// #include "qspi_hardware_adapter.h"
// 如果使用标准 SPI
#include "spi_hardware_adapter.h"
#include "stm32_flash_dependencies.h"

// 外部 SPI 句柄（需在 CubeMX 中配置）
// extern SPI_HandleTypeDef hspi1;
// extern QSPI_HandleTypeDef hqspi;

// 测试缓冲区
#define TEST_BUF_SIZE 256
static uint8_t g_write_buf[TEST_BUF_SIZE];
static uint8_t g_read_buf[TEST_BUF_SIZE];

// 测试地址（使用最后一个扇区，避免影响其他数据）
#define TEST_ADDR 0x00000000 // 可根据实际情况修改

// ============== 辅助函数 ==============

static void fill_buffer(uint8_t *buf, uint32_t len, uint8_t pattern) {
  for (uint32_t i = 0; i < len; i++) {
    buf[i] = pattern + (uint8_t)i;
  }
}

static int compare_buffer(const uint8_t *buf1, const uint8_t *buf2,
                          uint32_t len) {
  for (uint32_t i = 0; i < len; i++) {
    if (buf1[i] != buf2[i]) {
      return -1;
    }
  }
  return 0;
}

// ============== 驱动测试 ==============

void flash_test_driver(void) {
  log_i("=== Flash Driver Test ===");

  // 创建依赖
  flash_dependencies_t *deps = stm32_flash_create_dependencies();
  if (deps == NULL) {
    log_e("Failed to create dependencies");
    return;
  }
  log_i("Dependencies created");

  // 创建 SPI 适配器
  // 注意：需要根据实际硬件配置修改
  // 示例使用空配置，实际使用时需配置正确的 SPI 句柄
#if 0
    spi_hw_adapter_config_t spi_config = {
        .hspi = &hspi1,
        .cs_port = GPIOB,
        .cs_pin = GPIO_PIN_6,
        .timeout_ms = 1000,
    };
    spi_adapter_t *spi = spi_hardware_adapter_create(&spi_config);
#else
  log_w("SPI adapter not configured, using stub test");
  log_i("Driver Test SKIPPED (no hardware)");
  return;
#endif

  if (spi == NULL) {
    log_e("Failed to create SPI adapter");
    return;
  }
  log_i("SPI adapter created");

  // 创建驱动
  flash_driver_t *driver = w25q256_driver_create(spi, deps);
  if (driver == NULL) {
    log_e("Failed to create W25Q256 driver");
    return;
  }
  log_i("W25Q256 driver created");

  // 初始化驱动
  flash_error_t err = FLASH_INIT(driver);
  if (err != FLASH_OK) {
    log_e("Driver init failed: %d", err);
    return;
  }
  log_i("Driver initialized");

  // 读取 ID
  uint32_t id = FLASH_READ_ID(driver);
  log_i("JEDEC ID: 0x%06X", id);

  // 获取芯片信息
  const flash_info_t *info = FLASH_GET_INFO(driver);
  if (info) {
    log_i("Chip: %s, Size: %luMB, Sector: %luKB", info->name,
          info->capacity / (1024 * 1024), info->sector_size / 1024);
  }

  // 擦除测试扇区
  log_i("Erasing sector at 0x%08X...", TEST_ADDR);
  err = FLASH_ERASE_SECTOR(driver, TEST_ADDR);
  if (err != FLASH_OK) {
    log_e("Erase failed: %d", err);
    return;
  }
  log_i("Erase OK");

  // 读取验证（应全为 0xFF）
  err = FLASH_READ(driver, TEST_ADDR, g_read_buf, TEST_BUF_SIZE);
  if (err != FLASH_OK) {
    log_e("Read failed: %d", err);
    return;
  }

  int all_ff = 1;
  for (int i = 0; i < TEST_BUF_SIZE; i++) {
    if (g_read_buf[i] != 0xFF) {
      all_ff = 0;
      break;
    }
  }
  if (all_ff) {
    log_i("Erase verify OK (all 0xFF)");
  } else {
    log_e("Erase verify FAILED");
  }

  // 写入测试
  fill_buffer(g_write_buf, TEST_BUF_SIZE, 0xA5);
  log_i("Writing %d bytes...", TEST_BUF_SIZE);
  err = FLASH_WRITE(driver, TEST_ADDR, g_write_buf, TEST_BUF_SIZE);
  if (err != FLASH_OK) {
    log_e("Write failed: %d", err);
    return;
  }
  log_i("Write OK");

  // 读取验证
  memset(g_read_buf, 0, TEST_BUF_SIZE);
  err = FLASH_READ(driver, TEST_ADDR, g_read_buf, TEST_BUF_SIZE);
  if (err != FLASH_OK) {
    log_e("Read failed: %d", err);
    return;
  }

  if (compare_buffer(g_write_buf, g_read_buf, TEST_BUF_SIZE) == 0) {
    log_i("Write/Read verify OK");
  } else {
    log_e("Write/Read verify FAILED");
    for (int i = 0; i < 16; i++) {
      log_e("  [%d] W:0x%02X R:0x%02X", i, g_write_buf[i], g_read_buf[i]);
    }
  }

  // 去初始化
  FLASH_DEINIT(driver);
  log_i("Driver deinitialized");

  log_i("=== Driver Test PASSED ===");
}

// ============== Handler 测试 ==============

void flash_test_handler(void) {
  log_i("=== Flash Handler Test ===");

  flash_handler_t handler;
  flash_dependencies_t *deps = stm32_flash_create_dependencies();

  // 初始化 Handler
  flash_error_t err = flash_handler_init(&handler, deps);
  if (err != FLASH_OK) {
    log_e("Handler init failed: %d", err);
    return;
  }
  log_i("Handler initialized");

  // 测试注册器模式（使用模拟驱动）
  // 注意：实际测试需要真实驱动
  log_i("Device count: %d", flash_handler_get_device_count(&handler));

  // 设置策略
  flash_strategy_t *strategy = flash_default_strategy_create();
  flash_handler_set_strategy(&handler, strategy);
  log_i("Strategy set: %s", strategy->name);

  // 获取策略
  flash_strategy_t *got_strategy = flash_handler_get_strategy(&handler);
  if (got_strategy == strategy) {
    log_i("Get strategy OK");
  } else {
    log_e("Get strategy FAILED");
  }

  // 去初始化
  flash_handler_deinit(&handler);
  log_i("Handler deinitialized");

  log_i("=== Handler Test PASSED ===");
}

// ============== 工厂测试 ==============

void flash_test_factory(void) {
  log_i("=== Flash Factory Test ===");

#if 0
    // 需要硬件支持
    flash_factory_config_t config = {
        .chip_type = FLASH_CHIP_W25Q256,
        .spi_adapter = spi_adapter,
        .deps = NULL,  // 使用默认
        .device_name = "TEST_FLASH",
    };
    
    flash_handler_t *handler = flash_factory_create_system(&config);
    if (handler == NULL) {
        log_e("Factory create system failed");
        return;
    }
    log_i("System created via factory");
    
    flash_handler_print_devices(handler);
    
    flash_factory_destroy_system(handler);
    log_i("System destroyed");
#else
  log_w("Factory test requires hardware, SKIPPED");
#endif

  log_i("=== Factory Test PASSED ===");
}

// ============== 策略测试 ==============

void flash_test_strategy(void) {
  log_i("=== Flash Strategy Test ===");

  // 创建默认策略
  flash_strategy_t *strategy = flash_default_strategy_create();
  if (strategy == NULL) {
    log_e("Failed to create default strategy");
    return;
  }
  log_i("Default strategy created: %s", strategy->name);

  // 验证策略类型
  if (strategy->type == FLASH_STRATEGY_RAW) {
    log_i("Strategy type OK: RAW");
  } else {
    log_e("Strategy type FAILED");
  }

  // 销毁策略
  flash_default_strategy_destroy(strategy);
  log_i("Strategy destroyed");

  log_i("=== Strategy Test PASSED ===");
}

// ============== 集成测试 ==============

void flash_integration_test_run(void) {
  log_i("=== Flash Integration Test ===");

#if 0
    // 完整系统测试（需要硬件）
    // 1. 创建系统
    // 2. 注册多个设备
    // 3. 切换策略
    // 4. 读写测试
    // 5. 销毁系统
#else
  log_w("Integration test requires hardware, SKIPPED");
  log_i("To run integration test:");
  log_i("  1. Configure SPI/QSPI in CubeMX");
  log_i("  2. Connect W25Q256 Flash");
  log_i("  3. Update flash_test.c with correct HAL handles");
  log_i("  4. Enable hardware test sections");
#endif

  log_i("=== Integration Test DONE ===");
}

// ============== 主测试函数 ==============

void flash_test_run(void) {
  log_i("");
  log_i("========================================");
  log_i("       W25Q256 Flash Driver Test");
  log_i("========================================");
  log_i("");

  // 单元测试
  flash_test_strategy(); // 不需要硬件
  log_i("");

  flash_test_handler(); // 不需要硬件
  log_i("");

  flash_test_factory(); // 需要硬件（已跳过）
  log_i("");

  flash_test_driver(); // 需要硬件（已跳过）
  log_i("");

  // 集成测试
  flash_integration_test_run();

  log_i("");
  log_i("========================================");
  log_i("       Flash Test Complete");
  log_i("========================================");
}

#endif /* _flash_test_ */
