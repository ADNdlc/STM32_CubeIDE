/*
 * stm32_i2c_driver.c
 *
 *  Created on: Feb 8, 2026
 *      Author: Antigravity
 */

#include "stm32_i2c_driver.h"
#include <stdlib.h>
//#include "MemPool.h"

#define I2C_MEMSOURCE SYS_MEM_INTERNAL

#define MAX_I2C_INSTANCES 2
static stm32_i2c_driver_t *i2c_instances[MAX_I2C_INSTANCES] = {0};

static void register_i2c_instance(stm32_i2c_driver_t *driver) {
  for (int i = 0; i < MAX_I2C_INSTANCES; i++) {
    if (i2c_instances[i] == NULL) {
      i2c_instances[i] = driver;
      break;
    }
  }
}

static stm32_i2c_driver_t *find_i2c_driver(I2C_HandleTypeDef *hi2c) {
  for (int i = 0; i < MAX_I2C_INSTANCES; i++) {
    if (i2c_instances[i] && i2c_instances[i]->config.resource.hi2c == hi2c) {
      return i2c_instances[i];
    }
  }
  return NULL;
}

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

static int stm32_i2c_master_transmit_asyn(i2c_driver_t *self, uint16_t dev_addr,
                                          const uint8_t *data, uint16_t size) {
  stm32_i2c_driver_t *driver = (stm32_i2c_driver_t *)self;
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_DMA(
      driver->config.resource.hi2c, dev_addr, (uint8_t *)data, size);
  return (status == HAL_OK) ? 0 : -1;
}

static int stm32_i2c_master_receive_asyn(i2c_driver_t *self, uint16_t dev_addr,
                                         uint8_t *buffer, uint16_t size) {
  stm32_i2c_driver_t *driver = (stm32_i2c_driver_t *)self;
  HAL_StatusTypeDef status = HAL_I2C_Master_Receive_DMA(
      driver->config.resource.hi2c, dev_addr, buffer, size);
  return (status == HAL_OK) ? 0 : -1;
}

static int stm32_i2c_set_callback(i2c_driver_t *self, i2c_callback_t cb,
                                  void *cb_context) {
  stm32_i2c_driver_t *driver = (stm32_i2c_driver_t *)self;
  driver->callback = cb;
  driver->callback_context = cb_context;
  return 0;
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
    .master_transmit_asyn = stm32_i2c_master_transmit_asyn,
    .master_receive_asyn = stm32_i2c_master_receive_asyn,
    .set_callback = stm32_i2c_set_callback,
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
  stm32_i2c_driver_t *driver =
      (stm32_i2c_driver_t *)malloc(sizeof(stm32_i2c_driver_t));
#endif
  if (driver) {
    driver->base.ops = &stm32_i2c_ops;
    driver->config.is_soft = 0;
    driver->config.resource.hi2c = hi2c;
    driver->callback = NULL;
    driver->callback_context = NULL;
    register_i2c_instance(driver);
  }
  return (i2c_driver_t *)driver;
}

// HAL Callbacks
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
  stm32_i2c_driver_t *driver = find_i2c_driver(hi2c);
  if (driver && driver->callback) {
    driver->callback(driver->callback_context, I2C_EVENT_TX_COMPLETE, NULL);
  }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  stm32_i2c_driver_t *driver = find_i2c_driver(hi2c);
  if (driver && driver->callback) {
    // 假设驱动层知道接收了多少。对于 DMA 模式，size 是已知的。
    // 在这里我们可以传递接收到的实际长度（如果 HAL 支持不确定长度的话，但在基础
    // HAL 里通常是固定的）
    driver->callback(driver->callback_context, I2C_EVENT_RX_COMPLETE,
                     (void *)(size_t)hi2c->XferSize);
  }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
  stm32_i2c_driver_t *driver = find_i2c_driver(hi2c);
  if (driver && driver->callback) {
    driver->callback(driver->callback_context, I2C_EVENT_ERROR,
                     (void *)(size_t)hi2c->ErrorCode);
  }
}
