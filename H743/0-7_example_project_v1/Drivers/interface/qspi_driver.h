/**
 * @file qspi_driver.h
 * @brief QSPI Driver Interface
 */

#ifndef DRIVERS_INTERFACE_QSPI_DRIVER_H_
#define DRIVERS_INTERFACE_QSPI_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief QSPI Modes and Sizes (Generic)
 */
typedef enum {
  QSPI_MODE_NONE = 0,
  QSPI_MODE_1_LINE = 1,
  QSPI_MODE_2_LINES = 2,
  QSPI_MODE_4_LINES = 4
} qspi_mode_t;

typedef enum {
  QSPI_ADDR_24_BITS = 24,
  QSPI_ADDR_32_BITS = 32
} qspi_addr_size_t;

typedef enum { QSPI_MATCH_AND = 0, QSPI_MATCH_OR = 1 } qspi_match_mode_t;

/**
 * @brief QSPI Command Structure
 */
typedef struct {
  uint8_t instruction;     // 命令
  uint32_t address;        // 地址(如果需要)
  uint32_t alternate_byte; // 交替字节(如果需要)
  uint32_t dummy_cycles;   // 延迟周期
  uint32_t data_size;      // 数据大小

  qspi_mode_t instruction_mode;
  qspi_mode_t address_mode;
  qspi_addr_size_t address_size;
  qspi_mode_t alternate_byte_mode;
  qspi_mode_t data_mode;
} qspi_command_t;

typedef struct qspi_driver_t qspi_driver_t;

/**
 * @brief QSPI 驱动操作
 */
typedef struct {
  /**
   * @brief Send Command
   */
  int (*command)(qspi_driver_t *self, qspi_command_t *cmd, uint32_t timeout);

  /**
   * @brief Transmit Data
   */
  int (*transmit)(qspi_driver_t *self, const uint8_t *data, uint32_t timeout);

  /**
   * @brief Receive Data
   */
  int (*receive)(qspi_driver_t *self, uint8_t *data, uint32_t timeout);

  /**
   * @brief 配置自动轮询模式（可选）
   */
  int (*auto_polling)(qspi_driver_t *self, qspi_command_t *cmd,
                      qspi_command_t *cfg, uint32_t timeout);

  /**
   * @brief 配置内存映射模式（可选）
   */
  int (*memory_mapped)(qspi_driver_t *self, qspi_command_t *cmd);

  /**
   * @brief Abort current operation (including memory mapped)
   */
  int (*abort)(qspi_driver_t *self);
} qspi_driver_ops_t;

/**
 * @brief QSPI Driver Base Structure
 */
struct qspi_driver_t {
  const qspi_driver_ops_t *ops;
  void *user_data;
};

/* Helper Macros */
#define QSPI_COMMAND(drv, cmd, t) ((drv)->ops->command(drv, cmd, t))
#define QSPI_TRANSMIT(drv, d, t) ((drv)->ops->transmit(drv, d, t))
#define QSPI_RECEIVE(drv, d, t) ((drv)->ops->receive(drv, d, t))
#define QSPI_AUTO_POLLING(drv, c, cfg, t)                                      \
  ((drv)->ops->auto_polling(drv, c, cfg, t))
#define QSPI_MEMORY_MAPPED(drv, c) ((drv)->ops->memory_mapped(drv, c))

#endif /* DRIVERS_INTERFACE_QSPI_DRIVER_H_ */
