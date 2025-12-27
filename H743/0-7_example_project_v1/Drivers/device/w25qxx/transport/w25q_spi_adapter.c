#include "w25q_spi_adapter.h"
#include "elog.h"
#include "sys.h"
#include <stdlib.h>

#define LOG_TAG "W25Q_SPI"

#define W25Q_SPI_MEMSOURCE SYS_MEM_INTERNAL

typedef struct {
  w25q_adapter_t parent;
  spi_driver_t *driver;
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
  log_d("SPI adapter init called");
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;

  if (!impl || !impl->driver) {
    log_e("SPI adapter or driver is NULL");
    return -1;
  }
  
  log_i("SPI adapter initialization completed");
  return 0;
}

static int _spi_write_enable(w25q_adapter_t *self) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd = CMD_WRITE_ENABLE;

  log_d("SPI write enable command");
  SPI_CS_CONTROL(impl->driver, 0);
  int result = SPI_TRANSMIT(impl->driver, &cmd, 1, 100);
  SPI_CS_CONTROL(impl->driver, 1);
  log_d("SPI write enable result: %d", result);
  return result;
}

static int _spi_read_id(w25q_adapter_t *self, uint32_t *id) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd = CMD_READ_ID;
  uint8_t buf[3];

  log_d("SPI reading ID");
  SPI_CS_CONTROL(impl->driver, 0);
  SPI_TRANSMIT(impl->driver, &cmd, 1, 100);
  int result = SPI_RECEIVE(impl->driver, buf, 3, 100);
  SPI_CS_CONTROL(impl->driver, 1);

  if (result != 0) {
    log_e("SPI read ID failed with result: %d", result);
    return result;
  }
  
  *id = (buf[0] << 16) | (buf[1] << 8) | buf[2];
  log_d("SPI read ID successful, ID: 0x%06X", *id);
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

  log_d("SPI read: addr=0x%08X, size=%d", addr, (int)size);
  SPI_CS_CONTROL(impl->driver, 0);
  SPI_TRANSMIT(impl->driver, cmd, 4, 100);
  int result = SPI_RECEIVE(impl->driver, buf, size, 1000);
  SPI_CS_CONTROL(impl->driver, 1);

  log_d("SPI read completed with result: %d", result);
  return result;
}

static int _spi_program_page(w25q_adapter_t *self, uint32_t addr,
                             const uint8_t *buf, size_t size) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd[4];
  cmd[0] = CMD_PAGE_PROGRAM;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  log_d("SPI program page: addr=0x%08X, size=%d", addr, (int)size);
  SPI_CS_CONTROL(impl->driver, 0);
  SPI_TRANSMIT(impl->driver, cmd, 4, 100);
  int result = SPI_TRANSMIT(impl->driver, buf, size, 1000);
  SPI_CS_CONTROL(impl->driver, 1);

  log_d("SPI program page completed with result: %d", result);
  return result;
}

static int _spi_erase_sector(w25q_adapter_t *self, uint32_t addr) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd[4];
  cmd[0] = CMD_SECTOR_ERASE;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  log_d("SPI sector erase: addr=0x%08X", addr);
  SPI_CS_CONTROL(impl->driver, 0);
  SPI_TRANSMIT(impl->driver, cmd, 4, 100);
  SPI_CS_CONTROL(impl->driver, 1);
  int result = 0;

  log_d("SPI sector erase completed with result: %d", result);
  return result;
}

static int _spi_erase_block(w25q_adapter_t *self, uint32_t addr) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd[4];
  cmd[0] = CMD_BLOCK_ERASE;
  cmd[1] = (addr >> 16) & 0xFF;
  cmd[2] = (addr >> 8) & 0xFF;
  cmd[3] = addr & 0xFF;

  SPI_CS_CONTROL(impl->driver, 0);
  SPI_TRANSMIT(impl->driver, cmd, 4, 100);
  SPI_CS_CONTROL(impl->driver, 1);

  return 0;
}

static int _spi_erase_chip(w25q_adapter_t *self) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd = CMD_CHIP_ERASE;

  SPI_CS_CONTROL(impl->driver, 0);
  SPI_TRANSMIT(impl->driver, &cmd, 1, 100);
  SPI_CS_CONTROL(impl->driver, 1);

  return 0;
}

static int _spi_wait_busy(w25q_adapter_t *self, uint32_t timeout) {
  w25q_spi_adapter_impl_t *impl = (w25q_spi_adapter_impl_t *)self;
  uint8_t cmd = CMD_READ_STATUS_REG;
  uint8_t status;
  uint32_t tick = 0; // Should use proper tick get

  do {
    SPI_CS_CONTROL(impl->driver, 0);
    SPI_TRANSMIT(impl->driver, &cmd, 1, 100);
    SPI_RECEIVE(impl->driver, &status, 1, 100);
    SPI_CS_CONTROL(impl->driver, 1);

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

w25q_adapter_t *w25q_spi_adapter_create(spi_driver_t *spi_driver){
  w25q_spi_adapter_impl_t *adapter = (w25q_spi_adapter_impl_t *)sys_malloc(
      W25Q_SPI_MEMSOURCE, sizeof(w25q_spi_adapter_impl_t));
  if (adapter) {
    adapter->parent.ops = &w25q_spi_ops;
    adapter->parent.user_data = adapter;
    adapter->driver = spi_driver;
    return &adapter->parent;
  }
  return NULL;
}

void w25q_spi_adapter_destroy(w25q_adapter_t *adapter) {
  if (adapter) {
    sys_free(W25Q_SPI_MEMSOURCE, adapter);
  }
}