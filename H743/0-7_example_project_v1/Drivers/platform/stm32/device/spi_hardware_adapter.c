/*
 * spi_hardware_adapter.c
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  STM32 标准 SPI 硬件适配器实现
 */

#include "spi_hardware_adapter.h"
#include <string.h>

// 私有数据
typedef struct {
  SPI_HandleTypeDef *hspi;
  GPIO_TypeDef *cs_port;
  uint16_t cs_pin;
} spi_hw_priv_t;

// ============== 操作实现 ==============

static spi_error_t spi_hw_init(spi_adapter_t *self) {
  // HAL SPI 通常已在 CubeMX 中初始化
  (void)self;
  return SPI_OK;
}

static spi_error_t spi_hw_deinit(spi_adapter_t *self) {
  (void)self;
  return SPI_OK;
}

static void spi_hw_select(spi_adapter_t *self) {
  if (self == NULL || self->priv == NULL)
    return;

  spi_hw_priv_t *priv = (spi_hw_priv_t *)self->priv;
  HAL_GPIO_WritePin(priv->cs_port, priv->cs_pin, GPIO_PIN_RESET);
}

static void spi_hw_deselect(spi_adapter_t *self) {
  if (self == NULL || self->priv == NULL)
    return;

  spi_hw_priv_t *priv = (spi_hw_priv_t *)self->priv;
  HAL_GPIO_WritePin(priv->cs_port, priv->cs_pin, GPIO_PIN_SET);
}

static spi_error_t spi_hw_transmit(spi_adapter_t *self, const uint8_t *data,
                                   uint32_t len, uint32_t timeout_ms) {
  if (self == NULL || self->priv == NULL || data == NULL) {
    return SPI_ERR_PARAM;
  }

  spi_hw_priv_t *priv = (spi_hw_priv_t *)self->priv;

  HAL_StatusTypeDef status =
      HAL_SPI_Transmit(priv->hspi, (uint8_t *)data, len, timeout_ms);

  if (status == HAL_OK) {
    return SPI_OK;
  } else if (status == HAL_TIMEOUT) {
    return SPI_ERR_TIMEOUT;
  } else if (status == HAL_BUSY) {
    return SPI_ERR_BUSY;
  }

  return SPI_ERR_TRANSFER;
}

static spi_error_t spi_hw_receive(spi_adapter_t *self, uint8_t *data,
                                  uint32_t len, uint32_t timeout_ms) {
  if (self == NULL || self->priv == NULL || data == NULL) {
    return SPI_ERR_PARAM;
  }

  spi_hw_priv_t *priv = (spi_hw_priv_t *)self->priv;

  HAL_StatusTypeDef status = HAL_SPI_Receive(priv->hspi, data, len, timeout_ms);

  if (status == HAL_OK) {
    return SPI_OK;
  } else if (status == HAL_TIMEOUT) {
    return SPI_ERR_TIMEOUT;
  } else if (status == HAL_BUSY) {
    return SPI_ERR_BUSY;
  }

  return SPI_ERR_TRANSFER;
}

static spi_error_t spi_hw_transmit_receive(spi_adapter_t *self,
                                           const uint8_t *tx_data,
                                           uint8_t *rx_data, uint32_t len,
                                           uint32_t timeout_ms) {
  if (self == NULL || self->priv == NULL) {
    return SPI_ERR_PARAM;
  }

  spi_hw_priv_t *priv = (spi_hw_priv_t *)self->priv;

  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(
      priv->hspi, (uint8_t *)tx_data, rx_data, len, timeout_ms);

  if (status == HAL_OK) {
    return SPI_OK;
  } else if (status == HAL_TIMEOUT) {
    return SPI_ERR_TIMEOUT;
  } else if (status == HAL_BUSY) {
    return SPI_ERR_BUSY;
  }

  return SPI_ERR_TRANSFER;
}

// 操作表
static const spi_adapter_ops_t g_spi_hw_ops = {
    .init = spi_hw_init,
    .deinit = spi_hw_deinit,
    .select = spi_hw_select,
    .deselect = spi_hw_deselect,
    .transmit = spi_hw_transmit,
    .receive = spi_hw_receive,
    .transmit_receive = spi_hw_transmit_receive,
    .command = NULL, // 标准SPI不支持
    .fast_read = NULL,
    .fast_write = NULL,
};

// 静态实例
static spi_adapter_t g_spi_hw_adapter;
static spi_hw_priv_t g_spi_hw_priv;
static uint8_t g_adapter_created = 0;

// ============== 公共接口 ==============

const spi_adapter_ops_t *spi_hardware_adapter_get_ops(void) {
  return &g_spi_hw_ops;
}

spi_adapter_t *
spi_hardware_adapter_create(const spi_hw_adapter_config_t *config) {
  if (config == NULL || config->hspi == NULL) {
    return NULL;
  }

  if (g_adapter_created) {
    return &g_spi_hw_adapter;
  }

  // 初始化私有数据
  memset(&g_spi_hw_priv, 0, sizeof(g_spi_hw_priv));
  g_spi_hw_priv.hspi = config->hspi;
  g_spi_hw_priv.cs_port = config->cs_port;
  g_spi_hw_priv.cs_pin = config->cs_pin;

  // 初始化适配器
  memset(&g_spi_hw_adapter, 0, sizeof(g_spi_hw_adapter));
  g_spi_hw_adapter.ops = &g_spi_hw_ops;
  g_spi_hw_adapter.type = SPI_TYPE_STANDARD;
  g_spi_hw_adapter.timeout_default =
      config->timeout_ms ? config->timeout_ms : 1000;
  g_spi_hw_adapter.priv = &g_spi_hw_priv;

  // 确保 CS 初始为高
  if (config->cs_port) {
    HAL_GPIO_WritePin(config->cs_port, config->cs_pin, GPIO_PIN_SET);
  }

  g_adapter_created = 1;

  return &g_spi_hw_adapter;
}

void spi_hardware_adapter_destroy(spi_adapter_t *adapter) {
  if (adapter == &g_spi_hw_adapter) {
    g_adapter_created = 0;
  }
}
