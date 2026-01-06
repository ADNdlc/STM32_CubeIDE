#ifndef APPLICATION_HOME_SYSTEM_CLOUD_BRIDGE_H_
#define APPLICATION_HOME_SYSTEM_CLOUD_BRIDGE_H_

#include "../../../Component/mqtt_service/mqtt_service.h"
#include "thing_model.h"

/**
 * @brief Initialize the cloud bridge
 *
 * @param mqtt_svc The MQTT service instance to use
 */
void cloud_bridge_init(mqtt_service_t *mqtt_svc);

/**
 * @brief Process cloud synchronization (Periodic Polling)
 *        Call this in the main loop or net_mgr_process
 */
void cloud_bridge_process(void);

#endif /* APPLICATION_HOME_SYSTEM_CLOUD_BRIDGE_H_ */
