/*
 * stm32_sdram_driver.c
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#include "stm32_sdram_driver.h"
#include <stdlib.h>
#include "MemPool.h"

// W9825G6KH specific constants (Standard SDRAM Mode Register)
#define SDRAM_MODEREG_BURST_LENGTH_1 ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2 ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4 ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8 ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2 ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3 ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE ((uint16_t)0x0200)

#define SDRAM_TIMEOUT ((uint32_t)0xFFFF)

static int stm32_sdram_send_command(sdram_driver_t *self, uint32_t command_mode,
                                    uint32_t target_bank,
                                    uint32_t auto_refresh_num,
                                    uint32_t mode_reg_def) {
  stm32_sdram_driver_t *driver = (stm32_sdram_driver_t *)self;
  FMC_SDRAM_CommandTypeDef Command;

  Command.CommandMode = command_mode;
  Command.CommandTarget = target_bank;
  Command.AutoRefreshNumber = auto_refresh_num;
  Command.ModeRegisterDefinition = mode_reg_def;

  if (HAL_SDRAM_SendCommand(driver->hsdram, &Command, SDRAM_TIMEOUT) !=
      HAL_OK) {
    return -1;
  }
  return 0;
}

static int stm32_sdram_init(sdram_driver_t *self) {
  int ret = 0;
  // Step 1: Clock Enable
  ret |= stm32_sdram_send_command(self, FMC_SDRAM_CMD_CLK_ENABLE,
                                  FMC_SDRAM_CMD_TARGET_BANK1, 1, 0);
  // Step 2: Delay
  HAL_Delay(1);
  // Step 3: PALL
  ret |= stm32_sdram_send_command(self, FMC_SDRAM_CMD_PALL,
                                  FMC_SDRAM_CMD_TARGET_BANK1, 1, 0);
  // Step 4: Auto Refresh
  ret |= stm32_sdram_send_command(self, FMC_SDRAM_CMD_AUTOREFRESH_MODE,
                                  FMC_SDRAM_CMD_TARGET_BANK1, 4, 0);
  // Step 5: Load Mode Register
  uint32_t tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_8 |
                  SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
                  SDRAM_MODEREG_CAS_LATENCY_2 |
                  SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                  SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
  ret |= stm32_sdram_send_command(self, FMC_SDRAM_CMD_LOAD_MODE,
                                  FMC_SDRAM_CMD_TARGET_BANK1, 1, tmpr);
  /* 设置自刷新速率 */
  // 刷新频率计数器(以SDCLK频率计数),计算方法:
  // COUNT=SDRAM刷新周期/行数-20=SDRAM刷新周期(us)*SDCLK频率(Mhz)/行数
  // 我们使用的SDRAM刷新周期为64ms,SDCLK=200/2=100Mhz,行数为8192(2^13).
  // 所以,COUNT=64*1000*100/8192-20=761(hsdram=100MHz)s
  // 或  ,COUNT=64*1000*120/8192-20=917(hsdram=120MHz)
  stm32_sdram_driver_t *driver = (stm32_sdram_driver_t *)self;
  HAL_SDRAM_ProgramRefreshRate(driver->hsdram, 917);

  return ret;
}

static const sdram_driver_ops_t sdram_ops = {
    .init = stm32_sdram_init, .send_command = stm32_sdram_send_command};

sdram_driver_t *stm32_sdram_create(SDRAM_HandleTypeDef *hsdram) {
#ifdef USE_MEMPOOL
  stm32_sdram_driver_t *driver = (stm32_sdram_driver_t *)sys_malloc(
      RAMDEV_MEMSOURCE, sizeof(stm32_sdram_driver_t));
#else
  stm32_sdram_driver_t *driver = (stm32_sdram_driver_t *)malloc(sizeof(stm32_sdram_driver_t));
#endif
  if (driver) {
    driver->base.ops = &sdram_ops;
    driver->hsdram = hsdram;
  }
  return (sdram_driver_t *)driver;
}
