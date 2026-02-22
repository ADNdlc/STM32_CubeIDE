/*
 * PowerMonitor_factory.h
 *
 *  Created on: Feb 22, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_FACTORY_DEVICE_INC_POWERMONITOR_FACTORY_H_
#define DRIVERS_FACTORY_DEVICE_INC_POWERMONITOR_FACTORY_H_

#include "PowerMonitor_driver.h"
#include "dev_map.h"

PowerMonitor_driver_t *PowerMonitor_factory_get(PowerMonitor_id_t id);

#endif /* DRIVERS_FACTORY_DEVICE_INC_POWERMONITOR_FACTORY_H_ */
