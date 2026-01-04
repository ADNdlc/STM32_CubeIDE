/*
 * wifi_driver.h
 *
 *  Hardware abstraction layer for WiFi drivers.
 */

#ifndef DRIVERS_INTERFACE_WIFI_DRIVER_H_
#define DRIVERS_INTERFACE_WIFI_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  WIFI_MODE_STATION = 1,
  WIFI_MODE_SOFTAP = 2,
  WIFI_MODE_STATION_SOFTAP = 3,
} wifi_mode_t;

typedef enum {
  WIFI_STATUS_DISCONNECTED = 0,
  WIFI_STATUS_CONNECTING,
  WIFI_STATUS_CONNECTED,
  WIFI_STATUS_GOT_IP,
} wifi_status_t;

typedef struct {
  char ssid[33];
  char mac[18];
  int8_t rssi;
  uint8_t channel;
  uint8_t encryption;      // <ecn>
  uint8_t pairwise_cipher; // <pairwise_cipher>
  uint8_t group_cipher;    // <group_cipher>
  uint8_t protocol;        // <wifi_protocol>
  uint8_t wps;             // <wps>
} wifi_ap_info_t;

/**
 * @brief WiFi扫描结果回调
 *
 * @param arg       用户数据指针
 * @param ap_list   扫描到的AP列表
 * @param count     AP数量
 */
typedef void (*wifi_scan_cb_t)(void *arg, wifi_ap_info_t *ap_list,
                               uint16_t count);

typedef struct wifi_driver_t wifi_driver_t;

typedef struct {
  int (*set_mode)(wifi_driver_t *self, wifi_mode_t mode);
  int (*connect)(wifi_driver_t *self, const char *ssid, const char *pwd);
  int (*disconnect)(wifi_driver_t *self);
  int (*scan)(wifi_driver_t *self, wifi_scan_cb_t cb, void *arg);
  wifi_status_t (*get_status)(wifi_driver_t *self);
} wifi_driver_ops_t;

struct wifi_driver_t {
  const wifi_driver_ops_t *ops;
};

// Helper macros
#define WIFI_SET_MODE(drv, mode) (drv)->ops->set_mode(drv, mode)
#define WIFI_CONNECT(drv, s, p) (drv)->ops->connect(drv, s, p)
#define WIFI_DISCONNECT(drv) (drv)->ops->disconnect(drv)
#define WIFI_SCAN(drv, cb, arg) (drv)->ops->scan(drv, cb, arg)
#define WIFI_GET_STATUS(drv) (drv)->ops->get_status(drv)

#endif /* DRIVERS_INTERFACE_WIFI_DRIVER_H_ */
