/*
 * stm32_sdram_driver.c
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#include "stm32_sdram_driver.h"
#include <stdlib.h>

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

// Internal implementation struct helper
typedef struct {
  stm32_sdram_driver_t public;
} stm32_sdram_impl_t;

static stm32_sdram_impl_t sdram_singleton;

static int stm32_sdram_send_command(sdram_driver_t *self, uint32_t command_mode,
                                    uint32_t target_bank,
                                    uint32_t auto_refresh_num,
                                    uint32_t mode_reg_def) {
  stm32_sdram_impl_t *impl = (stm32_sdram_impl_t *)self;
  FMC_SDRAM_CommandTypeDef Command;

  Command.CommandMode = command_mode;
  Command.CommandTarget = target_bank;
  Command.AutoRefreshNumber = auto_refresh_num;
  Command.ModeRegisterDefinition = mode_reg_def;

  if (HAL_SDRAM_SendCommand(impl->public.hsdram, &Command, SDRAM_TIMEOUT) !=
      HAL_OK) {
    return -1;
  }
  return 0;
}

static int stm32_sdram_init(sdram_driver_t *self) {
  // Ported from W9825G6KH.c SDRAM_InitSequence
  // Assumes hsdram is already initialized by HAL_SDRAM_Init in main/fmc.c

  // We can use the helper send_command or direct HAL call.
  // Use helper to verify it works.

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

  // Step 6: Set Refresh Rate
  // Count = 64ms * 100MHz / 8192 - 20 = 761 (approx)
  // Old driver used 917 (assuming 120MHz?).
  // If HCLK is 200MHz, FMC clock is HCLK/2 = 100MHz?
  // Let's stick to the calculation or value from old driver if clock matches.
  // Usage in old driver: HAL_SDRAM_ProgramRefreshRate(&sdramHandle, 917);
  // Let's copy 917 for now, assuming same clock config.
  stm32_sdram_impl_t *impl = (stm32_sdram_impl_t *)self;
  HAL_SDRAM_ProgramRefreshRate(impl->public.hsdram, 917);

  return ret;
}

static const sdram_driver_ops_t sdram_ops = {
    .init = stm32_sdram_init, .send_command = stm32_sdram_send_command};

sdram_driver_t *stm32_sdram_create(SDRAM_HandleTypeDef *hsdram) {
  stm32_sdram_impl_t *driver = &sdram_singleton;
  driver->public.base.ops = &sdram_ops;
  driver->public.hsdram = hsdram;
  return (sdram_driver_t *)driver;
}
