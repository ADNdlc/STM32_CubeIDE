#ifndef DRIVERS_DEVICE_AT_DEVICE_AT_DEVICE_H_
#define DRIVERS_DEVICE_AT_DEVICE_AT_DEVICE_H_

#include "uart_queue/uart_queue.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief AT命令执行结果
 */
typedef enum {
  AT_RESULT_OK = 0,
  AT_RESULT_ERROR,
  AT_RESULT_TIMEOUT,
  AT_RESULT_BUSY
} at_result_t;

/**
 * @brief AT命令响应回调
 */
typedef void (*at_response_cb)(at_result_t result, const char *response,
                               void *user_data);

/**
 * @brief AT非占位符数据处理回调 (用于处理 +IPD 等 URC)
 */
typedef void (*at_urc_handler)(const char *line, void *user_data);

/**
 * @brief AT设备句柄
 */
typedef struct at_device_t at_device_t;

/**
 * @brief AT设备配置
 */
typedef struct {
  uart_queue_t *uart_queue;
  uint32_t default_timeout;
  at_urc_handler urc_handler;
  void *user_data;
} at_device_config_t;

/**
 * @brief 创建AT设备实例
 */
at_device_t *at_device_create(const at_device_config_t *config);

/**
 * @brief 销毁AT设备实例
 */
void at_device_destroy(at_device_t *device);

/**
 * @brief 发送AT指令 (异步)
 * @param device AT设备句柄
 * @param cmd 指令内容
 * @param timeout 超时时间 (0使用默认)
 * @param cb 回调函数
 * @param user_data 回调透传参数
 */
int at_device_send_cmd(at_device_t *device, const char *cmd, uint32_t timeout,
                       at_response_cb cb, void *user_data);

/**
 * @brief 发送纯数据 (用于 > 提示符后的逻辑)
 */
int at_device_send_data(at_device_t *device, const uint8_t *data, size_t len);

/**
 * @brief 周期性处理函数 (需在主循环调用)
 */
void at_device_process(at_device_t *device);

/**
 * @brief 进入原始数据接收模式
 * @param device AT设备句柄
 * @param len 需要接收的字节数
 */
void at_device_enter_raw_mode(at_device_t *device, uint32_t len);

#endif /* DRIVERS_DEVICE_AT_DEVICE_AT_DEVICE_H_ */
