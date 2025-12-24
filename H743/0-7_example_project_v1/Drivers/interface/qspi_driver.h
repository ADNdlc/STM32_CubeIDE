/**
 * @file qspi_driver.h
 * @brief QSPI Driver Interface
 */

#ifndef DRIVERS_INTERFACE_QSPI_DRIVER_H_
#define DRIVERS_INTERFACE_QSPI_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief QSPI Command Structure
 */
typedef struct {
  uint8_t instruction;     ///< Command Instruction
  uint32_t address;        ///< Address (if applicable)
  uint32_t alternate_byte; ///< Alternate Byte (if applicable)
  uint32_t dummy_cycles;   ///< Number of dummy cycles
  uint32_t data_size;      ///< Size of data to transfer

  // Mode configuration (implementation dependent, usually enums for 1/2/4
  // lines)
  uint8_t instruction_mode;
  uint8_t address_mode;
  uint8_t address_size; ///< 24 or 32 bit
  uint8_t alternate_byte_mode;
  uint8_t data_mode;
  uint8_t dummy_cycles_cfg; // If needed specifically
} qspi_command_t;

typedef struct qspi_driver_t qspi_driver_t;

/**
 * @brief QSPI Driver Operations
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
   * @brief Configure Automatic Polling Mode (optional)
   */
  int (*auto_polling)(qspi_driver_t *self, qspi_command_t *cmd,
                      qspi_command_t *cfg, uint32_t timeout);

  /**
   * @brief Memory Mapped Mode (optional)
   */
  int (*memory_mapped)(qspi_driver_t *self, qspi_command_t *cmd);

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
