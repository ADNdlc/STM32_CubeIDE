/*
 * qspi_driver.h
 *
 *  Created on: Feb 16, 2026
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_INTERFACE_QSPI_DRIVER_H_
#define BSP_DEVICE_DRIVER_INTERFACE_QSPI_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

// 前向声明
typedef struct qspi_driver_t qspi_driver_t;

// QSPI 模式常量 (抽象定义)
#define QSPI_DRV_INSTR_NONE 0
#define QSPI_DRV_INSTR_1_LINE 1
#define QSPI_DRV_INSTR_2_LINES 2
#define QSPI_DRV_INSTR_4_LINES 3

#define QSPI_DRV_ADDR_NONE 0
#define QSPI_DRV_ADDR_1_LINE 1
#define QSPI_DRV_ADDR_2_LINES 2
#define QSPI_DRV_ADDR_4_LINES 3

#define QSPI_DRV_ADDR_8BITS 0
#define QSPI_DRV_ADDR_16BITS 1
#define QSPI_DRV_ADDR_24BITS 2
#define QSPI_DRV_ADDR_32BITS 3

#define QSPI_DRV_DATA_NONE 0
#define QSPI_DRV_DATA_1_LINE 1
#define QSPI_DRV_DATA_2_LINES 2
#define QSPI_DRV_DATA_4_LINES 3

#define QSPI_DRV_SIOO_INST_EVERY_CMD 0
#define QSPI_DRV_SIOO_INST_ONLY_FIRST_CMD 1
#define QSPI_DRV_DDR_HHC_NONE 0

// QSPI 指令结构体 (参考 HAL_QSPI_CommandTypeDef)
typedef struct {
  uint32_t Instruction;        /* 指令码 */
  uint32_t Address;            /* 地址 */
  uint32_t AlternateBytes;     /* 备用字节 */
  uint32_t AddressSize;        /* 地址长度: 8, 16, 24, 32 bit */
  uint32_t AlternateBytesSize; /* 备用字节长度 */
  uint32_t DummyCycles;        /* 空周期 */
  uint32_t InstructionMode;    /* 指令模式: Single, Dual, Quad */
  uint32_t AddressMode;        /* 地址模式 */
  uint32_t AlternateByteMode;  /* 备用字节模式 */
  uint32_t DataMode;           /* 数据模式 */
  uint32_t NbData;             /* 数据量 */
  uint32_t DdrMode;            /* DDR 模式 */
  uint32_t DdrHoldHalfCycle;   /* DDR 持有半周期 */
  uint32_t SIOOMode;           /* SIOO 模式 */
} qspi_command_t;

// QSPI 驱动操作接口 (虚函数表)
typedef struct {
  // 发送指令 (无数据阶段)
  int (*command)(qspi_driver_t *self, qspi_command_t *cmd, uint32_t timeout);
  // 发送数据 (包含指令阶段)
  int (*transmit)(qspi_driver_t *self, qspi_command_t *cmd, const uint8_t *data,
                  uint32_t timeout);
  // 接收数据 (包含指令阶段)
  int (*receive)(qspi_driver_t *self, qspi_command_t *cmd, uint8_t *buffer,
                 uint32_t timeout);
  // 内存映射模式
  int (*memory_mapped)(qspi_driver_t *self, qspi_command_t *cmd);
} qspi_driver_ops_t;

// QSPI 驱动基类
struct qspi_driver_t {
  const qspi_driver_ops_t *ops;
};

// 辅助宏
#define QSPI_COMMAND(driver, cmd, timeout)                                     \
  (driver)->ops->command(driver, cmd, timeout)

#define QSPI_TRANSMIT(driver, cmd, data, timeout)                              \
  (driver)->ops->transmit(driver, cmd, data, timeout)

#define QSPI_RECEIVE(driver, cmd, buffer, timeout)                             \
  (driver)->ops->receive(driver, cmd, buffer, timeout)

#define QSPI_MEMORY_MAPPED(driver, cmd)                                        \
  (driver)->ops->memory_mapped(driver, cmd)

#endif /* BSP_DEVICE_DRIVER_INTERFACE_QSPI_DRIVER_H_ */
