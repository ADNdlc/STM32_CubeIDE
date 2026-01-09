/**
 * @file spi_driver.h
 * @brief SPI Driver Interface
 */

#ifndef DRIVERS_INTERFACE_SPI_DRIVER_H_
#define DRIVERS_INTERFACE_SPI_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct spi_driver_t spi_driver_t;

/**
 * @brief SPI Driver Operations
 */
typedef struct {
  /**
   * @brief Transmit data
   */
  int (*transmit)(spi_driver_t *self, const uint8_t *data, size_t size,
                  uint32_t timeout);

  /**
   * @brief Receive data
   */
  int (*receive)(spi_driver_t *self, uint8_t *data, size_t size,
                 uint32_t timeout);

  /**
   * @brief Transmit and Receive (Full Duplex)
   */
  int (*transmit_receive)(spi_driver_t *self, const uint8_t *tx_data,
                          uint8_t *rx_data, size_t size, uint32_t timeout);

  /**
   * @brief Select Chip (CS)
   * @param state 0 for Low (Active), 1 for High (Inactive)
   */
  void (*cs_control)(spi_driver_t *self, uint8_t state);

} spi_driver_ops_t;

/**
 * @brief SPI Driver Base Structure
 */
struct spi_driver_t {
  const spi_driver_ops_t *ops;
  void *user_data;
};

/* Helper Macros */
#define SPI_TRANSMIT(drv, d, s, t) ((drv)->ops->transmit(drv, d, s, t))
#define SPI_RECEIVE(drv, d, s, t) ((drv)->ops->receive(drv, d, s, t))
#define SPI_TRANSMIT_RECEIVE(d, tx, rx, s, t)                                  \
  ((d)->ops->transmit_receive(d, tx, rx, s, t))
#define SPI_CS_CONTROL(drv, state)                                             \
  do {                                                                         \
    if ((drv)->ops->cs_control)                                                \
      (drv)->ops->cs_control(drv, state);                                      \
  } while (0)

#endif /* DRIVERS_INTERFACE_SPI_DRIVER_H_ */