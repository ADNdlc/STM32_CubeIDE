#ifndef DRIVERS_PLATFORM_STM32_ONENET_PORT_ONENET_PORT_H_
#define DRIVERS_PLATFORM_STM32_ONENET_PORT_ONENET_PORT_H_

#include "esp8266.h"

/**
 * @brief 将 ESP8266 实例绑定到 OneNET TCP 适配层
 */
void plat_tcp_set_device(esp8266_t *esp);

#endif /* DRIVERS_PLATFORM_STM32_ONENET_PORT_ONENET_PORT_H_ */
