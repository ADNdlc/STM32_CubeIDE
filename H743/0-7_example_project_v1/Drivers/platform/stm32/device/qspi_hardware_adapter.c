/*
 * qspi_hardware_adapter.c
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  STM32 QSPI 硬件适配器实现
 */

#include "qspi_hardware_adapter.h"
#include <string.h>

// 私有数据
typedef struct {
  QSPI_HandleTypeDef *hqspi;
  uint32_t flash_size;
  uint8_t use_4byte_addr;
} qspi_hw_priv_t;

// ============== 操作实现 ==============

static spi_error_t qspi_hw_init(spi_adapter_t *self) {
  // QSPI 通常已在 CubeMX 中初始化
  (void)self;
  return SPI_OK;
}

static spi_error_t qspi_hw_deinit(spi_adapter_t *self) {
  (void)self;
  return SPI_OK;
}

static void qspi_hw_select(spi_adapter_t *self) {
  // QSPI 由硬件自动管理 CS
  (void)self;
}

static void qspi_hw_deselect(spi_adapter_t *self) {
  // QSPI 由硬件自动管理 CS
  (void)self;
}

// QSPI 单线发送（用于命令）
static spi_error_t qspi_hw_transmit(spi_adapter_t *self, const uint8_t *data,
                                    uint32_t len, uint32_t timeout_ms) {
  if (self == NULL || self->priv == NULL || data == NULL || len == 0) {
    return SPI_ERR_PARAM;
  }

  qspi_hw_priv_t *priv = (qspi_hw_priv_t *)self->priv;

  // 对于简单命令（1字节），使用间接写模式
  QSPI_CommandTypeDef cmd = {0};
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction = data[0];
  cmd.AddressMode = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DummyCycles = 0;
  cmd.DataMode = QSPI_DATA_NONE;
  cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  if (len > 1) {
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = len - 1;
  }

  HAL_StatusTypeDef status = HAL_QSPI_Command(priv->hqspi, &cmd, timeout_ms);
  if (status != HAL_OK) {
    return SPI_ERR_TRANSFER;
  }

  if (len > 1) {
    status = HAL_QSPI_Transmit(priv->hqspi, (uint8_t *)(data + 1), timeout_ms);
    if (status != HAL_OK) {
      return SPI_ERR_TRANSFER;
    }
  }

  return SPI_OK;
}

// QSPI 单线接收
static spi_error_t qspi_hw_receive(spi_adapter_t *self, uint8_t *data,
                                   uint32_t len, uint32_t timeout_ms) {
  if (self == NULL || self->priv == NULL || data == NULL || len == 0) {
    return SPI_ERR_PARAM;
  }

  qspi_hw_priv_t *priv = (qspi_hw_priv_t *)self->priv;

  HAL_StatusTypeDef status = HAL_QSPI_Receive(priv->hqspi, data, timeout_ms);

  return (status == HAL_OK) ? SPI_OK : SPI_ERR_TRANSFER;
}

static spi_error_t qspi_hw_transmit_receive(spi_adapter_t *self,
                                            const uint8_t *tx_data,
                                            uint8_t *rx_data, uint32_t len,
                                            uint32_t timeout_ms) {
  // QSPI 不支持同时收发，分开处理
  (void)self;
  (void)tx_data;
  (void)rx_data;
  (void)len;
  (void)timeout_ms;
  return SPI_ERR_PARAM;
}

// QSPI 命令模式
static spi_error_t qspi_hw_command(spi_adapter_t *self,
                                   const spi_command_t *cmd) {
  if (self == NULL || self->priv == NULL || cmd == NULL) {
    return SPI_ERR_PARAM;
  }

  qspi_hw_priv_t *priv = (qspi_hw_priv_t *)self->priv;

  QSPI_CommandTypeDef qspi_cmd = {0};

  // 设置指令模式
  switch (cmd->mode) {
  case SPI_MODE_4_4_4:
    qspi_cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    qspi_cmd.AddressMode =
        (cmd->address_size > 0) ? QSPI_ADDRESS_4_LINES : QSPI_ADDRESS_NONE;
    qspi_cmd.DataMode = QSPI_DATA_4_LINES;
    break;
  case SPI_MODE_1_4_4:
    qspi_cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    qspi_cmd.AddressMode =
        (cmd->address_size > 0) ? QSPI_ADDRESS_4_LINES : QSPI_ADDRESS_NONE;
    qspi_cmd.DataMode = QSPI_DATA_4_LINES;
    break;
  case SPI_MODE_1_1_4:
    qspi_cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    qspi_cmd.AddressMode =
        (cmd->address_size > 0) ? QSPI_ADDRESS_1_LINE : QSPI_ADDRESS_NONE;
    qspi_cmd.DataMode = QSPI_DATA_4_LINES;
    break;
  default: // SPI_MODE_1_1_1
    qspi_cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    qspi_cmd.AddressMode =
        (cmd->address_size > 0) ? QSPI_ADDRESS_1_LINE : QSPI_ADDRESS_NONE;
    qspi_cmd.DataMode = QSPI_DATA_1_LINE;
    break;
  }

  qspi_cmd.Instruction = cmd->instruction;
  qspi_cmd.Address = cmd->address;

  // 地址大小
  switch (cmd->address_size) {
  case 1:
    qspi_cmd.AddressSize = QSPI_ADDRESS_8_BITS;
    break;
  case 2:
    qspi_cmd.AddressSize = QSPI_ADDRESS_16_BITS;
    break;
  case 3:
    qspi_cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    break;
  case 4:
    qspi_cmd.AddressSize = QSPI_ADDRESS_32_BITS;
    break;
  default:
    qspi_cmd.AddressMode = QSPI_ADDRESS_NONE;
    break;
  }

  // 交替字节
  if (cmd->alternate_size > 0) {
    qspi_cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_1_LINE;
    qspi_cmd.AlternateBytesSize = (cmd->alternate_size == 1)
                                      ? QSPI_ALTERNATE_BYTES_8_BITS
                                      : QSPI_ALTERNATE_BYTES_16_BITS;
    qspi_cmd.AlternateBytes = cmd->alternate_bytes[0];
  } else {
    qspi_cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  }

  qspi_cmd.DummyCycles = cmd->dummy_cycles;
  qspi_cmd.NbData = 0;
  qspi_cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
  qspi_cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  qspi_cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  HAL_StatusTypeDef status =
      HAL_QSPI_Command(priv->hqspi, &qspi_cmd, self->timeout_default);

  return (status == HAL_OK) ? SPI_OK : SPI_ERR_TRANSFER;
}

// QSPI 快速读取
static spi_error_t qspi_hw_fast_read(spi_adapter_t *self,
                                     const spi_command_t *cmd, uint8_t *data,
                                     uint32_t len, uint32_t timeout_ms) {
  if (self == NULL || self->priv == NULL || cmd == NULL || data == NULL) {
    return SPI_ERR_PARAM;
  }

  qspi_hw_priv_t *priv = (qspi_hw_priv_t *)self->priv;

  QSPI_CommandTypeDef qspi_cmd = {0};

  // 配置命令（类似 qspi_hw_command）
  qspi_cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  qspi_cmd.Instruction = cmd->instruction;

  if (cmd->address_size > 0) {
    qspi_cmd.AddressMode = (cmd->mode >= SPI_MODE_1_4_4) ? QSPI_ADDRESS_4_LINES
                                                         : QSPI_ADDRESS_1_LINE;
    qspi_cmd.Address = cmd->address;
    qspi_cmd.AddressSize =
        (cmd->address_size == 4) ? QSPI_ADDRESS_32_BITS : QSPI_ADDRESS_24_BITS;
  } else {
    qspi_cmd.AddressMode = QSPI_ADDRESS_NONE;
  }

  qspi_cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  qspi_cmd.DummyCycles = cmd->dummy_cycles;
  qspi_cmd.DataMode =
      (cmd->mode >= SPI_MODE_1_1_4) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
  qspi_cmd.NbData = len;
  qspi_cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
  qspi_cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  qspi_cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  HAL_StatusTypeDef status =
      HAL_QSPI_Command(priv->hqspi, &qspi_cmd, timeout_ms);
  if (status != HAL_OK) {
    return SPI_ERR_TRANSFER;
  }

  status = HAL_QSPI_Receive(priv->hqspi, data, timeout_ms);

  return (status == HAL_OK) ? SPI_OK : SPI_ERR_TRANSFER;
}

// QSPI 快速写入
static spi_error_t qspi_hw_fast_write(spi_adapter_t *self,
                                      const spi_command_t *cmd,
                                      const uint8_t *data, uint32_t len,
                                      uint32_t timeout_ms) {
  if (self == NULL || self->priv == NULL || cmd == NULL || data == NULL) {
    return SPI_ERR_PARAM;
  }

  qspi_hw_priv_t *priv = (qspi_hw_priv_t *)self->priv;

  QSPI_CommandTypeDef qspi_cmd = {0};

  qspi_cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  qspi_cmd.Instruction = cmd->instruction;

  if (cmd->address_size > 0) {
    qspi_cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    qspi_cmd.Address = cmd->address;
    qspi_cmd.AddressSize =
        (cmd->address_size == 4) ? QSPI_ADDRESS_32_BITS : QSPI_ADDRESS_24_BITS;
  } else {
    qspi_cmd.AddressMode = QSPI_ADDRESS_NONE;
  }

  qspi_cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  qspi_cmd.DummyCycles = 0;
  qspi_cmd.DataMode =
      (cmd->mode >= SPI_MODE_1_1_4) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
  qspi_cmd.NbData = len;
  qspi_cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
  qspi_cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  qspi_cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  HAL_StatusTypeDef status =
      HAL_QSPI_Command(priv->hqspi, &qspi_cmd, timeout_ms);
  if (status != HAL_OK) {
    return SPI_ERR_TRANSFER;
  }

  status = HAL_QSPI_Transmit(priv->hqspi, (uint8_t *)data, timeout_ms);

  return (status == HAL_OK) ? SPI_OK : SPI_ERR_TRANSFER;
}

// 操作表
static const spi_adapter_ops_t g_qspi_hw_ops = {
    .init = qspi_hw_init,
    .deinit = qspi_hw_deinit,
    .select = qspi_hw_select,
    .deselect = qspi_hw_deselect,
    .transmit = qspi_hw_transmit,
    .receive = qspi_hw_receive,
    .transmit_receive = qspi_hw_transmit_receive,
    .command = qspi_hw_command,
    .fast_read = qspi_hw_fast_read,
    .fast_write = qspi_hw_fast_write,
};

// 静态实例
static spi_adapter_t g_qspi_hw_adapter;
static qspi_hw_priv_t g_qspi_hw_priv;
static uint8_t g_qspi_adapter_created = 0;

// ============== 公共接口 ==============

const spi_adapter_ops_t *qspi_hardware_adapter_get_ops(void) {
  return &g_qspi_hw_ops;
}

spi_adapter_t *
qspi_hardware_adapter_create(const qspi_hw_adapter_config_t *config) {
  if (config == NULL || config->hqspi == NULL) {
    return NULL;
  }

  if (g_qspi_adapter_created) {
    return &g_qspi_hw_adapter;
  }

  // 初始化私有数据
  memset(&g_qspi_hw_priv, 0, sizeof(g_qspi_hw_priv));
  g_qspi_hw_priv.hqspi = config->hqspi;
  g_qspi_hw_priv.flash_size = config->flash_size;
  g_qspi_hw_priv.use_4byte_addr = config->use_4byte_addr;

  // 初始化适配器
  memset(&g_qspi_hw_adapter, 0, sizeof(g_qspi_hw_adapter));
  g_qspi_hw_adapter.ops = &g_qspi_hw_ops;
  g_qspi_hw_adapter.type = SPI_TYPE_QSPI;
  g_qspi_hw_adapter.timeout_default =
      config->timeout_ms ? config->timeout_ms : 1000;
  g_qspi_hw_adapter.priv = &g_qspi_hw_priv;

  g_qspi_adapter_created = 1;

  return &g_qspi_hw_adapter;
}

void qspi_hardware_adapter_destroy(spi_adapter_t *adapter) {
  if (adapter == &g_qspi_hw_adapter) {
    g_qspi_adapter_created = 0;
  }
}
