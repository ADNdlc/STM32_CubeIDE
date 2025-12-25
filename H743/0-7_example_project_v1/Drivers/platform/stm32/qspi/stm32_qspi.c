#include "stm32_qspi.h"
#include <stdlib.h>

#define STM32_QSPI_MEMSOURCE SYS_MEM_INTERNAL

// 将qspi_command_t 转换为 HAL_QSPI_CommandTypeDef
static void _convert_cmd(qspi_command_t *src, QSPI_CommandTypeDef *dst) {
  dst->Instruction = src->instruction;
  dst->Address = src->address;
  dst->AlternateBytes = src->alternate_byte;
  dst->AddressSize = src->address_size;
  dst->DummyCycles = src->dummy_cycles;
  dst->NbData = src->data_size;

  dst->InstructionMode = src->instruction_mode;
  dst->AddressMode = src->address_mode;
  dst->AlternateByteMode = src->alternate_byte_mode;
  dst->DataMode = src->data_mode;

  // Default/Config mappings need careful handling based on STM32 HAL constants
  // For now we assume the user passes compatible values or we need a mapping
  // function But since this is specific to STM32, the upper layer (Adapter)
  // might need to know STM32 constants OR we define generic constants in
  // qspi_driver.h and map them here. For simplicity, we assume the adapter
  // passes valid HAL constants casted to uint8_t

  dst->SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
  dst->DdrMode = QSPI_DDR_MODE_DISABLE; // Assume SDR for now or add to config
  dst->DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
}

static int _stm32_qspi_command(qspi_driver_t *self, qspi_command_t *cmd,
                               uint32_t timeout) {
  stm32_qspi_t *qspi = (stm32_qspi_t *)self;
  QSPI_CommandTypeDef s_command;
  _convert_cmd(cmd, &s_command);

  if (HAL_QSPI_Command(qspi->hqspi, &s_command, timeout) == HAL_OK) {
    return 0;
  }
  return -1;
}

static int _stm32_qspi_transmit(qspi_driver_t *self, const uint8_t *data,
                                uint32_t timeout) {
  stm32_qspi_t *qspi = (stm32_qspi_t *)self;
  if (HAL_QSPI_Transmit(qspi->hqspi, (uint8_t *)data, timeout) == HAL_OK) {
    return 0;
  }
  return -1;
}

static int _stm32_qspi_receive(qspi_driver_t *self, uint8_t *data,
                               uint32_t timeout) {
  stm32_qspi_t *qspi = (stm32_qspi_t *)self;
  if (HAL_QSPI_Receive(qspi->hqspi, data, timeout) == HAL_OK) {
    return 0;
  }
  return -1;
}

static int _stm32_qspi_auto_polling(qspi_driver_t *self, qspi_command_t *cmd,
                                    qspi_command_t *cfg, uint32_t timeout) {
  stm32_qspi_t *qspi = (stm32_qspi_t *)self;
  QSPI_CommandTypeDef s_command;
  QSPI_AutoPollingTypeDef s_config;

  _convert_cmd(cmd, &s_command);

  // We repurpose the 'cfg' command struct to hold polling config if needed
  // or we extend the interface. For now, assuming cfg->address holds match and
  // cfg->alternate is mask
  s_config.Match = cfg->address;
  s_config.Mask = cfg->alternate_byte;
  s_config.MatchMode = cfg->instruction; // Hacky mapping, better if we had a
                                         // dedicated config struct
  s_config.StatusBytesSize = cfg->data_size;
  s_config.Interval = 0x10;
  s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(qspi->hqspi, &s_command, &s_config, timeout) ==
      HAL_OK) {
    return 0;
  }
  return -1;
}

static int _stm32_qspi_memory_mapped(qspi_driver_t *self, qspi_command_t *cmd) {
  stm32_qspi_t *qspi = (stm32_qspi_t *)self;
  QSPI_CommandTypeDef s_command;
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;

  _convert_cmd(cmd, &s_command);

  s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  s_mem_mapped_cfg.TimeOutPeriod = 0;

  if (HAL_QSPI_MemoryMapped(qspi->hqspi, &s_command, &s_mem_mapped_cfg) ==
      HAL_OK) {
    return 0;
  }
  return -1;
}

static const qspi_driver_ops_t stm32_qspi_ops = {
    .command = _stm32_qspi_command,
    .transmit = _stm32_qspi_transmit,
    .receive = _stm32_qspi_receive,
    .auto_polling = _stm32_qspi_auto_polling,
    .memory_mapped = _stm32_qspi_memory_mapped,
};

qspi_driver_t *stm32_qspi_create(QSPI_HandleTypeDef *hqspi) {
  stm32_qspi_t *qspi =
      (stm32_qspi_t *)sys_malloc(STM32_QSPI_MEMSOURCE, sizeof(stm32_qspi_t));
  if (qspi) {
    qspi->parent.ops = &stm32_qspi_ops;
    qspi->parent.user_data = qspi;
    qspi->hqspi = hqspi;
    return &qspi->parent;
  }
  return NULL;
}

void stm32_qspi_destroy(qspi_driver_t *driver) {
  if (driver) {
    sys_free(STM32_QSPI_MEMSOURCE, driver);
  }
}
