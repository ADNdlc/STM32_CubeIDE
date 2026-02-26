/*
 * flash_hal_level_test.c
 *
 *  Created on: Feb 25, 2026
 *
 *  直接基于 HAL 库的裸 QSPI Flash 读写测试
 *  用来排除 nor_flash_factory 框架包装导致的问题
 */
#include "test_config.h"

#if ENABLE_TEST_FLASH_HAL

#define LOG_TAG "TEST_FLASH_HAL"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "Sys.h"
#include "elog.h"
#include "quadspi.h" // 获取 hqspi 实例
#include "test_framework.h"
#include "utils/CRC/crc32.h"

extern QSPI_HandleTypeDef hqspi;

// W25Q 指令集
#define W25Q_WRITE_ENABLE 0x06
#define W25Q_READ_STATUS_REG1 0x05
#define W25Q_READ_STATUS_REG2 0x35
#define W25Q_WRITE_STATUS_REG2 0x31
#define W25Q_QUAD_PAGE_PROGRAM 0x32
#define W25Q_SECTOR_ERASE 0x20
#define W25Q_QUAD_OUT_READ 0x6B
#define W25Q_QUAD_IO_READ 0xEB

static int w25q_hal_wait_busy(uint32_t timeout) {
  uint8_t status = 0;
  QSPI_CommandTypeDef cmd = {0};
  cmd.Instruction = W25Q_READ_STATUS_REG1;
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.DataMode = QSPI_DATA_1_LINE;
  cmd.NbData = 1;

  uint32_t start_tick = sys_get_systick_ms();
  do {
    if (HAL_QSPI_Command(&hqspi, &cmd, 10) != HAL_OK)
      return -1;
    if (HAL_QSPI_Receive(&hqspi, &status, 10) != HAL_OK)
      return -1;
    if (!(status & 0x01))
      return 0; // WIP bit clear
  } while (sys_get_systick_ms() - start_tick < timeout);

  return -2; // Timeout
}

static int w25q_hal_write_enable(void) {
  QSPI_CommandTypeDef cmd = {0};
  cmd.Instruction = W25Q_WRITE_ENABLE;
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  return HAL_QSPI_Command(&hqspi, &cmd, 10) == HAL_OK ? 0 : -1;
}

static int w25q_hal_enable_quad_mode(void) {
  uint8_t status2 = 0;
  QSPI_CommandTypeDef cmd = {0};

  cmd.Instruction = W25Q_READ_STATUS_REG2;
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.DataMode = QSPI_DATA_1_LINE;
  cmd.NbData = 1;
  if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    return -1;
  if (HAL_QSPI_Receive(&hqspi, &status2, 100) != HAL_OK)
    return -1;

  if (status2 & 0x02)
    return 0; // Already QE=1

  if (w25q_hal_write_enable() != 0)
    return -2;

  status2 |= 0x02;
  cmd.Instruction = W25Q_WRITE_STATUS_REG2;
  cmd.NbData = 1;
  if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    return -3;
  if (HAL_QSPI_Transmit(&hqspi, &status2, 100) != HAL_OK)
    return -4;

  return w25q_hal_wait_busy(100);
}

static int w25q_hal_erase_sector(uint32_t addr) {
  QSPI_CommandTypeDef cmd = {0};

  if (w25q_hal_write_enable() != 0)
    return -1;

  cmd.Instruction = 0x21; // W25Q_SECTOR_ERASE_4B (0x21) instead of 0x20
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.AddressMode = QSPI_ADDRESS_1_LINE;
  cmd.AddressSize = QSPI_ADDRESS_32_BITS; // 32 Bits for W25Q256
  cmd.Address = addr;

  if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    return -2;
  return w25q_hal_wait_busy(500);
}

static int w25q_hal_write_page(uint32_t addr, const uint8_t *data, size_t len) {
  QSPI_CommandTypeDef cmd = {0};

  if (w25q_hal_write_enable() != 0)
    return -1;

  cmd.Instruction = 0x34; // W25Q_QUAD_PAGE_PROGRAM_4B (0x34) instead of 0x32
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.AddressMode = QSPI_ADDRESS_1_LINE;
  cmd.AddressSize = QSPI_ADDRESS_32_BITS; // 32 Bits for W25Q256
  cmd.Address = addr;
  cmd.DataMode = QSPI_DATA_4_LINES;
  cmd.NbData = len;

  if (HAL_QSPI_Command(&hqspi, &cmd, 1000) != HAL_OK)
    return -2;
  if (HAL_QSPI_Transmit(&hqspi, (uint8_t *)data, 1000) != HAL_OK)
    return -3;
  return w25q_hal_wait_busy(200);
}

static int w25q_hal_read(uint32_t addr, uint8_t *buf, size_t len) {
  QSPI_CommandTypeDef cmd = {0};

  cmd.Instruction = 0xEC; // W25Q_QUAD_IO_READ_4B
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.AddressMode = QSPI_ADDRESS_4_LINES;
  cmd.AddressSize = QSPI_ADDRESS_32_BITS; // 32 bits for W25Q256
  cmd.Address = addr;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
  cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
  cmd.AlternateBytes = 0xFF; // Mode Bits
  cmd.DummyCycles = 4;
  cmd.DataMode = QSPI_DATA_4_LINES;
  cmd.NbData = len;

  if (HAL_QSPI_Command(&hqspi, &cmd, 1000) != HAL_OK)
    return -1;
  return HAL_QSPI_Receive(&hqspi, buf, 1000) == HAL_OK ? 0 : -2;
}

static void w25q_hal_hard_reset(void) {
  QSPI_CommandTypeDef cmd = {0};

  // 1. Try to exit QPI mode (0xFF on 4 lines)
  cmd.Instruction = 0xFF;
  cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
  HAL_QSPI_Command(&hqspi, &cmd, 100);

  // 2. Try to exit QPI mode (0xFF on 1 line just in case)
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  HAL_QSPI_Command(&hqspi, &cmd, 100);

  // 3. Enable Reset (0x66) on 1 line
  cmd.Instruction = 0x66;
  HAL_QSPI_Command(&hqspi, &cmd, 100);

  // 4. Reset Device (0x99) on 1 line
  cmd.Instruction = 0x99;
  HAL_QSPI_Command(&hqspi, &cmd, 100);

  sys_delay_ms(50); // wait for reset completion
}

static void w25q_hal_read_id(void) {
  uint8_t id[3] = {0};
  QSPI_CommandTypeDef cmd = {0};
  cmd.Instruction = 0x9F;
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.DataMode = QSPI_DATA_1_LINE;
  cmd.NbData = 3;

  if (HAL_QSPI_Command(&hqspi, &cmd, 100) == HAL_OK) {
    if (HAL_QSPI_Receive(&hqspi, id, 100) == HAL_OK) {
      log_i("JEDEC ID: %02X %02X %02X", id[0], id[1], id[2]);
    } else {
      log_e("Failed to receive JEDEC ID");
    }
  } else {
    log_e("Failed to send JEDEC ID command");
  }
}

static void flash_hal_test_setup(void) {
  utils_crc32_init();
  w25q_hal_hard_reset();
  w25q_hal_read_id();
  w25q_hal_enable_quad_mode();
  log_i("HAL Flash setup success.");
}

static void flash_hal_test_loop(void) {
  uint32_t test_addr = 0x000000;
  uint8_t write_buf[256];
  uint8_t read_buf[256];

  log_i("--- Start HAL-based QSPI Flash Test ---");

  // 1. 擦除
  log_i("Erasing sector at 0x%06X...", (unsigned int)test_addr);
  if (w25q_hal_erase_sector(test_addr) != 0) {
    log_e("HAL Erase failed!");
    return;
  }
  log_i("Erase done.");

  // 2. 写入测试数据
  for (int i = 0; i < 256; i++)
    write_buf[i] = (uint8_t)i;
  uint32_t expected_crc = utils_crc32_calc(write_buf, 256, 0);

  log_i("Writing 256 bytes...");
  if (w25q_hal_write_page(test_addr, write_buf, 256) != 0) {
    log_e("HAL Write failed!");
    return;
  }

  // 3. 读取校验
  log_i("Reading back...");
  memset(read_buf, 0, 256);
  if (w25q_hal_read(test_addr, read_buf, 256) != 0) {
    log_e("HAL Read failed!");
    return;
  }

  uint32_t actual_crc = utils_crc32_calc(read_buf, 256, 0);
  if (actual_crc == expected_crc) {
    log_i("CRC Check Passed: 0x%08X", (unsigned int)actual_crc);
  } else {
    log_e("CRC Check Failed! Exp: 0x%08X, Act: 0x%08X",
          (unsigned int)expected_crc, (unsigned int)actual_crc);
    log_e("First 4 bytes actual: %02X %02X %02X %02X", read_buf[0], read_buf[1],
          read_buf[2], read_buf[3]);
  }

  log_i("--- HAL Test Finished ---");
}

static void flash_hal_test_teardown() { log_i("HAL Flash test teardown."); }

REGISTER_TEST(flash_hal, "NOR Flash Low-level HAL test", flash_hal_test_setup,
              flash_hal_test_loop, flash_hal_test_teardown);
#endif
