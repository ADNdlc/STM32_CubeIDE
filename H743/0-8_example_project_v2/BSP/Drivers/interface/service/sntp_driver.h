#ifndef DRIVERS_INTERFACE_SNTP_DRIVER_H_
#define DRIVERS_INTERFACE_SNTP_DRIVER_H_

#include "../Timer/rtc_driver.h" // 包含 rtc_time_t 和 rtc_date_t
#include <stdint.h>

/**
 * @brief SNTP 同步状态
 */
typedef enum {
    SNTP_DRV_STATUS_IDLE = 0,    // 空闲
    SNTP_DRV_STATUS_PENDING,     // 同步中
    SNTP_DRV_STATUS_SUCCESS,     // 同步成功
    SNTP_DRV_STATUS_FAIL,        // 同步失败
} sntp_drv_status_t;

/**
 * @brief SNTP 时间结果
 */
typedef struct {
    rtc_time_t time;
    rtc_date_t date;
} sntp_time_t;

/**
 * @brief SNTP 事件回调
 * @param arg 用户参数
 * @param result 同步结果 (0:成功, 其他:失败)
 * @param time 得到的时间数据 (仅成功时有效)
 */
typedef void (*sntp_sync_cb_t)(void *arg, int result, sntp_time_t *time);

typedef struct sntp_driver_t sntp_driver_t;

/**
 * @brief SNTP 驱动接口
 */
typedef struct {
    int (*config)(sntp_driver_t *self, int timezone, const char *server1);
    int (*start_sync)(sntp_driver_t *self, sntp_sync_cb_t cb, void *arg);
    sntp_drv_status_t (*get_status)(sntp_driver_t *self);
} sntp_driver_ops_t;

struct sntp_driver_t {
    const sntp_driver_ops_t *ops;
};

// 辅助宏
#define SNTP_DRV_CONFIG(d, tz, s) (d)->ops->config(d, tz, s)
#define SNTP_DRV_START_SYNC(d, c, a) (d)->ops->start_sync(d, c, a)
#define SNTP_DRV_GET_STATUS(d) (d)->ops->get_status(d)

#endif /* DRIVERS_INTERFACE_SNTP_DRIVER_H_ */
