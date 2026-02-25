/*
 * flash_lowlevel_test.c
 *
 *  Created on: Feb 19, 2026
 *      Author: Antigravity
 *
 *  基于 Factory 的底层 Flash 读写与 XIP 测试
 */
#include "test_config.h"
#if ENABLE_TEST_FLASH

#define LOG_TAG "TEST_FLASH"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "Sys.h"
#include "elog.h"
#include "nor_flash_factory.h"
#include "test_framework.h"
#include "utils/CRC/crc32.h"

static nor_flash_driver_t *flash = NULL;

static void flash_test_setup(void) {
  // 初始化 CRC 模块
  utils_crc32_init();

  // 1. 通过 Factory 创建/获取驱动实例
  flash = nor_flash_factory_creat(NOR_FLASH_SYS);
  if (!flash) {
    log_e("Failed to create Flash driver via factory!");
    return;
  }

  log_i("Flash setup success.");

  nor_flash_info_t info;
  if (NOR_FLASH_GET_INFO(flash, &info) == 0) {
    log_i("Flash Info: Size=%dMB, Sector=%dB, Page=%dB",
          info.total_size / (1024 * 1024), info.sector_size, info.page_size);
    log_i("Device ID: %02X %02X %02X", info.device_id[0], info.device_id[1],
          info.device_id[2]);
  }
}

static void flash_test_loop(void) {
  if (flash == NULL)
    return;

  uint32_t test_addr = 0x000000; // 测试地址：起始位置
  uint8_t write_buf[256];
  uint8_t read_buf[256];

  log_i("--- Start Flash Factory-based Test ---");

  // 1. 擦除
  log_i("Erasing sector at 0x%06X...", (unsigned int)test_addr);
  if (NOR_FLASH_ERASE_SECTOR(flash, test_addr) != 0) {
    log_e("Erase failed!");
    return;
  }
  log_i("Erase done.");

  // 2. 写入测试数据
  for (int i = 0; i < 256; i++)
    write_buf[i] = (uint8_t)i;
  uint32_t expected_crc = utils_crc32_calc(write_buf, 256, 0);

  log_i("Writing 256 bytes...");
  if (NOR_FLASH_WRITE(flash, test_addr, write_buf, 256) != 0) {
    log_e("Write failed!");
    return;
  }

  // 3. 读取校验
  log_i("Reading back...");
  memset(read_buf, 0, 256);
  if (NOR_FLASH_READ(flash, test_addr, read_buf, 256) != 0) {
    log_e("Read failed!");
    return;
  }

  uint32_t actual_crc = utils_crc32_calc(read_buf, 256, 0);
  if (actual_crc == expected_crc) {
    log_i("CRC Check Passed: 0x%08X", (unsigned int)actual_crc);
  } else {
    log_e("CRC Check Failed! Exp: 0x%08X, Act: 0x%08X",
          (unsigned int)expected_crc, (unsigned int)actual_crc);
  }

  // 4. XIP 模式测试
  log_i("Switching to XIP mode...");
  if (NOR_FLASH_SET_MODE(flash, NOR_FLASH_MODE_XIP) == 0) {
    // 映射到 0x90000000
    volatile uint8_t *xip_ptr = (volatile uint8_t *)0x90000000;
    log_i("XIP First 4 bytes: %02X %02X %02X %02X", xip_ptr[0], xip_ptr[1],
          xip_ptr[2], xip_ptr[3]);

    if (xip_ptr[0] == 0x00 && xip_ptr[1] == 0x01) {
      log_i("XIP Access Success!");
    } else {
      log_e("XIP Access Data Mismatch!");
    }
  } else {
    log_e("Failed to enter XIP mode!");
  }

  flash = NULL;
  log_i("--- Test Finished ---");
}

static void flash_test_teardown(){
  flash = NULL;
  log_i("Flash test teardown.");
}

REGISTER_TEST(flash_ll, "NOR Flash Low-level Factory test", flash_test_setup,
              flash_test_loop, flash_test_teardown);
#endif
