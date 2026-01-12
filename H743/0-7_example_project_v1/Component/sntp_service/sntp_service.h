/*
 * sntp_service.h
 *
 *  Created on: Jan 12, 2026
 *      Author: 12114
 */

#ifndef COMPONENT_SNTP_SERVICE_SNTP_SERVICE_H_
#define COMPONENT_SNTP_SERVICE_SNTP_SERVICE_H_

#include "../../Drivers/interface/sntp_driver.h"
#include <stdbool.h>
#include <time.h>


typedef struct sntp_service_t sntp_service_t;

typedef enum {
  SNTP_SVC_STATUS_IDLE,
  SNTP_SVC_STATUS_SYNCING,
  SNTP_SVC_STATUS_SYNCED,
  SNTP_SVC_STATUS_ERROR
} sntp_svc_status_t;

// 事件回调
typedef void (*sntp_svc_event_cb_t)(sntp_service_t *self,
                                    sntp_svc_status_t status, void *user_data);

struct sntp_service_t {
  sntp_driver_t *driver;
  sntp_svc_status_t status;
  sntp_svc_event_cb_t event_cb;
  void *user_data;

  uint32_t sync_interval_ms; // 同步周期
  uint32_t last_sync_tick;   // 上次成功同步的系统滴答
  int timezone;              // 当前配置的时区

  bool wifi_ready; // 网络是否可用
};

/**
 * @brief 初始化 SNTP 服务
 */
void sntp_svc_init(sntp_service_t *self, sntp_driver_t *driver);

/**
 * @brief 注册事件回调
 */
void sntp_svc_register_callback(sntp_service_t *self, sntp_svc_event_cb_t cb,
                                void *user_data);

/**
 * @brief 设置网络状态
 */
void sntp_svc_set_network_ready(sntp_service_t *self, bool ready);

/**
 * @brief 开始/配置同步
 */
int sntp_svc_start_sync(sntp_service_t *self, int timezone,
                        uint32_t interval_ms);

/**
 * @brief 定期处理逻辑
 */
void sntp_svc_process(sntp_service_t *self);

/**
 * @brief 获取当前 Unix 时间戳 (由 RTC 维护)
 */
time_t sntp_svc_get_time(sntp_service_t *self);

#endif /* COMPONENT_SNTP_SERVICE_SNTP_SERVICE_H_ */
