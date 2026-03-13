/*
 * wifi_service.h
 *
 *  High-level WiFi management service.
 */

#ifndef COMPONENT_WIFI_SERVICE_WIFI_SERVICE_H_
#define COMPONENT_WIFI_SERVICE_WIFI_SERVICE_H_

#include "wifi_driver.h"
#include "service_id.h"

typedef struct wifi_service_t wifi_service_t;
// wifi状态变化回调原型
typedef void (*wifi_service_event_cb_t)(wifi_service_t *self,
                                        wifi_status_t status, void *user_data);

struct wifi_service_t {
  wifi_driver_t *driver; // 内部持有的驱动
  wifi_service_event_cb_t event_cb;
  void *user_data;
  wifi_status_t last_status; // 内部状态跟踪
};

void wifi_svc_init(wifi_service_t *self, wifi_id_t id);
void wifi_service_register_callback(wifi_service_t *self,
                                    wifi_service_event_cb_t cb,
                                    void *user_data);

void wifi_svc_process(wifi_service_t *self);
int wifi_svc_connect(wifi_service_t *self, const char *ssid, const char *pwd);
int wifi_svc_disconnect(wifi_service_t *self);
int wifi_svc_scan(wifi_service_t *self, wifi_scan_cb_t cb, void *arg);
wifi_status_t wifi_svc_get_status(wifi_service_t *self);
int wifi_svc_set_mode(wifi_service_t *self, wifi_mode_t mode);

#endif /* COMPONENT_WIFI_SERVICE_WIFI_SERVICE_H_ */
