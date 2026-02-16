/*
 * i2c_driver.h
 *
 *  Created on: Feb 8, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_INTERFACE_I2C_DRIVER_H_
#define BSP_DEVICE_DRIVER_INTERFACE_I2C_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

// 前向声明
typedef struct i2c_driver_t i2c_driver_t;

#define USE_I2C_MEM

// I2C 寄存器地址长度
#define I2C_MEMADD_SIZE_8BIT (0x00000001U)
#define I2C_MEMADD_SIZE_16BIT (0x00000002U)

// I2C 驱动操作接口 (虚函数表)
typedef struct {
  // 基础收发 (同步)
  int (*master_transmit)(i2c_driver_t *self, uint16_t dev_addr,
                         const uint8_t *data, uint16_t size, uint32_t timeout);
  int (*master_receive)(i2c_driver_t *self, uint16_t dev_addr, uint8_t *buffer,
                        uint16_t size, uint32_t timeout);
#ifdef USE_I2C_MEM
  // 针对存储设备的寄存器读写 (同步)
  int (*mem_write)(i2c_driver_t *self, uint16_t dev_addr, uint16_t mem_addr,
                   uint16_t mem_addr_size, uint8_t *data, uint16_t size,
                   uint32_t timeout);
  int (*mem_read)(i2c_driver_t *self, uint16_t dev_addr, uint16_t mem_addr,
                  uint16_t mem_addr_size, uint8_t *buffer, uint16_t size,
                  uint32_t timeout);
#endif
} i2c_driver_ops_t;

// I2C 驱动基类
struct i2c_driver_t {
  const i2c_driver_ops_t *ops;
};

// 辅助宏
#define I2C_MASTER_TRANSMIT(driver, addr, data, size, timeout)                 \
  (driver)->ops->master_transmit(driver, addr, data, size, timeout)

#define I2C_MASTER_RECEIVE(driver, addr, buffer, size, timeout)                \
  (driver)->ops->master_receive(driver, addr, buffer, size, timeout)
#ifdef USE_I2C_MEM
#define I2C_MEM_WRITE(driver, dev_addr, mem_addr, mem_addr_size, data, size,   \
                      timeout)                                                 \
  (driver)->ops->mem_write(driver, dev_addr, mem_addr, mem_addr_size, data,    \
                           size, timeout)

#define I2C_MEM_READ(driver, dev_addr, mem_addr, mem_addr_size, buffer, size,  \
                     timeout)                                                  \
  (driver)->ops->mem_read(driver, dev_addr, mem_addr, mem_addr_size, buffer,   \
                          size, timeout)
#endif
#endif /* BSP_DEVICE_DRIVER_INTERFACE_I2C_DRIVER_H_ */
