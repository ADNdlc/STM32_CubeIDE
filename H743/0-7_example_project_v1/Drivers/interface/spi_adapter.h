/*
 * spi_adapter.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  SPI 适配器接口（适配器模式）
 *  抽象不同 SPI 硬件的差异（标准SPI、QSPI等）
 */

#ifndef INTERFACE_SPI_ADAPTER_H_
#define INTERFACE_SPI_ADAPTER_H_

#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

// 前向声明
typedef struct spi_adapter_t spi_adapter_t;

// SPI 适配器错误码
typedef enum {
  SPI_OK = 0,
  SPI_ERR_PARAM,
  SPI_ERR_TIMEOUT,
  SPI_ERR_BUSY,
  SPI_ERR_TRANSFER,
} spi_error_t;

// SPI 传输模式（用于QSPI）
typedef enum {
  SPI_MODE_1_1_1 = 0, // 1线指令、1线地址、1线数据（标准SPI）
  SPI_MODE_1_1_2,     // 1线指令、1线地址、2线数据
  SPI_MODE_1_1_4,     // 1线指令、1线地址、4线数据
  SPI_MODE_1_2_2,     // 1线指令、2线地址、2线数据
  SPI_MODE_1_4_4,     // 1线指令、4线地址、4线数据
  SPI_MODE_4_4_4,     // 4线QPI模式
} spi_mode_t;

// QSPI 命令结构
typedef struct {
  uint8_t instruction;        // 指令
  uint32_t address;           // 地址
  uint8_t address_size;       // 地址大小：0/1/2/3/4 字节
  uint8_t alternate_bytes[4]; // 交替字节
  uint8_t alternate_size;     // 交替字节大小
  uint8_t dummy_cycles;       // 空周期数
  spi_mode_t mode;            // 传输模式
} spi_command_t;

// SPI 适配器操作接口
typedef struct {
  // 初始化
  spi_error_t (*init)(spi_adapter_t *self);

  // 去初始化
  spi_error_t (*deinit)(spi_adapter_t *self);

  // 选择设备（拉低CS）
  void (*select)(spi_adapter_t *self);

  // 取消选择（拉高CS）
  void (*deselect)(spi_adapter_t *self);

  // 发送数据
  spi_error_t (*transmit)(spi_adapter_t *self, const uint8_t *data,
                          uint32_t len, uint32_t timeout_ms);

  // 接收数据
  spi_error_t (*receive)(spi_adapter_t *self, uint8_t *data, uint32_t len,
                         uint32_t timeout_ms);

  // 双向传输（发送同时接收）
  spi_error_t (*transmit_receive)(spi_adapter_t *self, const uint8_t *tx_data,
                                  uint8_t *rx_data, uint32_t len,
                                  uint32_t timeout_ms);

  // QSPI 命令传输（可选，标准SPI可设为NULL）
  spi_error_t (*command)(spi_adapter_t *self, const spi_command_t *cmd);

  // QSPI 快速读取（可选）
  spi_error_t (*fast_read)(spi_adapter_t *self, const spi_command_t *cmd,
                           uint8_t *data, uint32_t len, uint32_t timeout_ms);

  // QSPI 快速写入（可选）
  spi_error_t (*fast_write)(spi_adapter_t *self, const spi_command_t *cmd,
                            const uint8_t *data, uint32_t len,
                            uint32_t timeout_ms);

} spi_adapter_ops_t;

// SPI 适配器类型
typedef enum {
  SPI_TYPE_STANDARD = 0, // 标准SPI
  SPI_TYPE_QSPI,         // Quad SPI
  SPI_TYPE_OSPI,         // Octo SPI（8线）
} spi_type_t;

// SPI 适配器基类
struct spi_adapter_t {
  const spi_adapter_ops_t *ops; // 操作接口
  spi_type_t type;              // 适配器类型
  uint32_t timeout_default;     // 默认超时时间(ms)
  void *priv;                   // 私有数据
};

// ============== 辅助宏定义 ==============

#define SPI_INIT(adapter) ((adapter)->ops->init(adapter))
#define SPI_DEINIT(adapter) ((adapter)->ops->deinit(adapter))
#define SPI_SELECT(adapter) ((adapter)->ops->select(adapter))
#define SPI_DESELECT(adapter) ((adapter)->ops->deselect(adapter))

#define SPI_TRANSMIT(adapter, data, len, timeout)                              \
  ((adapter)->ops->transmit(adapter, data, len, timeout))

#define SPI_RECEIVE(adapter, data, len, timeout)                               \
  ((adapter)->ops->receive(adapter, data, len, timeout))

#define SPI_TRANSMIT_RECEIVE(adapter, tx, rx, len, timeout)                    \
  ((adapter)->ops->transmit_receive(adapter, tx, rx, len, timeout))

// QSPI 专用宏
#define SPI_COMMAND(adapter, cmd)                                              \
  ((adapter)->ops->command ? (adapter)->ops->command(adapter, cmd)             \
                           : SPI_ERR_PARAM)

#define SPI_FAST_READ(adapter, cmd, data, len, timeout)                        \
  ((adapter)->ops->fast_read                                                   \
       ? (adapter)->ops->fast_read(adapter, cmd, data, len, timeout)           \
       : SPI_ERR_PARAM)

#define SPI_FAST_WRITE(adapter, cmd, data, len, timeout)                       \
  ((adapter)->ops->fast_write                                                  \
       ? (adapter)->ops->fast_write(adapter, cmd, data, len, timeout)          \
       : SPI_ERR_PARAM)

// 检查是否支持QSPI功能
#define SPI_IS_QSPI(adapter) ((adapter)->type == SPI_TYPE_QSPI)

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_SPI_ADAPTER_H_ */
