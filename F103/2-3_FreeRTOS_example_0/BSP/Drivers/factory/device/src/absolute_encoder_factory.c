#include "..\inc\absolute_encoder_factory.h"
#include "as5600/as5600.h"
#include "factory_inc.h"
#include <stddef.h>
#include <stdlib.h>

// 静态实例缓存
static absolute_encoder_driver_t *encoder_drivers[ENCODER_ID_MAX] = {NULL};

absolute_encoder_driver_t *absolute_encoder_driver_get(absolute_encoder_id_t id) {
    if (id >= ENCODER_ID_MAX) {
        return NULL;
    }
    if (encoder_drivers[id] == NULL) {
        const absolute_encoder_mapping_t *mapping = &absolute_encoder_mappings[id];
        if (!mapping || !mapping->resource) {
            return NULL;
        }
        i2c_driver_t *i2c_drv = i2c_driver_get((i2c_device_id_t)mapping->resource);
        if (i2c_drv) {
            encoder_drivers[id] = as5600_driver_create(i2c_drv);
        }
    }
    return encoder_drivers[id];
}
