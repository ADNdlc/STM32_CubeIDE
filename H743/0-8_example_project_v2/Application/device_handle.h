#ifndef APPLICATION_HOME_SYSTEM_DEVICE_INIT_H_
#define APPLICATION_HOME_SYSTEM_DEVICE_INIT_H_

/**
 * @brief device_handle
 *
 * 这里作为所有设备的注册点(静态注册)
 * TODO: 动态注册和云端子设备绑定逻辑
 *
 */

/**
 * @brief 初始化并注册所有设备
 */
void devices_init(void);

/**
 * @brief 设备处理函数
 */
void devices_process(void);

#endif /* APPLICATION_HOME_SYSTEM_DEVICE_INIT_H_ */
