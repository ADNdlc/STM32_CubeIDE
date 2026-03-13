/*
 * wifi_service.c
 *
 *  Implementation of the high-level WiFi service.
 */

#include "wifi_service.h"
#include <string.h>

void wifi_service_init(wifi_service_t *self, wifi_driver_t *driver) {
  if (!self || !driver)
    return;
  memset(self, 0, sizeof(wifi_service_t));
  self->driver = driver;
}

/**
 * @brief 注册状态变化回调
 *
 * @param self 实例
 * @param cb   回调函数
 * @param user_data 用户数据指针
 */
void wifi_service_register_callback(wifi_service_t *self,
                                    wifi_service_event_cb_t cb,
                                    void *user_data) {
  if (!self)
    return;
  self->event_cb = cb;
  self->user_data = user_data;
}

/**
 * @brief 处理WiFi服务
 *
 * @param self 实例
 */
void wifi_svc_process(wifi_service_t *self) {
  if (!self || !self->driver)
    return;

  // 状态轮询
  wifi_status_t current_status = WIFI_GET_STATUS(self->driver);

  // 如果状态变化,触发回调
  if (current_status != self->last_status) {
    if (self->event_cb) {
      self->event_cb(self, current_status, self->user_data);
    }
    self->last_status = current_status;
  }
}

/**
 * @brief 连接WiFi
 *
 * @param self 实例
 * @param ssid WiFi名称
 * @param pwd  WiFi密码
 * @return int 0:成功, -1:失败
 */
int wifi_svc_connect(wifi_service_t *self, const char *ssid, const char *pwd) {
  if (!self || !self->driver)
    return -1;
  return WIFI_CONNECT(self->driver, ssid, pwd);
}

/**
 * @brief 断开WiFi连接
 *
 * @param self 实例
 * @return int 0:成功, -1:失败
 */
int wifi_svc_disconnect(wifi_service_t *self) {
  if (!self || !self->driver)
    return -1;
  return WIFI_DISCONNECT(self->driver);
}

/**
 * @brief 扫描WiFi网络
 *
 * @param self  实例
 * @param cb    扫描回调函数(传递结果)
 * @param arg   用户数据指针
 * @return int
 */
int wifi_svc_scan(wifi_service_t *self, wifi_scan_cb_t cb, void *arg) {
  if (!self || !self->driver)
    return -1;
  return WIFI_SCAN(self->driver, cb, arg);
}

/**
 * @brief 获取当前WiFi状态
 *
 * @param self 实例
 * @return wifi_status_t 当前状态
 */
wifi_status_t wifi_svc_get_status(wifi_service_t *self) {
  if (!self || !self->driver)
    return WIFI_STATUS_DISCONNECTED;
  return WIFI_GET_STATUS(self->driver);
}

/**
 * @brief 设置WiFi工作模式
 *
 * @param self 实例
 * @param mode 工作模式
 * @return int 0:成功, -1:失败
 */
int wifi_svc_set_mode(wifi_service_t *self, wifi_mode_t mode) {
  if (!self || !self->driver)
    return -1;
  return WIFI_SET_MODE(self->driver, mode);
}
