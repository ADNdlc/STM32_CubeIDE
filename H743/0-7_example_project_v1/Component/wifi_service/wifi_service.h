/*
 * wifi_service.h
 *
 *  High-level WiFi management service.
 */

#ifndef COMPONENT_WIFI_SERVICE_WIFI_SERVICE_H_
#define COMPONENT_WIFI_SERVICE_WIFI_SERVICE_H_

#include "../../Drivers/interface/wifi_driver.h"

typedef struct wifi_service_t wifi_service_t;

// Event callback for status changes
typedef void (*wifi_service_event_cb_t)(wifi_service_t *svc,
                                        wifi_status_t status, void *user_data);

struct wifi_service_t {
  wifi_driver_t *driver;
  wifi_service_event_cb_t event_cb;
  void *user_data;
};

/**
 * @brief Initialize the WiFi service with a driver
 */
void wifi_service_init(wifi_service_t *self, wifi_driver_t *driver);

/**
 * @brief Register a status changed callback
 */
void wifi_service_register_callback(wifi_service_t *self,
                                    wifi_service_event_cb_t cb,
                                    void *user_data);

/**
 * @brief Connect to an AP
 */
int wifi_svc_connect(wifi_service_t *self, const char *ssid, const char *pwd);

/**
 * @brief Disconnect from AP
 */
int wifi_svc_disconnect(wifi_service_t *self);

/**
 * @brief Scan for available APs
 */
int wifi_svc_scan(wifi_service_t *self, wifi_scan_cb_t cb, void *arg);

/**
 * @brief Get current WiFi status
 */
wifi_status_t wifi_svc_get_status(wifi_service_t *self);

/**
 * @brief Set WiFi mode
 */
int wifi_svc_set_mode(wifi_service_t *self, wifi_mode_t mode);

/**
 * @brief Get scan result count
 */
uint16_t wifi_svc_get_scan_count(wifi_service_t *self);

/**
 * @brief Get scan result list
 */
wifi_ap_info_t *wifi_svc_get_scan_results(wifi_service_t *self);

#endif /* COMPONENT_WIFI_SERVICE_WIFI_SERVICE_H_ */
