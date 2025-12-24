/*
 * w25q256_driver.c
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  W25Q256 Flash 驱动实现
 */

#include "w25q256_driver.h"
#include "w25qxxx_common.h"
#include <string.h>

// ============== 驱动操作实现 ==============

static flash_error_t w25q256_init(flash_driver_t *self) {
  if (self == NULL || self->priv == NULL) {
    return FLASH_ERR_PARAM;
  }

  w25qxxx_priv_t *priv = (w25qxxx_priv_t *)self->priv;

  // 初始化 SPI
  if (priv->spi->ops->init) {
    SPI_INIT(priv->spi);
  }

  // 读取 JEDEC ID
  uint32_t jedec_id = w25qxxx_read_jedec_id(priv);

  // 识别芯片
  w25q_chip_type_t chip_type = w25qxxx_identify_chip(jedec_id);

  if (chip_type != W25Q_TYPE_W25Q256) {
    FLASH_LOG_W(priv->deps, "Expected W25Q256, got ID: 0x%06X", jedec_id);
    // 仍继续，但设置为识别到的类型
  }

  priv->chip_type = chip_type;

  // 填充芯片信息
  w25qxxx_fill_info(&self->info, W25Q_TYPE_W25Q256, jedec_id);

  // 进入4字节地址模式（W25Q256必需）
  flash_error_t err = w25qxxx_enter_4b_mode(priv);
  if (err != FLASH_OK) {
    FLASH_LOG_E(priv->deps, "Failed to enter 4-byte address mode");
    return err;
  }

  self->initialized = 1;

  FLASH_LOG_I(priv->deps, "W25Q256 initialized, ID: 0x%06X, Size: %luMB",
              jedec_id, self->info.capacity / (1024 * 1024));

  return FLASH_OK;
}

static flash_error_t w25q256_deinit(flash_driver_t *self) {
  if (self == NULL) {
    return FLASH_ERR_PARAM;
  }

  self->initialized = 0;
  return FLASH_OK;
}

static flash_error_t w25q256_read(flash_driver_t *self, uint32_t addr,
                                  uint8_t *buf, uint32_t len) {
  if (self == NULL || self->priv == NULL || buf == NULL || len == 0) {
    return FLASH_ERR_PARAM;
  }

  if (!self->initialized) {
    return FLASH_ERR_NOT_INIT;
  }

  w25qxxx_priv_t *priv = (w25qxxx_priv_t *)self->priv;

  // 检查地址范围
  if (addr + len > self->info.capacity) {
    return FLASH_ERR_ADDR;
  }

  // W25Q256 使用4字节地址
  return w25qxxx_read_data_4b(priv, addr, buf, len);
}

static flash_error_t w25q256_write(flash_driver_t *self, uint32_t addr,
                                   const uint8_t *buf, uint32_t len) {
  if (self == NULL || self->priv == NULL || buf == NULL || len == 0) {
    return FLASH_ERR_PARAM;
  }

  if (!self->initialized) {
    return FLASH_ERR_NOT_INIT;
  }

  w25qxxx_priv_t *priv = (w25qxxx_priv_t *)self->priv;

  // 检查地址范围
  if (addr + len > self->info.capacity) {
    return FLASH_ERR_ADDR;
  }

  // 分页写入
  uint32_t written = 0;
  while (written < len) {
    uint32_t current_addr = addr + written;
    uint32_t page_offset = current_addr % 256;
    uint32_t to_write = 256 - page_offset;

    if (to_write > (len - written)) {
      to_write = len - written;
    }

    flash_error_t err =
        w25qxxx_page_program_4b(priv, current_addr, buf + written, to_write);
    if (err != FLASH_OK) {
      return err;
    }

    written += to_write;
  }

  return FLASH_OK;
}

static flash_error_t w25q256_erase_sector(flash_driver_t *self, uint32_t addr) {
  if (self == NULL || self->priv == NULL) {
    return FLASH_ERR_PARAM;
  }

  if (!self->initialized) {
    return FLASH_ERR_NOT_INIT;
  }

  w25qxxx_priv_t *priv = (w25qxxx_priv_t *)self->priv;

  // 对齐到扇区边界
  addr = (addr / 4096) * 4096;

  if (addr >= self->info.capacity) {
    return FLASH_ERR_ADDR;
  }

  return w25qxxx_sector_erase_4b(priv, addr);
}

static flash_error_t w25q256_erase_block(flash_driver_t *self, uint32_t addr) {
  if (self == NULL || self->priv == NULL) {
    return FLASH_ERR_PARAM;
  }

  if (!self->initialized) {
    return FLASH_ERR_NOT_INIT;
  }

  w25qxxx_priv_t *priv = (w25qxxx_priv_t *)self->priv;

  // 对齐到块边界
  addr = (addr / 65536) * 65536;

  if (addr >= self->info.capacity) {
    return FLASH_ERR_ADDR;
  }

  return w25qxxx_block_erase_4b(priv, addr);
}

static flash_error_t w25q256_erase_chip(flash_driver_t *self) {
  if (self == NULL || self->priv == NULL) {
    return FLASH_ERR_PARAM;
  }

  if (!self->initialized) {
    return FLASH_ERR_NOT_INIT;
  }

  w25qxxx_priv_t *priv = (w25qxxx_priv_t *)self->priv;

  FLASH_LOG_I(priv->deps,
              "Chip erase started, this may take several minutes...");

  return w25qxxx_chip_erase(priv);
}

static int w25q256_is_busy(flash_driver_t *self) {
  if (self == NULL || self->priv == NULL) {
    return 1;
  }

  w25qxxx_priv_t *priv = (w25qxxx_priv_t *)self->priv;
  uint8_t sr = w25qxxx_read_sr1(priv);

  return (sr & W25X_SR1_BUSY) ? 1 : 0;
}

static uint32_t w25q256_read_id(flash_driver_t *self) {
  if (self == NULL || self->priv == NULL) {
    return 0;
  }

  w25qxxx_priv_t *priv = (w25qxxx_priv_t *)self->priv;
  return w25qxxx_read_jedec_id(priv);
}

static const flash_info_t *w25q256_get_info(flash_driver_t *self) {
  if (self == NULL) {
    return NULL;
  }
  return &self->info;
}

static flash_error_t w25q256_power_down(flash_driver_t *self) {
  if (self == NULL || self->priv == NULL) {
    return FLASH_ERR_PARAM;
  }

  w25qxxx_priv_t *priv = (w25qxxx_priv_t *)self->priv;
  return w25qxxx_power_down(priv);
}

static flash_error_t w25q256_power_up(flash_driver_t *self) {
  if (self == NULL || self->priv == NULL) {
    return FLASH_ERR_PARAM;
  }

  w25qxxx_priv_t *priv = (w25qxxx_priv_t *)self->priv;
  return w25qxxx_power_up(priv);
}

static flash_error_t w25q256_write_protect(flash_driver_t *self, int enable) {
  // TODO: 实现写保护控制
  (void)self;
  (void)enable;
  return FLASH_OK;
}

// ============== 操作表 ==============

static const flash_driver_ops_t g_w25q256_ops = {
    .init = w25q256_init,
    .deinit = w25q256_deinit,
    .read = w25q256_read,
    .write = w25q256_write,
    .erase_sector = w25q256_erase_sector,
    .erase_block = w25q256_erase_block,
    .erase_chip = w25q256_erase_chip,
    .is_busy = w25q256_is_busy,
    .read_id = w25q256_read_id,
    .get_info = w25q256_get_info,
    .power_down = w25q256_power_down,
    .power_up = w25q256_power_up,
    .write_protect = w25q256_write_protect,
};

// ============== 公共接口 ==============

const flash_driver_ops_t *w25q256_get_ops(void) { return &g_w25q256_ops; }

// 静态实例（用于单例场景）
static flash_driver_t g_w25q256_driver;
static w25qxxx_priv_t g_w25q256_priv;
static uint8_t g_driver_created = 0;

flash_driver_t *w25q256_driver_create(spi_adapter_t *spi,
                                      flash_dependencies_t *deps) {
  if (spi == NULL) {
    return NULL;
  }

  if (g_driver_created) {
    // 已创建，返回现有实例
    return &g_w25q256_driver;
  }

  // 初始化私有数据
  memset(&g_w25q256_priv, 0, sizeof(g_w25q256_priv));
  g_w25q256_priv.spi = spi;
  g_w25q256_priv.deps = deps;
  g_w25q256_priv.addr_mode = 4; // W25Q256 使用4字节地址
  g_w25q256_priv.use_qspi = (spi->type == SPI_TYPE_QSPI) ? 1 : 0;

  // 初始化驱动
  memset(&g_w25q256_driver, 0, sizeof(g_w25q256_driver));
  g_w25q256_driver.ops = &g_w25q256_ops;
  g_w25q256_driver.priv = &g_w25q256_priv;
  strncpy(g_w25q256_driver.name, "W25Q256", sizeof(g_w25q256_driver.name) - 1);

  g_driver_created = 1;

  return &g_w25q256_driver;
}

void w25q256_driver_destroy(flash_driver_t *driver) {
  if (driver == &g_w25q256_driver) {
    if (driver->initialized) {
      FLASH_DEINIT(driver);
    }
    g_driver_created = 0;
  }
}
