/*
 * w25qxxx_common.c
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  W25Qxxx 系列 Flash 通用驱动实现
 */

#include "w25qxxx_common.h"
#include <string.h>

// ============== 芯片识别 ==============

w25q_chip_type_t w25qxxx_identify_chip(uint32_t jedec_id) {
  uint8_t manufacturer = (jedec_id >> 16) & 0xFF;
  uint8_t memory_type = (jedec_id >> 8) & 0xFF;
  uint8_t capacity_id = jedec_id & 0xFF;

  // 检查是否为 Winbond
  if (manufacturer != W25X_MANUFACTURER_WINBOND) {
    return W25Q_TYPE_UNKNOWN;
  }

  // 检查内存类型（0x40 = W25Q系列）
  if (memory_type != 0x40) {
    return W25Q_TYPE_UNKNOWN;
  }

  // 根据容量ID识别型号
  switch (capacity_id) {
  case 0x15:
    return W25Q_TYPE_W25Q16; // 2MB
  case 0x16:
    return W25Q_TYPE_W25Q32; // 4MB
  case 0x17:
    return W25Q_TYPE_W25Q64; // 8MB
  case 0x18:
    return W25Q_TYPE_W25Q128; // 16MB
  case 0x19:
    return W25Q_TYPE_W25Q256; // 32MB
  case 0x20:
    return W25Q_TYPE_W25Q512; // 64MB
  default:
    return W25Q_TYPE_UNKNOWN;
  }
}

uint32_t w25qxxx_get_capacity(w25q_chip_type_t type) {
  switch (type) {
  case W25Q_TYPE_W25Q16:
    return 2 * 1024 * 1024;
  case W25Q_TYPE_W25Q32:
    return 4 * 1024 * 1024;
  case W25Q_TYPE_W25Q64:
    return 8 * 1024 * 1024;
  case W25Q_TYPE_W25Q128:
    return 16 * 1024 * 1024;
  case W25Q_TYPE_W25Q256:
    return 32 * 1024 * 1024;
  case W25Q_TYPE_W25Q512:
    return 64 * 1024 * 1024;
  default:
    return 0;
  }
}

const char *w25qxxx_get_name(w25q_chip_type_t type) {
  switch (type) {
  case W25Q_TYPE_W25Q16:
    return "W25Q16";
  case W25Q_TYPE_W25Q32:
    return "W25Q32";
  case W25Q_TYPE_W25Q64:
    return "W25Q64";
  case W25Q_TYPE_W25Q128:
    return "W25Q128";
  case W25Q_TYPE_W25Q256:
    return "W25Q256";
  case W25Q_TYPE_W25Q512:
    return "W25Q512";
  default:
    return "Unknown";
  }
}

void w25qxxx_fill_info(flash_info_t *info, w25q_chip_type_t type,
                       uint32_t jedec_id) {
  if (info == NULL)
    return;

  info->jedec_id = jedec_id;
  info->capacity = w25qxxx_get_capacity(type);
  info->sector_size = 4096; // 4KB
  info->block_size = 65536; // 64KB
  info->page_size = 256;    // 256 bytes
  info->sector_count = info->capacity / info->sector_size;

  // W25Q256及以上使用4字节地址
  info->addr_mode = (type >= W25Q_TYPE_W25Q256) ? 4 : 3;
  info->name = w25qxxx_get_name(type);
}

// ============== 底层SPI操作 ==============

flash_error_t w25qxxx_send_cmd(w25qxxx_priv_t *priv, uint8_t cmd) {
  if (priv == NULL || priv->spi == NULL) {
    return FLASH_ERR_PARAM;
  }

  SPI_SELECT(priv->spi);
  spi_error_t err =
      SPI_TRANSMIT(priv->spi, &cmd, 1, priv->spi->timeout_default);
  SPI_DESELECT(priv->spi);

  return (err == SPI_OK) ? FLASH_OK : FLASH_ERR_WRITE;
}

uint8_t w25qxxx_read_sr1(w25qxxx_priv_t *priv) {
  if (priv == NULL || priv->spi == NULL) {
    return 0xFF;
  }

  uint8_t cmd = W25X_CMD_READ_SR1;
  uint8_t sr;

  SPI_SELECT(priv->spi);
  SPI_TRANSMIT(priv->spi, &cmd, 1, priv->spi->timeout_default);
  SPI_RECEIVE(priv->spi, &sr, 1, priv->spi->timeout_default);
  SPI_DESELECT(priv->spi);

  return sr;
}

flash_error_t w25qxxx_write_enable(w25qxxx_priv_t *priv) {
  return w25qxxx_send_cmd(priv, W25X_CMD_WRITE_ENABLE);
}

flash_error_t w25qxxx_wait_busy(w25qxxx_priv_t *priv, uint32_t timeout_ms) {
  if (priv == NULL) {
    return FLASH_ERR_PARAM;
  }

  uint32_t start = 0;
  uint32_t elapsed = 0;

  while (elapsed < timeout_ms) {
    uint8_t sr = w25qxxx_read_sr1(priv);
    if ((sr & W25X_SR1_BUSY) == 0) {
      return FLASH_OK;
    }

    FLASH_DELAY_MS(priv->deps, 1);
    elapsed++;
  }

  return FLASH_ERR_TIMEOUT;
}

uint32_t w25qxxx_read_jedec_id(w25qxxx_priv_t *priv) {
  if (priv == NULL || priv->spi == NULL) {
    return 0;
  }

  uint8_t cmd = W25X_CMD_READ_JEDEC_ID;
  uint8_t id[3];

  SPI_SELECT(priv->spi);
  SPI_TRANSMIT(priv->spi, &cmd, 1, priv->spi->timeout_default);
  SPI_RECEIVE(priv->spi, id, 3, priv->spi->timeout_default);
  SPI_DESELECT(priv->spi);

  return ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | id[2];
}

// ============== 读取操作 ==============

flash_error_t w25qxxx_read_data_3b(w25qxxx_priv_t *priv, uint32_t addr,
                                   uint8_t *buf, uint32_t len) {
  if (priv == NULL || priv->spi == NULL || buf == NULL) {
    return FLASH_ERR_PARAM;
  }

  uint8_t cmd[4];
  cmd[0] = W25X_CMD_READ_DATA;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  SPI_SELECT(priv->spi);
  spi_error_t err = SPI_TRANSMIT(priv->spi, cmd, 4, priv->spi->timeout_default);
  if (err == SPI_OK) {
    err = SPI_RECEIVE(priv->spi, buf, len, priv->spi->timeout_default);
  }
  SPI_DESELECT(priv->spi);

  return (err == SPI_OK) ? FLASH_OK : FLASH_ERR_READ;
}

flash_error_t w25qxxx_read_data_4b(w25qxxx_priv_t *priv, uint32_t addr,
                                   uint8_t *buf, uint32_t len) {
  if (priv == NULL || priv->spi == NULL || buf == NULL) {
    return FLASH_ERR_PARAM;
  }

  uint8_t cmd[5];
  cmd[0] = W25X_CMD_READ_DATA_4B;
  cmd[1] = (addr >> 24) & 0xFF;
  cmd[2] = (addr >> 16) & 0xFF;
  cmd[3] = (addr >> 8) & 0xFF;
  cmd[4] = addr & 0xFF;

  SPI_SELECT(priv->spi);
  spi_error_t err = SPI_TRANSMIT(priv->spi, cmd, 5, priv->spi->timeout_default);
  if (err == SPI_OK) {
    err = SPI_RECEIVE(priv->spi, buf, len, priv->spi->timeout_default);
  }
  SPI_DESELECT(priv->spi);

  return (err == SPI_OK) ? FLASH_OK : FLASH_ERR_READ;
}

// ============== 写入操作 ==============

flash_error_t w25qxxx_page_program_3b(w25qxxx_priv_t *priv, uint32_t addr,
                                      const uint8_t *buf, uint32_t len) {
  if (priv == NULL || priv->spi == NULL || buf == NULL) {
    return FLASH_ERR_PARAM;
  }

  // 限制不超过页边界
  uint32_t page_offset = addr % 256;
  if (len > (256 - page_offset)) {
    len = 256 - page_offset;
  }

  // 写使能
  flash_error_t err = w25qxxx_write_enable(priv);
  if (err != FLASH_OK)
    return err;

  uint8_t cmd[4];
  cmd[0] = W25X_CMD_PAGE_PROGRAM;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  SPI_SELECT(priv->spi);
  spi_error_t spi_err =
      SPI_TRANSMIT(priv->spi, cmd, 4, priv->spi->timeout_default);
  if (spi_err == SPI_OK) {
    spi_err = SPI_TRANSMIT(priv->spi, buf, len, priv->spi->timeout_default);
  }
  SPI_DESELECT(priv->spi);

  if (spi_err != SPI_OK) {
    return FLASH_ERR_WRITE;
  }

  // 等待写入完成
  return w25qxxx_wait_busy(priv, 10); // 页编程典型3ms
}

flash_error_t w25qxxx_page_program_4b(w25qxxx_priv_t *priv, uint32_t addr,
                                      const uint8_t *buf, uint32_t len) {
  if (priv == NULL || priv->spi == NULL || buf == NULL) {
    return FLASH_ERR_PARAM;
  }

  // 限制不超过页边界
  uint32_t page_offset = addr % 256;
  if (len > (256 - page_offset)) {
    len = 256 - page_offset;
  }

  // 写使能
  flash_error_t err = w25qxxx_write_enable(priv);
  if (err != FLASH_OK)
    return err;

  uint8_t cmd[5];
  cmd[0] = W25X_CMD_PAGE_PROGRAM_4B;
  cmd[1] = (addr >> 24) & 0xFF;
  cmd[2] = (addr >> 16) & 0xFF;
  cmd[3] = (addr >> 8) & 0xFF;
  cmd[4] = addr & 0xFF;

  SPI_SELECT(priv->spi);
  spi_error_t spi_err =
      SPI_TRANSMIT(priv->spi, cmd, 5, priv->spi->timeout_default);
  if (spi_err == SPI_OK) {
    spi_err = SPI_TRANSMIT(priv->spi, buf, len, priv->spi->timeout_default);
  }
  SPI_DESELECT(priv->spi);

  if (spi_err != SPI_OK) {
    return FLASH_ERR_WRITE;
  }

  // 等待写入完成
  return w25qxxx_wait_busy(priv, 10);
}

// ============== 擦除操作 ==============

flash_error_t w25qxxx_sector_erase_3b(w25qxxx_priv_t *priv, uint32_t addr) {
  if (priv == NULL || priv->spi == NULL) {
    return FLASH_ERR_PARAM;
  }

  flash_error_t err = w25qxxx_write_enable(priv);
  if (err != FLASH_OK)
    return err;

  uint8_t cmd[4];
  cmd[0] = W25X_CMD_SECTOR_ERASE;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  SPI_SELECT(priv->spi);
  spi_error_t spi_err =
      SPI_TRANSMIT(priv->spi, cmd, 4, priv->spi->timeout_default);
  SPI_DESELECT(priv->spi);

  if (spi_err != SPI_OK) {
    return FLASH_ERR_ERASE;
  }

  // 等待擦除完成（扇区擦除典型45ms，最大400ms）
  return w25qxxx_wait_busy(priv, 500);
}

flash_error_t w25qxxx_sector_erase_4b(w25qxxx_priv_t *priv, uint32_t addr) {
  if (priv == NULL || priv->spi == NULL) {
    return FLASH_ERR_PARAM;
  }

  flash_error_t err = w25qxxx_write_enable(priv);
  if (err != FLASH_OK)
    return err;

  uint8_t cmd[5];
  cmd[0] = W25X_CMD_SECTOR_ERASE_4B;
  cmd[1] = (addr >> 24) & 0xFF;
  cmd[2] = (addr >> 16) & 0xFF;
  cmd[3] = (addr >> 8) & 0xFF;
  cmd[4] = addr & 0xFF;

  SPI_SELECT(priv->spi);
  spi_error_t spi_err =
      SPI_TRANSMIT(priv->spi, cmd, 5, priv->spi->timeout_default);
  SPI_DESELECT(priv->spi);

  if (spi_err != SPI_OK) {
    return FLASH_ERR_ERASE;
  }

  return w25qxxx_wait_busy(priv, 500);
}

flash_error_t w25qxxx_block_erase_3b(w25qxxx_priv_t *priv, uint32_t addr) {
  if (priv == NULL || priv->spi == NULL) {
    return FLASH_ERR_PARAM;
  }

  flash_error_t err = w25qxxx_write_enable(priv);
  if (err != FLASH_OK)
    return err;

  uint8_t cmd[4];
  cmd[0] = W25X_CMD_BLOCK_ERASE_64K;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  SPI_SELECT(priv->spi);
  spi_error_t spi_err =
      SPI_TRANSMIT(priv->spi, cmd, 4, priv->spi->timeout_default);
  SPI_DESELECT(priv->spi);

  if (spi_err != SPI_OK) {
    return FLASH_ERR_ERASE;
  }

  // 块擦除典型150ms，最大2s
  return w25qxxx_wait_busy(priv, 3000);
}

flash_error_t w25qxxx_block_erase_4b(w25qxxx_priv_t *priv, uint32_t addr) {
  if (priv == NULL || priv->spi == NULL) {
    return FLASH_ERR_PARAM;
  }

  flash_error_t err = w25qxxx_write_enable(priv);
  if (err != FLASH_OK)
    return err;

  uint8_t cmd[5];
  cmd[0] = W25X_CMD_BLOCK_ERASE_64K_4B;
  cmd[1] = (addr >> 24) & 0xFF;
  cmd[2] = (addr >> 16) & 0xFF;
  cmd[3] = (addr >> 8) & 0xFF;
  cmd[4] = addr & 0xFF;

  SPI_SELECT(priv->spi);
  spi_error_t spi_err =
      SPI_TRANSMIT(priv->spi, cmd, 5, priv->spi->timeout_default);
  SPI_DESELECT(priv->spi);

  if (spi_err != SPI_OK) {
    return FLASH_ERR_ERASE;
  }

  return w25qxxx_wait_busy(priv, 3000);
}

flash_error_t w25qxxx_chip_erase(w25qxxx_priv_t *priv) {
  if (priv == NULL || priv->spi == NULL) {
    return FLASH_ERR_PARAM;
  }

  flash_error_t err = w25qxxx_write_enable(priv);
  if (err != FLASH_OK)
    return err;

  err = w25qxxx_send_cmd(priv, W25X_CMD_CHIP_ERASE);
  if (err != FLASH_OK) {
    return FLASH_ERR_ERASE;
  }

  // 全片擦除时间较长（32MB大约40-400s）
  return w25qxxx_wait_busy(priv, 600000); // 10分钟超时
}

// ============== 地址模式切换 ==============

flash_error_t w25qxxx_enter_4b_mode(w25qxxx_priv_t *priv) {
  flash_error_t err = w25qxxx_send_cmd(priv, W25X_CMD_ENTER_4B_MODE);
  if (err == FLASH_OK) {
    priv->addr_mode = 4;
  }
  return err;
}

flash_error_t w25qxxx_exit_4b_mode(w25qxxx_priv_t *priv) {
  flash_error_t err = w25qxxx_send_cmd(priv, W25X_CMD_EXIT_4B_MODE);
  if (err == FLASH_OK) {
    priv->addr_mode = 3;
  }
  return err;
}

// ============== 电源管理 ==============

flash_error_t w25qxxx_power_down(w25qxxx_priv_t *priv) {
  return w25qxxx_send_cmd(priv, W25X_CMD_POWER_DOWN);
}

flash_error_t w25qxxx_power_up(w25qxxx_priv_t *priv) {
  flash_error_t err = w25qxxx_send_cmd(priv, W25X_CMD_RELEASE_POWER_DOWN);
  if (err == FLASH_OK) {
    // 等待芯片唤醒（典型3us）
    FLASH_DELAY_US(priv->deps, 5);
  }
  return err;
}
