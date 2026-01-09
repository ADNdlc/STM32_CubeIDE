#include "stm32_qspi.h"
#include "elog.h"
#include <stdlib.h>

#define LOG_TAG "STM32_QSPI"
#define STM32_QSPI_MEMSOURCE SYS_MEM_INTERNAL

// 指令模式映射 -> QSPI_INSTRUCTION_*
static uint32_t _map_instruction_mode(qspi_mode_t mode) {
  switch (mode) {
  case QSPI_MODE_1_LINE:
    return QSPI_INSTRUCTION_1_LINE;
  case QSPI_MODE_2_LINES:
    return QSPI_INSTRUCTION_2_LINES;
  case QSPI_MODE_4_LINES:
    return QSPI_INSTRUCTION_4_LINES;
  default:
    return QSPI_INSTRUCTION_NONE;
  }
}

// 地址模式映射 -> QSPI_ADDRESS_*
static uint32_t _map_address_mode(qspi_mode_t mode) {
  switch (mode) {
  case QSPI_MODE_1_LINE:
    return QSPI_ADDRESS_1_LINE;
  case QSPI_MODE_2_LINES:
    return QSPI_ADDRESS_2_LINES;
  case QSPI_MODE_4_LINES:
    return QSPI_ADDRESS_4_LINES;
  default:
    return QSPI_ADDRESS_NONE;
  }
}

// 数据模式映射 -> QSPI_DATA_*
static uint32_t _map_data_mode(qspi_mode_t mode) {
  switch (mode) {
  case QSPI_MODE_1_LINE:
    return QSPI_DATA_1_LINE;
  case QSPI_MODE_2_LINES:
    return QSPI_DATA_2_LINES;
  case QSPI_MODE_4_LINES:
    return QSPI_DATA_4_LINES;
  default:
    return QSPI_DATA_NONE;
  }
}

// 交替字节模式映射 -> QSPI_ALTERNATE_BYTES_*
static uint32_t _map_alternate_bytes_mode(qspi_mode_t mode) {
  switch (mode) {
  case QSPI_MODE_1_LINE:
    return QSPI_ALTERNATE_BYTES_1_LINE;
  case QSPI_MODE_2_LINES:
    return QSPI_ALTERNATE_BYTES_2_LINES;
  case QSPI_MODE_4_LINES:
    return QSPI_ALTERNATE_BYTES_4_LINES;
  default:
    return QSPI_ALTERNATE_BYTES_NONE;
  }
}

// 将 qspi_command_t 转换为 HAL_QSPI_CommandTypeDef
static void _convert_cmd(qspi_command_t *src, QSPI_CommandTypeDef *dst) {
  dst->Instruction = src->instruction;
  dst->Address = src->address;
  dst->AlternateBytes = src->alternate_byte;
  dst->DummyCycles = src->dummy_cycles;
  dst->NbData = src->data_size;

  dst->InstructionMode = _map_instruction_mode(src->instruction_mode);
  dst->AddressMode = _map_address_mode(src->address_mode);

  // 映射地址大小
  if (src->address_size == QSPI_ADDR_32_BITS) {
    dst->AddressSize = QSPI_ADDRESS_32_BITS;
  } else {
    dst->AddressSize = QSPI_ADDRESS_24_BITS;
  }

  dst->AlternateByteMode = _map_alternate_bytes_mode(src->alternate_byte_mode);
  dst->AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
  dst->DataMode = _map_data_mode(src->data_mode);

  dst->DdrMode = QSPI_DDR_MODE_DISABLE;
  dst->DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY; // 关键：必须设置
  dst->SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
}

static int _stm32_qspi_command(qspi_driver_t *self, qspi_command_t *cmd,
                               uint32_t timeout) {
  stm32_qspi_t *qspi = (stm32_qspi_t *)self;
  QSPI_CommandTypeDef s_command = {0};
  _convert_cmd(cmd, &s_command);
  // 根据实际需求配置DdrHoldHalfCycle，对于QSPI Flash操作，通常设置为DISABLE
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;

  HAL_StatusTypeDef status = HAL_QSPI_Command(qspi->hqspi, &s_command, timeout);
  if (status == HAL_OK) {
    return 0;
  }

  // 调试：打印QSPI状态和错误码
  log_e(
      "HAL_QSPI_Command failed, HAL_Status=%d, QSPI_State=%d, ErrorCode=0x%08X",
      status, qspi->hqspi->State, (unsigned int)qspi->hqspi->ErrorCode);
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
  QSPI_CommandTypeDef s_command = {0};
  QSPI_AutoPollingTypeDef s_config = {0};

  _convert_cmd(cmd, &s_command);
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;

  // 映射配置 (使用通用的 cfg 结构体传递 match/mask)
  s_config.Match = cfg->address;
  s_config.Mask = cfg->alternate_byte;
  s_config.MatchMode = (cfg->instruction == QSPI_MATCH_AND)
                           ? QSPI_MATCH_MODE_AND
                           : QSPI_MATCH_MODE_OR;
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
  QSPI_CommandTypeDef s_command = {0};
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

  _convert_cmd(cmd, &s_command);
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;

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
