/*
 * sdram_driver.h
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#ifndef BSP_DEVICE_DRIVER_INTERFACE_SDRAM_DRIVER_H_
#define BSP_DEVICE_DRIVER_INTERFACE_SDRAM_DRIVER_H_

#include <stddef.h>
#include <stdint.h>


// Forward declaration
typedef struct sdram_driver_t sdram_driver_t;

typedef struct {
  // Initializes the SDRAM device (runs the startup sequence)
  int (*init)(sdram_driver_t *self);
  // Sends a command to the SDRAM controller
  int (*send_command)(sdram_driver_t *self, uint32_t command_mode,
                      uint32_t target_bank, uint32_t auto_refresh_num,
                      uint32_t mode_reg_def);
} sdram_driver_ops_t;

struct sdram_driver_t {
  const sdram_driver_ops_t *ops;
};

// Helper macros
#define SDRAM_INIT(driver) (driver)->ops->init(driver)

#define SDRAM_SEND_COMMAND(driver, mode, target, refresh, reg)                 \
  (driver)->ops->send_command(driver, mode, target, refresh, reg)

#endif /* BSP_DEVICE_DRIVER_INTERFACE_SDRAM_DRIVER_H_ */
