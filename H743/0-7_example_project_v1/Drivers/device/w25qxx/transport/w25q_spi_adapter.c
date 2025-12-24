#include "w25q_spi_adapter.h"
#include "sys.h"
#include <stdlib.h>

#define W25Q_SPI_MEMSOURCE SYS_MEM_INTERNAL

typedef struct {
  w25q_adapter_t parent;
  spi_hal_t *hal;
} w25q_spi_adapter_impl_t;

// Commands
#define CMD_WRITE_ENABLE 0x06
#define CMD_READ_ID 0x9F
#define CMD_READ_DATA 0x03
#define CMD_PAGE_PROGRAM 0x02
#define CMD_SECTOR_ERASE 0x20
#define CMD_BLOCK_ERASE 0xD8
#define CMD_CHIP_ERASE 0xC7
#define CMD_READ_STATUS_REG 0x05

static int _spi_init(w25q_adapter_t *self) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  spi_hal_cs_control(impl->hal, 1); // High (Inactive)
  return 0;
}

static int _spi_write_enable(w25q_adapter_t *self) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd = CMD_WRITE_ENABLE;

  spi_hal_cs_control(impl->hal, 0);
  spi_hal_transmit(impl->hal, &cmd, 1, 100);
  spi_hal_cs_control(impl->hal, 1);
  return 0;
}

static int _spi_read_id(w25q_adapter_t *self, uint32_t *id) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd = CMD_READ_ID;
  uint8_t buf[3];

  spi_hal_cs_control(impl->hal, 0);
  spi_hal_transmit(impl->hal, &cmd, 1, 100);
  spi_hal_receive(impl->hal, buf, 3, 100);
  spi_hal_cs_control(impl->hal, 1);

  *id = (buf[0] << 16) | (buf[1] << 8) | buf[2];
  return 0;
}

static int _spi_read(w25q_adapter_t *self, uint32_t addr, uint8_t *buf,
                     size_t size) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd[4];
  cmd[0] = CMD_READ_DATA;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  spi_hal_cs_control(impl->hal, 0);
  spi_hal_transmit(impl->hal, cmd, 4, 100);
  spi_hal_receive(impl->hal, buf, size, 1000);
  spi_hal_cs_control(impl->hal, 1);

  return 0;
}

static int _spi_program_page(w25q_adapter_t *self, uint32_t addr,
                             const uint8_t *buf, size_t size) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd[4];
  cmd[0] = CMD_PAGE_PROGRAM;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  spi_hal_cs_control(impl->hal, 0);
  spi_hal_transmit(impl->hal, cmd, 4, 100);
  spi_hal_transmit(impl->hal, buf, size, 1000);
  spi_hal_cs_control(impl->hal, 1);

  return 0;
}

static int _spi_erase_sector(w25q_adapter_t *self, uint32_t addr) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd[4];
  cmd[0] = CMD_SECTOR_ERASE;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  spi_hal_cs_control(impl->hal, 0);
  spi_hal_transmit(impl->hal, cmd, 4, 100);
  spi_hal_cs_control(impl->hal, 1);

  return 0;
}

static int _spi_erase_block(w25q_adapter_t *self, uint32_t addr) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd[4];
  cmd[0] = CMD_BLOCK_ERASE;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  spi_hal_cs_control(impl->hal, 0);
  spi_hal_transmit(impl->hal, cmd, 4, 100);
  spi_hal_cs_control(impl->hal, 1);

  return 0;
}

static int _spi_erase_chip(w25q_adapter_t *self) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd = CMD_CHIP_ERASE;

  spi_hal_cs_control(impl->hal, 0);
  spi_hal_transmit(impl->hal, &cmd, 1, 100);
  spi_hal_cs_control(impl->hal, 1);

  return 0;
}

static int _spi_wait_busy(w25q_adapter_t *self, uint32_t timeout) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd = CMD_READ_STATUS_REG;
  uint8_t status;
  uint32_t tick = 0; // Should use proper tick get

  do {
    spi_hal_cs_control(impl->hal, 0);
    spi_hal_transmit(impl->hal, &cmd, 1, 100);
    spi_hal_receive(impl->hal, &status, 1, 100);
    spi_hal_cs_control(impl->hal, 1);

    if ((status & 0x01) == 0)
      return 0;

    // sys_delay_ms(1); // Need delay
    tick++;
  } while (tick < timeout);

  return -1;
}

static const w25q_adapter_ops_t w25q_spi_ops = {
    .init = _spi_init,
    .read_id = _spi_read_id,
    .read = _spi_read,
    .write_enable = _spi_write_enable,
    .program_page = _spi_program_page,
    .erase_sector = _spi_erase_sector,
    .erase_block = _spi_erase_block,
    .erase_chip = _spi_erase_chip,
    .wait_busy = _spi_wait_busy,
};

w25q_adapter_t *w25q_spi_adapter_create(spi_hal_t *hal) {
  w25q_spi_adapter_impl_t *adapter = (w25q_spi_adapter_impl_t *)sys_malloc(
      W25Q_SPI_MEMSOURCE, sizeof(w25q_spi_adapter_impl_t));
  if (adapter) {
    adapter->parent.ops = &w25q_spi_ops;
    adapter->parent.user_data = adapter;
    adapter->hal = hal;
    return &adapter->parent;
  }
  return NULL;
}

void w25q_spi_adapter_destroy(w25q_adapter_t *adapter) {
  if (adapter) {
    sys_free(W25Q_SPI_MEMSOURCE, adapter);
  }
}
