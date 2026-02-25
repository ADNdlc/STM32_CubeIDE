/*
 * stm32_qspi_driver.c
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 */

#include "stm32_qspi_driver.h"
#include "MemPool.h"
#include <stdlib.h>
#include <string.h>

#define QSPI_MEMSOURCE SYS_MEM_INTERNAL

// 内部辅助函数：转换模式
static uint32_t map_mode(uint32_t mode) {
  switch (mode) {
  case QSPI_INSTRUCTION_1_LINE:
    return QSPI_INSTRUCTION_1_LINE; // 注意：HAL 的宏实际上包含了位偏移
  case QSPI_INSTRUCTION_2_LINES:
    return QSPI_INSTRUCTION_2_LINES;
  case QSPI_INSTRUCTION_4_LINES:
    return QSPI_INSTRUCTION_4_LINES;
  default:
    return QSPI_INSTRUCTION_NONE;
  }
}

// 实际上 HAL 的宏定义如下，我们需要确定是否可以直接赋值
// #define QSPI_INSTRUCTION_1_LINE (QUADSPI_CCR_IMODE_0) -> 0x00000100
// 而我的抽象定义是 1。所以必须进行转换或重新定义宏。

// 由于我在 qspi_driver.h 中定义的 1, 2, 3 是抽象值，这里需要映射到 HAL 的位定义
static uint32_t hal_map_mode(uint32_t abstract_mode) {
  if (abstract_mode == QSPI_INSTRUCTION_1_LINE)
    return QSPI_INSTRUCTION_1_LINE;
  if (abstract_mode == QSPI_INSTRUCTION_2_LINES)
    return QSPI_INSTRUCTION_2_LINES;
  if (abstract_mode == QSPI_INSTRUCTION_4_LINES)
    return QSPI_INSTRUCTION_4_LINES;
  return QSPI_INSTRUCTION_NONE;
}

static uint32_t hal_map_address_size(uint32_t abstract_size) {
  if (abstract_size == QSPI_ADDRESS_8BITS)
    return QSPI_ADDRESS_8BITS;
  if (abstract_size == QSPI_ADDRESS_16BITS)
    return QSPI_ADDRESS_16BITS;
  if (abstract_size == QSPI_ADDRESS_24BITS)
    return QSPI_ADDRESS_24BITS;
  if (abstract_size == QSPI_ADDRESS_32BITS)
    return QSPI_ADDRESS_32BITS;
  return QSPI_ADDRESS_8BITS;
}

static void convert_command(qspi_command_t *src, QSPI_CommandTypeDef *dest) {
  dest->Instruction = src->Instruction;
  dest->Address = src->Address;
  dest->AlternateBytes = src->AlternateBytes;

  // 映射指令模式
  dest->InstructionMode = (src->InstructionMode == QSPI_INSTRUCTION_1_LINE)
                              ? QSPI_INSTRUCTION_1_LINE
                          : (src->InstructionMode == QSPI_INSTRUCTION_2_LINES)
                              ? QSPI_INSTRUCTION_2_LINES
                          : (src->InstructionMode == QSPI_INSTRUCTION_4_LINES)
                              ? QSPI_INSTRUCTION_4_LINES
                              : QSPI_INSTRUCTION_NONE;

  // 映射地址模式
  dest->AddressMode =
      (src->AddressMode == QSPI_ADDRESS_1_LINE)    ? QSPI_ADDRESS_1_LINE
      : (src->AddressMode == QSPI_ADDRESS_2_LINES) ? QSPI_ADDRESS_2_LINES
      : (src->AddressMode == QSPI_ADDRESS_4_LINES) ? QSPI_ADDRESS_4_LINES
                                                   : QSPI_ADDRESS_NONE;

  // 映射数据模式
  dest->DataMode = (src->DataMode == QSPI_DATA_1_LINE)    ? QSPI_DATA_1_LINE
                   : (src->DataMode == QSPI_DATA_2_LINES) ? QSPI_DATA_2_LINES
                   : (src->DataMode == QSPI_DATA_4_LINES) ? QSPI_DATA_4_LINES
                                                          : QSPI_DATA_NONE;

  // 映射地址长度
  dest->AddressSize =
      (src->AddressSize == QSPI_ADDRESS_8BITS)    ? QSPI_ADDRESS_8BITS
      : (src->AddressSize == QSPI_ADDRESS_16BITS) ? QSPI_ADDRESS_16BITS
      : (src->AddressSize == QSPI_ADDRESS_24BITS) ? QSPI_ADDRESS_24BITS
      : (src->AddressSize == QSPI_ADDRESS_32BITS) ? QSPI_ADDRESS_32BITS
                                                  : QSPI_ADDRESS_8BITS;

  dest->AlternateBytesSize =
      (src->AlternateBytesSize == QSPI_ADDRESS_8BITS)    ? QSPI_ADDRESS_8BITS
      : (src->AlternateBytesSize == QSPI_ADDRESS_16BITS) ? QSPI_ADDRESS_16BITS
      : (src->AlternateBytesSize == QSPI_ADDRESS_24BITS) ? QSPI_ADDRESS_24BITS
      : (src->AlternateBytesSize == QSPI_ADDRESS_32BITS) ? QSPI_ADDRESS_32BITS
                                                         : QSPI_ADDRESS_8BITS;

  dest->AlternateByteMode =
      (src->AlternateByteMode == QSPI_ADDRESS_1_LINE)    ? QSPI_ADDRESS_1_LINE
      : (src->AlternateByteMode == QSPI_ADDRESS_2_LINES) ? QSPI_ADDRESS_2_LINES
      : (src->AlternateByteMode == QSPI_ADDRESS_4_LINES)
          ? QSPI_ADDRESS_4_LINES
          : QSPI_ALTERNATE_BYTES_NONE;

  dest->DummyCycles = src->DummyCycles;
  dest->NbData = src->NbData;
  dest->DdrMode = (src->DdrMode) ? QSPI_DDR_MODE_ENABLE : QSPI_DDR_MODE_DISABLE;
  dest->DdrHoldHalfCycle =
      (src->DdrHoldHalfCycle) ? QSPI_DDR_HHC_ANALOG_DELAY : QSPI_DDR_HHC_NONE;
  dest->SIOOMode = (src->SIOOMode) ? QSPI_SIOO_INST_ONLY_FIRST_CMD
                                   : QSPI_SIOO_INST_EVERY_CMD;
}

static int stm32_qspi_command(qspi_driver_t *self, qspi_command_t *cmd,
                              uint32_t timeout) {
  stm32_qspi_driver_t *driver = (stm32_qspi_driver_t *)self;
  QSPI_CommandTypeDef sCommand;
  convert_command(cmd, &sCommand);

  if (HAL_QSPI_Command(driver->hqspi, &sCommand, timeout) != HAL_OK) {
    return -1;
  }
  return 0;
}

static int stm32_qspi_transmit(qspi_driver_t *self, qspi_command_t *cmd,
                               const uint8_t *data, uint32_t timeout) {
  stm32_qspi_driver_t *driver = (stm32_qspi_driver_t *)self;
  QSPI_CommandTypeDef sCommand;
  convert_command(cmd, &sCommand);

  if (HAL_QSPI_Command(driver->hqspi, &sCommand, timeout) != HAL_OK) {
    return -1;
  }

  if (HAL_QSPI_Transmit(driver->hqspi, (uint8_t *)data, timeout) != HAL_OK) {
    return -2;
  }
  return 0;
}

static int stm32_qspi_receive(qspi_driver_t *self, qspi_command_t *cmd,
                              uint8_t *buffer, uint32_t timeout) {
  stm32_qspi_driver_t *driver = (stm32_qspi_driver_t *)self;
  QSPI_CommandTypeDef sCommand;
  convert_command(cmd, &sCommand);

  if (HAL_QSPI_Command(driver->hqspi, &sCommand, timeout) != HAL_OK) {
    return -1;
  }

  if (HAL_QSPI_Receive(driver->hqspi, buffer, timeout) != HAL_OK) {
    return -2;
  }
  return 0;
}

static int stm32_qspi_memory_mapped(qspi_driver_t *self, qspi_command_t *cmd) {
  stm32_qspi_driver_t *driver = (stm32_qspi_driver_t *)self;
  QSPI_CommandTypeDef sCommand;
  QSPI_MemoryMappedTypeDef sMemMappedCfg;

  convert_command(cmd, &sCommand);

  sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  sMemMappedCfg.TimeOutPeriod = 0;

  if (HAL_QSPI_MemoryMapped(driver->hqspi, &sCommand, &sMemMappedCfg) !=
      HAL_OK) {
    return -1;
  }
  return 0;
}

static const qspi_driver_ops_t stm32_qspi_ops = {
    .command = stm32_qspi_command,
    .transmit = stm32_qspi_transmit,
    .receive = stm32_qspi_receive,
    .memory_mapped = stm32_qspi_memory_mapped,
};

qspi_driver_t *stm32_qspi_driver_create(QSPI_HandleTypeDef *hqspi) {
#ifdef USE_MEMPOOL
  stm32_qspi_driver_t *driver = (stm32_qspi_driver_t *)sys_malloc(
      QSPI_MEMSOURCE, sizeof(stm32_qspi_driver_t));
#else
  stm32_qspi_driver_t *driver =
      (stm32_qspi_driver_t *)malloc(sizeof(stm32_qspi_driver_t));
#endif
  if (driver) {
    memset(driver, 0, sizeof(stm32_qspi_driver_t));
    driver->base.ops = &stm32_qspi_ops;
    driver->hqspi = hqspi;
  }
  return (qspi_driver_t *)driver;
}