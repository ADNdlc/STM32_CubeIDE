/*
 * PowerMonitor_factory.c
 *
 *  Created on: Feb 22, 2026
 *      Author: 12114
 */

#if (DEV_POWER_MONITOR == USE_INA219)
#include "i2c_factory.h"
#include "ina219/ina219_driver.h"
#include "timer_factory.h"
#endif

static PowerMonitor_driver_t *power_monitor_drivers[POWER_MONITOR_MAX] = {NULL};

//PowerMonitor_driver_t *PowerMonitor_factory_get(PowerMonitor_id_t id){
//  if (id >= NOR_FLASH_MAX) {
//    return NULL;
//  }
//
//  if (power_monitor_drivers[id] == NULL) {
//    const norflash_mapping_t *mapping = &norflash_mappings[id];
//#if (DEV_POWER_MONITOR == USE_INA219)
//    i2c_driver_t *i2c_driver = stm32_i2c_soft_driver_create();
//    PowerMonitor_driver_t *power_monitor_drv =
//        ina219_create();
//    if (power_monitor_drv == NULL) {
//      return NULL;
//    }
//
//#endif
//  }
//  return power_monitor_drivers[id];
//}
