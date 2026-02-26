/*
 * w25q_driver.c
 *
 *  Created on: Feb 19, 2026
 *      Author: Antigravity
 */

#include "w25q_driver.h"
#include "MemPool.h"
#include <string.h>

// W25Q 指令集
#define W25Q_WRITE_ENABLE 0x06
#define W25Q_READ_STATUS_REG1 0x05
#define W25Q_READ_STATUS_REG2 0x35
#define W25Q_WRITE_STATUS_REG1 0x01
#define W25Q_WRITE_STATUS_REG2 0x31
#define W25Q_PAGE_PROGRAM 0x02
#define W25Q_QUAD_PAGE_PROGRAM 0x32
#define W25Q_SECTOR_ERASE 0x20
#define W25Q_CHIP_ERASE 0xC7
#define W25Q_READ_DATA 0x03
#define W25Q_FAST_READ 0x0B
#define W25Q_QUAD_OUT_READ 0x6B
#define W25Q_QUAD_IO_READ 0xEB
#define W25Q_READ_ID 0x9F
#define W25Q_READ_SFDP 0x5A

// W25Q 4-byte 指令集 (32MB及以上容量使用)
#define W25Q_QUAD_IO_READ_4B 0xEC
#define W25Q_QUAD_PAGE_PROGRAM_4B 0x34
#define W25Q_SECTOR_ERASE_4B 0x21

/**
 * @brief 等待忙标志失效
 */
static int w25q_wait_busy(w25q_driver_t *drv, uint32_t timeout) {
  uint8_t status = 0;
  qspi_command_t cmd = {0};
  cmd.Instruction = W25Q_READ_STATUS_REG1;
  cmd.InstructionMode = QSPI_DRV_INSTR_1_LINE;
  cmd.DataMode = QSPI_DRV_DATA_1_LINE;
  cmd.NbData = 1;

  uint32_t start_tick = sys_get_systick_ms();
  do {
    if (QSPI_RECEIVE(drv->qspi, &cmd, &status, 10) != 0)
      return -1;
    if (!(status & 0x01))
      return 0; // WIP bit clear
  } while (sys_get_systick_ms() - start_tick < timeout);

  return -2; // Timeout
}

/**
 * @brief 写使能
 */
static int w25q_write_enable(w25q_driver_t *drv) {
  qspi_command_t cmd = {0};
  cmd.Instruction = W25Q_WRITE_ENABLE;
  cmd.InstructionMode = QSPI_DRV_INSTR_1_LINE;
  return QSPI_COMMAND(drv->qspi, &cmd, 10);
}

/**
 * @brief 开启 Quad 模式 (设置 Status Register 2 的 QE 位)
 */
static int w25q_enable_quad_mode(w25q_driver_t *drv) {
  uint8_t status2 = 0;
  qspi_command_t cmd = {0};

  // 1. 读取 Status Register 2
  cmd.Instruction = W25Q_READ_STATUS_REG2;
  cmd.InstructionMode = QSPI_DRV_INSTR_1_LINE;
  cmd.DataMode = QSPI_DRV_DATA_1_LINE;
  cmd.NbData = 1;
  if (QSPI_RECEIVE(drv->qspi, &cmd, &status2, 100) != 0)
    return -1;

  // 2. 如果 QE 位 (Bit 1) 已经为 1，则返回
  if (status2 & 0x02)
    return 0;

  // 3. 写使能
  if (w25q_write_enable(drv) != 0)
    return -2;

  // 4. 写入 Status Register 2 (置位 QE)
  status2 |= 0x02;
  cmd.Instruction = W25Q_WRITE_STATUS_REG2;
  cmd.NbData = 1;
  if (QSPI_TRANSMIT(drv->qspi, &cmd, &status2, 100) != 0)
    return -3;

  return w25q_wait_busy(drv, 100);
}

/**
 * @brief 读取 SFDP 参数 (0x5A)
 */
static int w25q_probe_sfdp(w25q_driver_t *drv) {
  uint8_t sfdp_header[16];
  qspi_command_t cmd = {0};

  // 读取 SFDP 头
  cmd.Instruction = W25Q_READ_SFDP;
  cmd.InstructionMode = QSPI_DRV_INSTR_1_LINE;
  cmd.AddressMode = QSPI_DRV_ADDR_1_LINE;
  cmd.AddressSize = QSPI_DRV_ADDR_24BITS;
  cmd.Address = 0;
  cmd.DummyCycles = 8;
  cmd.DataMode = QSPI_DRV_DATA_1_LINE;
  cmd.NbData = 16;

  if (QSPI_RECEIVE(drv->qspi, &cmd, sfdp_header, 100) != 0)
    return -1;

  // 检查 Magic 'SFDP'
  if (memcmp(sfdp_header, "SFDP", 4) != 0)
    return -2;

  // 简化处理：对于 W25Q，默认使用 4K Sector, 256B Page
  drv->base.info.sector_size = 4096;
  drv->base.info.page_size = 256;
  drv->base.info.block_size = 65536;

  // 读取 JEDEC ID
  cmd.Instruction = W25Q_READ_ID;
  cmd.AddressMode = QSPI_DRV_ADDR_NONE;
  cmd.DummyCycles = 0;
  cmd.NbData = 3;
  if (QSPI_RECEIVE(drv->qspi, &cmd, drv->base.info.device_id, 100) == 0) {
    // 通过 ID 计算容量 (W25Q128 = 0x18 -> 2^18 = 256K * 64 = 16MB)
    if (drv->base.info.device_id[0] == 0xEF) { // Winbond
      drv->base.info.total_size = 1 << drv->base.info.device_id[2];
    }
  }

  return 0;
}

static int w25q_read(nor_flash_driver_t *self, uint32_t addr, uint8_t *buf,
                     size_t len) {
  w25q_driver_t *drv = (w25q_driver_t *)self;
  qspi_command_t cmd = {0};

  if (drv->base.info.total_size > (16 * 1024 * 1024)) {
    cmd.Instruction = W25Q_QUAD_IO_READ_4B;
    cmd.AddressSize = QSPI_DRV_ADDR_32BITS;
  } else {
    cmd.Instruction = W25Q_QUAD_IO_READ;
    cmd.AddressSize = QSPI_DRV_ADDR_24BITS;
  }
  cmd.InstructionMode = QSPI_DRV_INSTR_1_LINE;
  cmd.AddressMode = QSPI_DRV_ADDR_4_LINES;
  cmd.Address = addr;
  cmd.AlternateByteMode = QSPI_DRV_ADDR_4_LINES;
  cmd.AlternateBytesSize = QSPI_DRV_ADDR_8BITS;
  cmd.AlternateBytes = 0xFF;
  cmd.DummyCycles = 4;
  cmd.DataMode = QSPI_DRV_DATA_4_LINES;
  cmd.NbData = len;

  return QSPI_RECEIVE(drv->qspi, &cmd, buf, 1000);
}

static int w25q_write_page(w25q_driver_t *drv, uint32_t addr,
                           const uint8_t *data, size_t len) {
  qspi_command_t cmd = {0};

  if (w25q_write_enable(drv) != 0)
    return -1;

  if (drv->base.info.total_size > (16 * 1024 * 1024)) {
    cmd.Instruction = W25Q_QUAD_PAGE_PROGRAM_4B;
    cmd.AddressSize = QSPI_DRV_ADDR_32BITS;
  } else {
    cmd.Instruction = W25Q_QUAD_PAGE_PROGRAM;
    cmd.AddressSize = QSPI_DRV_ADDR_24BITS;
  }
  cmd.InstructionMode = QSPI_DRV_INSTR_1_LINE;
  cmd.AddressMode = QSPI_DRV_ADDR_1_LINE;
  cmd.Address = addr;
  cmd.DataMode = QSPI_DRV_DATA_4_LINES;
  cmd.NbData = len;

  if (QSPI_TRANSMIT(drv->qspi, &cmd, data, 1000) != 0)
    return -2;
  return w25q_wait_busy(drv, 200);
}

static int w25q_write(nor_flash_driver_t *self, uint32_t addr,
                      const uint8_t *data, size_t len) {
  w25q_driver_t *drv = (w25q_driver_t *)self;
  uint32_t left = len;
  uint32_t current_addr = addr;
  const uint8_t *p = data;

  while (left > 0) {
    uint32_t page_offset = current_addr % drv->base.info.page_size;
    uint32_t can_write = drv->base.info.page_size - page_offset;
    uint32_t write_len = (left < can_write) ? left : can_write;

    if (w25q_write_page(drv, current_addr, p, write_len) != 0)
      return -1;

    current_addr += write_len;
    p += write_len;
    left -= write_len;
  }
  return 0;
}

static int w25q_erase_sector(nor_flash_driver_t *self, uint32_t addr) {
  w25q_driver_t *drv = (w25q_driver_t *)self;
  qspi_command_t cmd = {0};

  if (w25q_write_enable(drv) != 0)
    return -1;

  if (drv->base.info.total_size > (16 * 1024 * 1024)) {
    cmd.Instruction = W25Q_SECTOR_ERASE_4B;
    cmd.AddressSize = QSPI_DRV_ADDR_32BITS;
  } else {
    cmd.Instruction = W25Q_SECTOR_ERASE;
    cmd.AddressSize = QSPI_DRV_ADDR_24BITS;
  }
  cmd.InstructionMode = QSPI_DRV_INSTR_1_LINE;
  cmd.AddressMode = QSPI_DRV_ADDR_1_LINE;
  cmd.Address = addr;

  if (QSPI_COMMAND(drv->qspi, &cmd, 100) != 0)
    return -2;
  return w25q_wait_busy(drv, 500);
}

static int w25q_set_mode(nor_flash_driver_t *self, nor_flash_mode_t mode) {
  w25q_driver_t *drv = (w25q_driver_t *)self;
  if (mode == NOR_FLASH_MODE_XIP) {
    qspi_command_t cmd = {0};
    if (drv->base.info.total_size > (16 * 1024 * 1024)) {
      cmd.Instruction = W25Q_QUAD_IO_READ_4B;
      cmd.AddressSize = QSPI_DRV_ADDR_32BITS;
    } else {
      cmd.Instruction = W25Q_QUAD_IO_READ;
      cmd.AddressSize = QSPI_DRV_ADDR_24BITS;
    }
    cmd.InstructionMode = QSPI_DRV_INSTR_1_LINE;
    cmd.AddressMode = QSPI_DRV_ADDR_4_LINES;
    cmd.AlternateByteMode = QSPI_DRV_ADDR_4_LINES;
    cmd.AlternateBytesSize = QSPI_DRV_ADDR_8BITS;
    cmd.AlternateBytes = 0xFF;
    cmd.DummyCycles = 4;
    cmd.DataMode = QSPI_DRV_DATA_4_LINES;
    cmd.SIOOMode = QSPI_DRV_SIOO_INST_EVERY_CMD;
    return QSPI_MEMORY_MAPPED(drv->qspi, &cmd);
  }
  return 0;
}

static int w25q_get_info(nor_flash_driver_t *self, nor_flash_info_t *info) {
  *info = self->info;
  return 0;
}

static const nor_flash_driver_ops_t w25q_ops = {
    .read = w25q_read,
    .write = w25q_write,
    .erase_sector = w25q_erase_sector,
    .get_info = w25q_get_info,
    .set_mode = w25q_set_mode,
};

nor_flash_driver_t *w25q_driver_create(qspi_driver_t *qspi,
                                       nor_flash_info_t *info) {
  if (qspi == NULL)
    return NULL;

  w25q_driver_t *drv =
      (w25q_driver_t *)sys_malloc(SYS_MEM_INTERNAL, sizeof(w25q_driver_t));
  if (drv) {
    memset(drv, 0, sizeof(w25q_driver_t));
    drv->base.ops = &w25q_ops;
    drv->qspi = qspi;

    // 强制先初始化 QE 位，否则任何 4 线操作都会失效
    w25q_enable_quad_mode(drv);

    if (info) {
      memcpy(&drv->base.info, info, sizeof(nor_flash_info_t));
    } else {
      w25q_probe_sfdp(drv);
    }
  }
  return (nor_flash_driver_t *)drv;
}
