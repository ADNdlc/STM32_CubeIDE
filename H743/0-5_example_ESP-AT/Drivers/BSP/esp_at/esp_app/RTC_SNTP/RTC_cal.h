/*
 * RTC_cal.h
 *
 *  Created on: Aug 14, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_ESP_APP_RTC_SNTP_RTC_CAL_H_
#define BSP_ESP_AT_ESP_APP_RTC_SNTP_RTC_CAL_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
/**
 * @brief 初始化 RTC/SNTP 管理器
 */
void RTC_sntp_init(void);

/**
 * @brief 配置ESP模块的SNTP功能和服务器,最多三个,至少需要一个有效/可用服务器
 * @param timezone 时区, 例如中国的东八区为 8
 * @param server1 SNTP服务器地址1, 例如 "ntp.aliyun.com"
 *
 */
void RTC_sntp_configure(int8_t timezone, const char* s1, const char* s2, const char* s3);

/**
 * @brief 发起一次RTC同步请求
 *        这是一个非阻塞函数。它会检查模块的NTP时间是否就绪，
 *        如果是，则发送查询命令。校准将在后台通过回调处理。
 */
void RTC_calibrat_BY_NTP(void);

/**
 * @brief 查询STM32的RTC是否已经通过网络校准过
 * @return true 如果已校准, false 如果未校准
 */
bool RTC_is_calibrated(void);


/**
 * @brief 从STM32的RTC中获取当前的UTC时间戳
 *        它会读取RTC的本地时间，然后根据指定的时区偏移量计算出标准的UTC时间戳。
 *
 * @param timezone_offset 时区偏移量。例如：
 *                        - 东八区 (中国): 传入 8
 *                        - 零时区 (UTC): 传入 0
 *                        - 西五区 (美国东部): 传入 -5
 *
 * @return time_t 类型的UTC时间戳 (自1970-01-01 00:00:00 UTC以来的秒数)。
 *         如果RTC未设置或读取失败，可能返回 -1。
 */
time_t RTC_get_unix(int8_t timezone);

// ============ 由Dispatcher调用的URC处理函数 ==============
/**
 * @brief 处理 +TIME_UPDATED URC 消息
 *        当模块成功从NTP服务器获取到时间后，会主动上报此消息。
 */
void SNTP_handle_time_update(const char* line);

#endif /* BSP_ESP_AT_ESP_APP_RTC_SNTP_RTC_CAL_H_ */
