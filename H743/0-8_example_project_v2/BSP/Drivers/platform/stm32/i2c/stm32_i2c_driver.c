/*
 * stm32_i2c_driver.c
 *
 *  Created on: Feb 8, 2026
 *      Author: Antigravity
 */

#include "stm32_i2c_driver.h"
#include <stdlib.h>
#include "MemPool.h"

#define I2C_MEMSOURCE SYS_MEM_INTERNAL

static int stm32_i2c_master_transmit(i2c_driver_t *self, uint16_t dev_addr,
                                     const uint8_t *data, uint16_t size,
                                     uint32_t timeout) {
  stm32_i2c_driver_t *driver = (stm32_i2c_driver_t *)self;
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
      driver->config.resource.hi2c, dev_addr, (uint8_t *)data, size, timeout);
  return (status == HAL_OK) ? 0 : -1;
}

static int stm32_i2c_master_receive(i2c_driver_t *self, uint16_t dev_addr,
                                    uint8_t *buffer, uint16_t size,
                                    uint32_t timeout) {
  stm32_i2c_driver_t *driver = (stm32_i2c_driver_t *)self;
  HAL_StatusTypeDef status = HAL_I2C_Master_Receive(
      driver->config.resource.hi2c, dev_addr, buffer, size, timeout);
  return (status == HAL_OK) ? 0 : -1;
}

#ifdef USE_I2C_MEM
static int stm32_i2c_mem_write(i2c_driver_t *self, uint16_t dev_addr,
                               uint16_t mem_addr, uint16_t mem_addr_size,
                               uint8_t *data, uint16_t size, uint32_t timeout) {
  stm32_i2c_driver_t *driver = (stm32_i2c_driver_t *)self;
  HAL_StatusTypeDef status =
      HAL_I2C_Mem_Write(driver->config.resource.hi2c, dev_addr, mem_addr,
                        mem_addr_size, data, size, timeout);
  return (status == HAL_OK) ? 0 : -1;
}

static int stm32_i2c_mem_read(i2c_driver_t *self, uint16_t dev_addr,
                              uint16_t mem_addr, uint16_t mem_addr_size,
                              uint8_t *buffer, uint16_t size,
                              uint32_t timeout) {
  stm32_i2c_driver_t *driver = (stm32_i2c_driver_t *)self;
  HAL_StatusTypeDef status =
      HAL_I2C_Mem_Read(driver->config.resource.hi2c, dev_addr, mem_addr,
                       mem_addr_size, buffer, size, timeout);
  return (status == HAL_OK) ? 0 : -1;
}
#endif

static const i2c_driver_ops_t stm32_i2c_ops = {
    .master_transmit = stm32_i2c_master_transmit,
    .master_receive = stm32_i2c_master_receive,
#ifdef USE_I2C_MEM
    .mem_write = stm32_i2c_mem_write,
    .mem_read = stm32_i2c_mem_read,
#endif
};

i2c_driver_t *stm32_i2c_driver_create(I2C_HandleTypeDef *hi2c) {
#ifdef USE_MEMPOOL
  stm32_i2c_driver_t *driver = (stm32_i2c_driver_t *)sys_malloc(
      I2C_MEMSOURCE, sizeof(stm32_i2c_driver_t));
#else
  stm32_i2c_driver_t *driver = (stm32_i2c_driver_t *)malloc(sizeof(stm32_i2c_driver_t));
#endif
  if (driver) {
    driver->base.ops = &stm32_i2c_ops;
    driver->config.resource.hi2c = hi2c;
  }
  return (i2c_driver_t *)driver;
}