#ifndef APPLICATION_HOME_SYSTEM_NET_MGR_H_
#define APPLICATION_HOME_SYSTEM_NET_MGR_H_

#include "wifi_driver.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 网络管理器,此模块
 * 
 */


/**
 * @brief 初始化网络管理器(wifi,mqtt,sntp,etc)
 * 
 */
void net_mgr_init(void);

/**
 * @brief 网络管理器循环处理函数
 * 
 */
void net_mgr_process(void);

/**
 * @brief 连接或断开WiFi
 * 
 * @param enable true:连接WiFi, false:断开WiFi
 */
void net_mgr_wifi_enable(bool enable);

/**
 * @brief 检查WiFi是否启用(与连接是否成功无关)
 * 
 * @return true 
 * @return false 
 */
bool net_mgr_wifi_is_enabled(void);

#endif /* APPLICATION_HOME_SYSTEM_NET_MGR_H_ */
