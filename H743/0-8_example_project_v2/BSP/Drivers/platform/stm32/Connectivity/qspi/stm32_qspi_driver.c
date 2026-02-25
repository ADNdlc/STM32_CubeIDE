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

// 内部辅助函数：转换指令结构体
static void convert_command(qspi_command_t *src, QSPI_CommandTypeDef *dest) {
  dest->Instruction = src->Instruction;
  dest->Address = src->Address;
  dest->AlternateBytes = src->AlternateBytes;

  // 映射指令模式
  dest->InstructionMode = (src->InstructionMode == QSPI_DRV_INSTR_1_LINE)
                              ? QSPI_INSTRUCTION_1_LINE
                          : (src->InstructionMode == QSPI_DRV_INSTR_2_LINES)
                              ? QSPI_INSTRUCTION_2_LINES
                          : (src->InstructionMode == QSPI_DRV_INSTR_4_LINES)
                              ? QSPI_INSTRUCTION_4_LINES
                              : QSPI_INSTRUCTION_NONE;

  // 映射地址模式
  dest->AddressMode =
      (src->AddressMode == QSPI_DRV_ADDR_1_LINE)    ? QSPI_ADDRESS_1_LINE
      : (src->AddressMode == QSPI_DRV_ADDR_2_LINES) ? QSPI_ADDRESS_2_LINES
      : (src->AddressMode == QSPI_DRV_ADDR_4_LINES) ? QSPI_ADDRESS_4_LINES
                                                    : QSPI_ADDRESS_NONE;

  // 映射数据模式
  dest->DataMode =
      (src->DataMode == QSPI_DRV_DATA_1_LINE)    ? QSPI_DATA_1_LINE
      : (src->DataMode == QSPI_DRV_DATA_2_LINES) ? QSPI_DATA_2_LINES
      : (src->DataMode == QSPI_DRV_DATA_4_LINES) ? QSPI_DATA_4_LINES
                                                 : QSPI_DATA_NONE;

  // 映射地址长度
  dest->AddressSize =
      (src->AddressSize == QSPI_DRV_ADDR_8BITS)    ? QSPI_ADDRESS_8_BITS
      : (src->AddressSize == QSPI_DRV_ADDR_16BITS) ? QSPI_ADDRESS_16_BITS
      : (src->AddressSize == QSPI_DRV_ADDR_24BITS) ? QSPI_ADDRESS_24_BITS
      : (src->AddressSize == QSPI_DRV_ADDR_32BITS) ? QSPI_ADDRESS_32_BITS
                                                   : QSPI_ADDRESS_8_BITS;

  dest->AlternateBytesSize =
      (src->AlternateBytesSize == QSPI_DRV_ADDR_8BITS)    ? QSPI_ADDRESS_8_BITS
      : (src->AlternateBytesSize == QSPI_DRV_ADDR_16BITS) ? QSPI_ADDRESS_16_BITS
      : (src->AlternateBytesSize == QSPI_DRV_ADDR_24BITS) ? QSPI_ADDRESS_24_BITS
      : (src->AlternateBytesSize == QSPI_DRV_ADDR_32BITS) ? QSPI_ADDRESS_32_BITS
                                                          : QSPI_ADDRESS_8_BITS;

  dest->AlternateByteMode = (src->AlternateByteMode == QSPI_DRV_ADDR_1_LINE)
                                ? QSPI_ALTERNATE_BYTES_1_LINE
                            : (src->AlternateByteMode == QSPI_DRV_ADDR_2_LINES)
                                ? QSPI_ALTERNATE_BYTES_2_LINES
                            : (src->AlternateByteMode == QSPI_DRV_ADDR_4_LINES)
                                ? QSPI_ALTERNATE_BYTES_4_LINES
                                : QSPI_ALTERNATE_BYTES_NONE;

  dest->DummyCycles = src->DummyCycles;
  dest->NbData = src->NbData;
  dest->DdrMode = (src->DdrMode) ? QSPI_DDR_MODE_ENABLE : QSPI_DDR_MODE_DISABLE;
  dest->DdrHoldHalfCycle = (src->DdrHoldHalfCycle) ? QSPI_DDR_HHC_HALF_CLK_DELAY
                                                   : QSPI_DDR_HHC_ANALOG_DELAY;
  dest->SIOOMode = (src->SIOOMode == QSPI_DRV_SIOO_INST_ONLY_FIRST_CMD)
                       ? QSPI_SIOO_INST_ONLY_FIRST_CMD
                       : QSPI_SIOO_INST_EVERY_CMD;
}

static int stm32_qspi_command(qspi_driver_t *self, qspi_command_t *cmd,
                              uint32_t timeout) {
  stm32_qspi_driver_t *driver = (stm32_qspi_driver_t *)self;
  QSPI_CommandTypeDef sCommand;

  // 如果处于内存映射模式，必须先中止才能发送普通指令
  if (driver->hqspi->State == HAL_QSPI_STATE_BUSY_MEM_MAPPED) {
    HAL_QSPI_Abort(driver->hqspi);
  }

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

  if (driver->hqspi->State == HAL_QSPI_STATE_BUSY_MEM_MAPPED) {
    HAL_QSPI_Abort(driver->hqspi);
  }

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

  if (driver->hqspi->State == HAL_QSPI_STATE_BUSY_MEM_MAPPED) {
    HAL_QSPI_Abort(driver->hqspi);
  }

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
