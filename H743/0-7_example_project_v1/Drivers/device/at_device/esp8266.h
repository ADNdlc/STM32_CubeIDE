#ifndef DRIVERS_DEVICE_AT_DEVICE_ESP8266_H_
#define DRIVERS_DEVICE_AT_DEVICE_ESP8266_H_

#include "at_device.h"

typedef struct esp8266_t esp8266_t;

/**
 * @brief ESP8266 TCP 接收回调 (由 plat_tcp 层处理)
 */
typedef void (*esp8266_tcp_recv_cb)(const uint8_t *data, size_t len,
                                    void *user_data);

/**
 * @brief 创建 ESP8266 实例
 */
esp8266_t *esp8266_create(uart_queue_t *uart_queue);

/**
 * @brief 初始化模块 (ATE0, CWMODE 等)
 */
int esp8266_init(esp8266_t *esp);

/**
 * @brief 连接 WiFi
 */
int esp8266_wifi_connect(esp8266_t *esp, const char *ssid, const char *pwd,
                         uint32_t timeout);

/**
 * @brief TCP 连接
 */
int esp8266_tcp_connect(esp8266_t *esp, const char *host, uint16_t port,
                        uint32_t timeout);

/**
 * @brief TCP 发送
 */
int esp8266_tcp_send(esp8266_t *esp, const uint8_t *data, size_t len,
                     uint32_t timeout);

/**
 * @brief TCP 断开
 */
int esp8266_tcp_disconnect(esp8266_t *esp);

/**
 * @brief 设置 TCP 接收回调
 */
void esp8266_set_tcp_recv_cb(esp8266_t *esp, esp8266_tcp_recv_cb cb,
                             void *user_data);

/**
 * @brief 周期性处理
 */
void esp8266_process(esp8266_t *esp);

#endif /* DRIVERS_DEVICE_AT_DEVICE_ESP8266_H_ */
