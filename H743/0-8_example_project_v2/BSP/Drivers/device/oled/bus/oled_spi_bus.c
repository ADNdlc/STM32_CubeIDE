/*
 * oled_spi_bus.c
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */
#include "MemPool.h"
#include "oled_spi_bus.h"

#define OLED_SPI_MEMSOURCE SYS_MEM_INTERNAL

static int oled_spi_bus_write(oled_bus_t *self, oled_data_type_t type,
                              const uint8_t *data, uint32_t len);

oled_bus_t oled_spi_bus = {
    .write = oled_spi_bus_write,
};

static int oled_spi_bus_write(oled_bus_t *self, oled_data_type_t type,
                              const uint8_t *data, uint32_t len) {

}


oled_bus_t *OLED_SPI_Bus_Create(bus_drivers_t *bus_drivers, const oled_spi_config_t *config) {

}
