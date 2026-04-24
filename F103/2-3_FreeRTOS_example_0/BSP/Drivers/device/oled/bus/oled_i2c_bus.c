/*
 * oled_i2c_bus.c
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */
#include "oled_i2c_bus.h"
#include "interface_inc.h"
//#include "MemPool.h"
#ifdef USE_MEMPOOL
#define OLED_I2C_MEMSOURCE SYS_MEM_INTERNAL
#endif

static int oled_i2c_bus_write(oled_bus_t *self, oled_data_type_t type,
                              const uint8_t *data, uint32_t len);

const oled_bus_t oled_i2c_bus_ops = {
    .write = oled_i2c_bus_write,
};

// 实现总线的 write 方法
static int oled_i2c_bus_write(oled_bus_t *self, oled_data_type_t type,
                              const uint8_t *data, uint32_t len) {
  oled_i2c_bus_t *oled_i2c_bus = (oled_i2c_bus_t *)self;
  // Control Byte: 0x00 为指令，0x40 为数据
  uint8_t control_byte = ((type == OLED_CMD) ? 0x00 : 0x40);
  return I2C_MEM_WRITE(oled_i2c_bus->i2c_dev, oled_i2c_bus->dev_addr, control_byte, I2C_MEMADD_SIZE_8BIT, data, len, 100);
}

// 创建 I2C 总线对象的函数
oled_bus_t *OLED_I2C_Bus_Create(i2c_driver_t *i2c, const oled_i2c_config_t *config) {
  if (!config)
    return NULL;
#ifdef USE_MEMPOOL
  oled_i2c_bus_t *self =
      (oled_i2c_bus_t *)sys_malloc(OLED_I2C_MEMSOURCE, sizeof(oled_i2c_bus_t));
#else
  oled_i2c_bus_t *self =
      (oled_i2c_bus_t *)malloc(sizeof(oled_i2c_bus_t));
#endif

  if (self) {
    self->base = oled_i2c_bus_ops;
    self->dev_addr = config->dev_addr;
    self->i2c_dev = i2c;
  }
  return (oled_bus_t *)self;
}
