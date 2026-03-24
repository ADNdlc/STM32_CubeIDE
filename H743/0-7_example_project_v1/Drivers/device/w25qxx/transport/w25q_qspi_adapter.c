#include "w25q_qspi_adapter.h"
#include "../../../interface/qspi_driver.h" // Ensure visibility of generic enums
#include "elog.h"
#include "sys.h"
#include <stdlib.h>

#define LOG_TAG "W25Q_QSPI"

#define W25Q_QSPI_MEMSOURCE SYS_MEM_INTERNAL

typedef struct {
  w25q_adapter_t parent;
  qspi_driver_t *driver;
  qspi_addr_size_t addr_size;
} w25q_qspi_adapter_impl_t;

// Commands
#define CMD_WRITE_ENABLE 0x06
#define CMD_READ_ID 0x9F
#define CMD_READ_DATA 0xEB
#define CMD_PAGE_PROGRAM 0x32
#define CMD_SECTOR_ERASE 0x20
#define CMD_BLOCK_ERASE 0xD8
#define CMD_CHIP_ERASE 0xC7
#define CMD_READ_STATUS_REG1 0x05
#define CMD_READ_STATUS_REG2 0x35
#define CMD_WRITE_STATUS_REG2 0x31
#define CMD_EXIT_QPI_MODE 0xFF
#define CMD_ENTER_4BYTE_ADDR 0xB7
#define CMD_EXIT_4BYTE_ADDR 0xE9

#define SR2_QE_MASK 0x02

static void _fill_cmd(w25q_qspi_adapter_impl_t *impl, qspi_command_t *cmd,
                      uint8_t inst, uint32_t addr, qspi_mode_t inst_mode,
                      qspi_mode_t addr_mode, qspi_mode_t data_mode,
                      uint8_t dummy_cycles, size_t size) {
  cmd->instruction = inst;
  cmd->address = addr;
  cmd->instruction_mode = inst_mode;
  cmd->address_mode = addr_mode;
  cmd->address_size = impl->addr_size;
  cmd->alternate_byte_mode = QSPI_MODE_NONE;
  cmd->data_mode = data_mode;
  cmd->dummy_cycles = dummy_cycles;
  cmd->data_size = size;
}

static int _qspi_init(w25q_adapter_t *self) {
  log_d("Starting QSPI adapter initialization");
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;

  if (!impl || !impl->driver) {
    log_e("QSPI adapter or driver is NULL");
    return -1;
  }

  log_d("QSPI adapter driver is valid, proceeding with initialization");
  qspi_command_t cmd;
  uint8_t sr2 = 0;

  // Step 1: 退出QPI模式 (以防芯片之前进入了QPI模式，导致SPI命令无法正常工作)
  // 使用4线传输指令模式发送 Exit QPI Mode 命令 (0xFF)
  log_d("Step 1: Exiting QPI mode if chip is in it");
  _fill_cmd(impl, &cmd, CMD_EXIT_QPI_MODE, 0, QSPI_MODE_4_LINES, QSPI_MODE_NONE,
            QSPI_MODE_NONE, 0, 0);
  int result =
      QSPI_COMMAND(impl->driver, &cmd, 5000); // 忽略返回值，芯片可能不在QPI模式
  log_d("QPI mode exit command result: %d", result);

  // Step 2: 读取状态寄存器2 (SPI单线模式)
  log_d("Step 2: Reading status register 2");
  _fill_cmd(impl, &cmd, CMD_READ_STATUS_REG2, 0, QSPI_MODE_1_LINE,
            QSPI_MODE_NONE, QSPI_MODE_1_LINE, 0, 1);
  if (QSPI_COMMAND(impl->driver, &cmd, 5000) != 0 ||
      QSPI_RECEIVE(impl->driver, &sr2, 5000) != 0) {
    log_e("Failed to read status register 2");
    return -1;
  }
  log_d("Successfully read status register 2, value: 0x%02X", sr2);

  // Step 3: 使能Quad Enable (QE) 位 - SR2 bit 1
  if (!(sr2 & SR2_QE_MASK)) {
    log_d("Step 3: QE bit is not set, enabling Quad Enable");
    sr2 |= SR2_QE_MASK;

    // 3.1 发送写使能命令
    log_d("Sending write enable command");
    _fill_cmd(impl, &cmd, CMD_WRITE_ENABLE, 0, QSPI_MODE_1_LINE, QSPI_MODE_NONE,
              QSPI_MODE_NONE, 0, 0);
    if (QSPI_COMMAND(impl->driver, &cmd, 100) != 0) {
      log_e("Failed to send write enable command");
      return -1;
    }

    // 3.2 写入状态寄存器2
    log_d("Writing status register 2 to enable QE bit");
    _fill_cmd(impl, &cmd, CMD_WRITE_STATUS_REG2, 0, QSPI_MODE_1_LINE,
              QSPI_MODE_NONE, QSPI_MODE_1_LINE, 0, 1);
    if (QSPI_COMMAND(impl->driver, &cmd, 100) != 0 ||
        QSPI_TRANSMIT(impl->driver, &sr2, 100) != 0) {
      log_e("Failed to write status register 2");
      return -1;
    }

    // 3.3 等待芯片完成写入 (轮询SR1的BUSY位)
    log_d("Waiting for flash chip to complete write operation");
    uint8_t status;
    uint32_t timeout = 1000;
    do {
      _fill_cmd(impl, &cmd, CMD_READ_STATUS_REG1, 0, QSPI_MODE_1_LINE,
                QSPI_MODE_NONE, QSPI_MODE_1_LINE, 0, 1);
      if (QSPI_COMMAND(impl->driver, &cmd, 100) != 0 ||
          QSPI_RECEIVE(impl->driver, &status, 100) != 0) {
        log_e("Error reading status register 1 during QE enable");
        return -1;
      }
      if (--timeout == 0) {
        log_e("Timeout waiting for flash operation to complete");
        return -1; // 超时
      }
    } while (status & 0x01);
    log_i("Quad Enable bit successfully set");
  } else {
    log_d("QE bit is already enabled");
  }

  log_i("QSPI adapter initialization completed successfully");
  return 0;
}

static int _qspi_write_enable(w25q_adapter_t *self) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  _fill_cmd(impl, &cmd, CMD_WRITE_ENABLE, 0, QSPI_MODE_1_LINE, QSPI_MODE_NONE,
            QSPI_MODE_NONE, 0, 0);
  int result = QSPI_COMMAND(impl->driver, &cmd, 100);
  log_d("Write enable command result: %d", result);
  return result;
}

static int _qspi_read_id(w25q_adapter_t *self, uint32_t *id) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  uint8_t buf[3];
  _fill_cmd(impl, &cmd, CMD_READ_ID, 0, QSPI_MODE_1_LINE, QSPI_MODE_NONE,
            QSPI_MODE_1_LINE, 0, 3);
  if (QSPI_COMMAND(impl->driver, &cmd, 100) != 0 ||
      QSPI_RECEIVE(impl->driver, buf, 100) != 0)
    return -1;
  *id = (buf[0] << 16) | (buf[1] << 8) | buf[2];
  return 0;
}

static int _qspi_read(w25q_adapter_t *self, uint32_t addr, uint8_t *buf,
                      size_t size) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  _fill_cmd(impl, &cmd, CMD_READ_DATA, addr, QSPI_MODE_1_LINE,
            QSPI_MODE_4_LINES, QSPI_MODE_4_LINES, 6, size);
  if (QSPI_COMMAND(impl->driver, &cmd, 100) != 0 ||
      QSPI_RECEIVE(impl->driver, buf, 1000) != 0)
    return -1;
  return 0;
}

static int _qspi_program_page(w25q_adapter_t *self, uint32_t addr,
                              const uint8_t *buf, size_t size) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  _fill_cmd(impl, &cmd, CMD_PAGE_PROGRAM, addr, QSPI_MODE_1_LINE,
            QSPI_MODE_1_LINE, QSPI_MODE_4_LINES, 0, size);
  if (QSPI_COMMAND(impl->driver, &cmd, 100) != 0 ||
      QSPI_TRANSMIT(impl->driver, buf, 1000) != 0)
    return -1;
  return 0;
}

static int _qspi_erase_sector(w25q_adapter_t *self, uint32_t addr) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  _fill_cmd(impl, &cmd, CMD_SECTOR_ERASE, addr, QSPI_MODE_1_LINE,
            QSPI_MODE_1_LINE, QSPI_MODE_NONE, 0, 0);
  return QSPI_COMMAND(impl->driver, &cmd, 100);
}

static int _qspi_erase_block(w25q_adapter_t *self, uint32_t addr) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  _fill_cmd(impl, &cmd, CMD_BLOCK_ERASE, addr, QSPI_MODE_1_LINE,
            QSPI_MODE_1_LINE, QSPI_MODE_NONE, 0, 0);
  return QSPI_COMMAND(impl->driver, &cmd, 5000);
}

static int _qspi_erase_chip(w25q_adapter_t *self) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  _fill_cmd(impl, &cmd, CMD_CHIP_ERASE, 0, QSPI_MODE_1_LINE, QSPI_MODE_NONE,
            QSPI_MODE_NONE, 0, 0);
  return QSPI_COMMAND(impl->driver, &cmd, 5000);
}

static int _qspi_wait_busy(w25q_adapter_t *self, uint32_t timeout) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  qspi_command_t cfg;
  _fill_cmd(impl, &cmd, CMD_READ_STATUS_REG1, 0, QSPI_MODE_1_LINE,
            QSPI_MODE_NONE, QSPI_MODE_1_LINE, 0, 1);
  cfg.address = 0;
  cfg.alternate_byte = 1;
  cfg.instruction = QSPI_MATCH_AND;
  cfg.data_size = 1;
  return QSPI_AUTO_POLLING(impl->driver, &cmd, &cfg, timeout);
}

static int _qspi_is_busy(w25q_adapter_t *self) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  uint8_t status = 0;
  _fill_cmd(impl, &cmd, CMD_READ_STATUS_REG1, 0, QSPI_MODE_1_LINE,
            QSPI_MODE_NONE, QSPI_MODE_1_LINE, 0, 1);
  if (QSPI_COMMAND(impl->driver, &cmd, 10) != 0 ||
      QSPI_RECEIVE(impl->driver, &status, 10) != 0) {
    return -1;
  }
  return (status & 0x01) ? 1 : 0;
}

static int _qspi_enter_4byte_addr_mode(w25q_adapter_t *self) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  log_d("Entering 4-byte address mode");
  _fill_cmd(impl, &cmd, CMD_ENTER_4BYTE_ADDR, 0, QSPI_MODE_1_LINE,
            QSPI_MODE_NONE, QSPI_MODE_NONE, 0, 0);
  if (QSPI_COMMAND(impl->driver, &cmd, 100) != 0)
    return -1;
  impl->addr_size = QSPI_ADDR_32_BITS;
  return 0;
}

static int _qspi_exit_4byte_addr_mode(w25q_adapter_t *self) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  log_d("Exiting 4-byte address mode");
  _fill_cmd(impl, &cmd, CMD_EXIT_4BYTE_ADDR, 0, QSPI_MODE_1_LINE,
            QSPI_MODE_NONE, QSPI_MODE_NONE, 0, 0);
  if (QSPI_COMMAND(impl->driver, &cmd, 100) != 0)
    return -1;
  impl->addr_size = QSPI_ADDR_24_BITS;
  return 0;
}

static const w25q_adapter_ops_t w25q_qspi_ops = {
    .init = _qspi_init,
    .read_id = _qspi_read_id,
    .read = _qspi_read,
    .write_enable = _qspi_write_enable,
    .program_page = _qspi_program_page,
    .erase_sector = _qspi_erase_sector,
    .erase_block = _qspi_erase_block,
    .erase_chip = _qspi_erase_chip,
    .wait_busy = _qspi_wait_busy,
    .is_busy = _qspi_is_busy,
    .enter_4byte_addr_mode = _qspi_enter_4byte_addr_mode,
    .exit_4byte_addr_mode = _qspi_exit_4byte_addr_mode,
};

w25q_adapter_t *w25q_qspi_adapter_create(qspi_driver_t *qspi_driver) {
  w25q_qspi_adapter_impl_t *adapter = (w25q_qspi_adapter_impl_t *)sys_malloc(
      W25Q_QSPI_MEMSOURCE, sizeof(w25q_qspi_adapter_impl_t));
  if (adapter) {
    adapter->parent.ops = &w25q_qspi_ops;
    adapter->parent.user_data = adapter;
    adapter->driver = qspi_driver;
    adapter->addr_size = QSPI_ADDR_24_BITS;
    return &adapter->parent;
  }
  return NULL;
}

void w25q_qspi_adapter_destroy(w25q_adapter_t *adapter) {
  if (adapter)
    sys_free(W25Q_QSPI_MEMSOURCE, adapter);
}