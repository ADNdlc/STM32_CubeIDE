#ifndef APPLICATION_HOME_SYSTEM_NET_MGR_H_
#define APPLICATION_HOME_SYSTEM_NET_MGR_H_

#include "../../../Drivers/interface/wifi_driver.h"
#include <stdbool.h>
#include <stdint.h>


/**
 * @brief Network Manager (WiFi, MQTT, SNTP)
 */

void net_mgr_init(void);
void net_mgr_process(void);

// WiFi Control
void net_mgr_wifi_enable(bool enable);
bool net_mgr_wifi_is_enabled(void);

#endif /* APPLICATION_HOME_SYSTEM_NET_MGR_H_ */
