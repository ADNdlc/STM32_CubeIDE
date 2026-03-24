/*
 * sntp_driver.h
 *
 *  Created on: Jan 12, 2026
 *      Author: 12114
 */

#ifndef INTERFACE_SNTP_DRIVER_H_
#define INTERFACE_SNTP_DRIVER_H_

#include <stddef.h>
#include <stdint.h>


/**
 * @brief SNTP 驱动状态
 */
typedef enum {
  SNTP_DRV_TATUS_NOCONFIGS,// 未配置
  SNTP_DRV_STATUS_WAITING, // 等待网络更新 (URC)
  SNTP_DRV_STATUS_SYNCED,  // 已同步
  SNTP_DRV_STATUS_ERROR
} sntp_drv_status_t;

typedef struct sntp_driver_t sntp_driver_t;

/**
 * @brief 获取 SNTP 时间回调
 * @param self 驱动实例
 * @param result 获取结果
 * @param time_str 时间字符串
 * @param user_data 回调上下文
 * 
 */
typedef void (*sntp_get_time_cb_t)(sntp_driver_t *self, int result,
                                   const char *time_str, void *user_data);

typedef struct {
  /**
   * @brief 配置 SNTP
   */
  int (*config)(sntp_driver_t *self, int enable, int timezone,
                const char *server1);

  /**
   * @brief 获取当前 SNTP 时间字符串 (异步)
   * @param cb 结果回调
   * @param user_data 回调上下文
   * @return 0 成功提交，其他失败
   */
  int (*get_time)(sntp_driver_t *self, sntp_get_time_cb_t cb, void *user_data);

  /**
   * @brief 获取驱动维护的状态
   */
  sntp_drv_status_t (*get_status)(sntp_driver_t *self);
} sntp_driver_ops_t;

struct sntp_driver_t {
  const sntp_driver_ops_t *ops;
};

#endif /* INTERFACE_SNTP_DRIVER_H_ */
