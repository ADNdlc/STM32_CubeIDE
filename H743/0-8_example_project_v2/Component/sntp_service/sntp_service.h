#ifndef SNTP_SERVICE_H
#define SNTP_SERVICE_H

#include "sntp_driver.h"
#include <stdbool.h>

/**
 * @brief SNTP 服务状态
 */
typedef enum {
    SNTP_SVC_STATE_IDLE,
    SNTP_SVC_STATE_SYNCING,
    SNTP_SVC_STATE_SYNCED,
    SNTP_SVC_STATE_ERROR
} sntp_svc_state_t;

/**
 * @brief SNTP 服务结构体
 */
typedef struct {
    sntp_driver_t *drv;
    sntp_svc_state_t state;
    bool network_ready;
    uint32_t last_sync_time;
    uint32_t sync_interval_ms;
} sntp_service_t;

/**
 * @brief 初始化 SNTP 服务
 * @param self 服务实例
 * @param drv 底层驱动实例
 */
void sntp_svc_init(sntp_service_t *self, sntp_driver_t *drv);

/**
 * @brief 设置网络就绪状态
 */
void sntp_svc_set_network_ready(sntp_service_t *self, bool ready);

/**
 * @brief 触发一次同步
 * @param timezone 时区 (例如 8)
 * @param interval_ms 下次自动同步间隔 (0表示不自动同步)
 */
void sntp_svc_start_sync(sntp_service_t *self, int timezone, uint32_t interval_ms);

/**
 * @brief 定时处理函数
 */
void sntp_svc_process(sntp_service_t *self);

/**
 * @brief 获取当前同步状态
 */
sntp_svc_state_t sntp_svc_get_state(sntp_service_t *self);

#endif // SNTP_SERVICE_H
