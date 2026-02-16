/*
 * stm32_qspi_driver.c
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 */

#include "stm32_qspi_driver.h"
#include "MemPool.h"
#include <string.h>

#define QSPI_MEMSOURCE SYS_MEM_INTERNAL

// 内部辅助函数：转换指令结构体
static void convert_command(qspi_command_t *src, QSPI_CommandTypeDef *dest) {
  dest->Instruction = src->Instruction;
  dest->Address = src->Address;
  dest->AlternateBytes = src->AlternateBytes;
  dest->AddressSize = src->AddressSize;
  dest->AlternateBytesSize = src->AlternateBytesSize;
  dest->DummyCycles = src->DummyCycles;
  dest->InstructionMode = src->InstructionMode;
  dest->AddressMode = src->AddressMode;
  dest->AlternateByteMode = src->AlternateByteMode;
  dest->DataMode = src->DataMode;
  dest->NbData = src->NbData;
  dest->DdrMode = src->DdrMode;
  dest->DdrHoldHalfCycle = src->DdrHoldHalfCycle;
  dest->SIOOMode = src->SIOOMode;
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
  stm32_qspi_driver_t *driver = (stm32_qspi_driver_t *)sys_malloc(
      QSPI_MEMSOURCE, sizeof(stm32_qspi_driver_t));
  if (driver) {
    memset(driver, 0, sizeof(stm32_qspi_driver_t));
    driver->base.ops = &stm32_qspi_ops;
    memcpy(&driver->hqspi, hqspi, sizeof(QSPI_HandleTypeDef));
  }
  return (qspi_driver_t *)driver;
}
