#include "w25q_qspi_adapter.h"
#include "sys.h"
#include <stdlib.h>

#define W25Q_QSPI_MEMSOURCE SYS_MEM_INTERNAL

typedef struct {
  w25q_adapter_t parent;
  qspi_hal_t *hal;
} w25q_qspi_adapter_impl_t;

// Commands
#define CMD_WRITE_ENABLE 0x06
#define CMD_READ_ID 0x9F
#define CMD_READ_DATA 0xEB    // Quad I/O Fast Read
#define CMD_PAGE_PROGRAM 0x32 // Quad Page Program
#define CMD_SECTOR_ERASE 0x20
#define CMD_BLOCK_ERASE 0xD8
#define CMD_CHIP_ERASE 0xC7
#define CMD_READ_STATUS_REG 0x05

// Helper to fill command structure
static void _fill_cmd(qspi_command_t *cmd, uint8_t inst, uint32_t addr,
                      uint8_t inst_mode, uint8_t addr_mode, uint8_t data_mode,
                      uint8_t dummy_cycles, size_t size) {
  cmd->instruction = inst;
  cmd->address = addr;
  cmd->instruction_mode = inst_mode;
  cmd->address_mode = addr_mode;
  cmd->address_size = QSPI_ADDRESS_24_BITS; // Default 24
  cmd->alternate_byte_mode = QSPI_ALTERNATE_BYTES_NONE;
  cmd->data_mode = data_mode;
  cmd->dummy_cycles = dummy_cycles;
  cmd->data_size = size;
}

static int _qspi_init(w25q_adapter_t *self) {
  // Basic QSPI init done by HAL, no specific CS control needed
  return 0;
}

static int _qspi_write_enable(w25q_adapter_t *self) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  _fill_cmd(&cmd, CMD_WRITE_ENABLE, 0, QSPI_INSTRUCTION_1_LINE,
            QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, 0);

  return qspi_hal_command(impl->hal, &cmd, 100);
}

static int _qspi_read_id(w25q_adapter_t *self, uint32_t *id) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  uint8_t buf[3];

  _fill_cmd(&cmd, CMD_READ_ID, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE,
            QSPI_DATA_1_LINE, 0, 3);

  if (qspi_hal_command(impl->hal, &cmd, 100) != 0)
    return -1;
  if (qspi_hal_receive(impl->hal, buf, 100) != 0)
    return -1;

  *id = (buf[0] << 16) | (buf[1] << 8) | buf[2];
  return 0;
}

static int _qspi_read(w25q_adapter_t *self, uint32_t addr, uint8_t *buf,
                      size_t size) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;

  // Using Quad Fast Read (0xEB)
  // Inst: 1 line, Addr: 4 lines, Data: 4 lines, Dummy: 6 cycles (check
  // datasheet) Actually standard W25Q256 0xEB needs 6 dummy cycles
  _fill_cmd(&cmd, CMD_READ_DATA, addr, QSPI_INSTRUCTION_1_LINE,
            QSPI_ADDRESS_4_LINES, QSPI_DATA_4_LINES, 6, size);

  if (qspi_hal_command(impl->hal, &cmd, 100) != 0)
    return -1;
  if (qspi_hal_receive(impl->hal, buf, 1000) != 0)
    return -1;
  return 0;
}

static int _qspi_program_page(w25q_adapter_t *self, uint32_t addr,
                              const uint8_t *buf, size_t size) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;

  // Quad Input Page Program (0x32)
  // Inst: 1, Addr: 1, Data: 4
  _fill_cmd(&cmd, CMD_PAGE_PROGRAM, addr, QSPI_INSTRUCTION_1_LINE,
            QSPI_ADDRESS_1_LINE, QSPI_DATA_4_LINES, 0, size);

  if (qspi_hal_command(impl->hal, &cmd, 100) != 0)
    return -1;
  if (qspi_hal_transmit(impl->hal, buf, 1000) != 0)
    return -1;
  return 0;
}

static int _qspi_erase_sector(w25q_adapter_t *self, uint32_t addr) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  _fill_cmd(&cmd, CMD_SECTOR_ERASE, addr, QSPI_INSTRUCTION_1_LINE,
            QSPI_ADDRESS_1_LINE, QSPI_DATA_NONE, 0, 0);
  return qspi_hal_command(impl->hal, &cmd, 100);
}

static int _qspi_erase_block(w25q_adapter_t *self, uint32_t addr) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  _fill_cmd(&cmd, CMD_BLOCK_ERASE, addr, QSPI_INSTRUCTION_1_LINE,
            QSPI_ADDRESS_1_LINE, QSPI_DATA_NONE, 0, 0);
  return qspi_hal_command(impl->hal, &cmd, 100);
}

static int _qspi_erase_chip(w25q_adapter_t *self) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  _fill_cmd(&cmd, CMD_CHIP_ERASE, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE,
            QSPI_DATA_NONE, 0, 0);
  return qspi_hal_command(impl->hal, &cmd, 100);
}

static int _qspi_wait_busy(w25q_adapter_t *self, uint32_t timeout) {
  w25q_qspi_adapter_impl_t *impl = (w25q_qspi_adapter_impl_t *)self;
  qspi_command_t cmd;
  qspi_command_t cfg; // Used for auto polling config

  // Configure auto polling for Status Register 1, bit 0 (BUSY) to be 0
  _fill_cmd(&cmd, CMD_READ_STATUS_REG, 0, QSPI_INSTRUCTION_1_LINE,
            QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 0, 1);

  cfg.address = 0;                       // Match 0
  cfg.alternate_byte = 1;                // Mask 1 (Bit 0)
  cfg.instruction = QSPI_MATCH_MODE_AND; // Mode
  cfg.data_size = 1;                     // Status byte size

  return qspi_hal_auto_polling(impl->hal, &cmd, &cfg, timeout);
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
};

w25q_adapter_t *w25q_qspi_adapter_create(qspi_hal_t *hal) {
  w25q_qspi_adapter_impl_t *adapter = (w25q_qspi_adapter_impl_t *)sys_malloc(
      W25Q_QSPI_MEMSOURCE, sizeof(w25q_qspi_adapter_impl_t));
  if (adapter) {
    adapter->parent.ops = &w25q_qspi_ops;
    adapter->parent.user_data = adapter;
    adapter->hal = hal;
    return &adapter->parent;
  }
  return NULL;
}

void w25q_qspi_adapter_destroy(w25q_adapter_t *adapter) {
  if (adapter) {
    sys_free(W25Q_QSPI_MEMSOURCE, adapter);
  }
}
