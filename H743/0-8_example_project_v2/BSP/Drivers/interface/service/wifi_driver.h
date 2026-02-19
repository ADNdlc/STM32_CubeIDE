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
  WIFI_MODE_MIXED = 3,
} wifi_mode_t;

typedef enum {
  WIFI_STATUS_DISCONNECTED = 0,
  WIFI_STATUS_CONNECTING,
  WIFI_STATUS_CONNECTED,
  WIFI_STATUS_GOT_IP,
} wifi_status_t;

typedef struct {
  char ssid[30];  // <ssid>
  char mac[18];   // mac地址
  int8_t rssi;    // 信号强度
  uint8_t channel;// 信道号
  uint8_t encryption;      // <ecn>,加密方式
  uint8_t pairwise_cipher; // <pairwise_cipher>,成对加密类型
  uint8_t group_cipher;    // <group_cipher>,组加密类型与pairwise_cipher相同
  uint8_t protocol;        // <wifi_protocol>,支持的协议类型
  uint8_t wps;             // <wps>,是否支持WPS
} wifi_ap_info_t;

typedef struct{
	uint16_t count;
	wifi_ap_info_t *ap_info;
} wifi_ap_list_t;

/**
 * @brief WiFi扫描结果回调原型
 *
 * @param arg       用户数据指针
 * @param ap_list   扫描到的AP列表
 */
typedef void (*wifi_scan_cb_t)(void *arg, wifi_ap_list_t *ap_list);

typedef struct wifi_driver_t wifi_driver_t;
/**
 * @brief WiFi驱动操作接口
 */
typedef struct {
  int (*set_mode)(wifi_driver_t *self, wifi_mode_t mode); // 设置本机模式
  int (*connect)(wifi_driver_t *self, const char *ssid, const char *pwd);// 连接WiFi
  int (*disconnect)(wifi_driver_t *self); // 断开WiFi连接
  int (*scan)(wifi_driver_t *self, wifi_scan_cb_t cb, void *arg);// 扫描AP
  wifi_status_t (*get_status)(wifi_driver_t *self); // 获取当前连接状态
  wifi_mode_t (*get_mode)(wifi_driver_t *self); // 获取当前模式态
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
