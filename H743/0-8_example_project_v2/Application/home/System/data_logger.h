#ifndef APPLICATION_HOME_SYSTEM_DATA_LOGGER_H_
#define APPLICATION_HOME_SYSTEM_DATA_LOGGER_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 初始化本地数据日志记录模块
 *        注册物模型事件监听，并执行其他需要的初始化（RTC等）。
 */
void data_logger_init(void);

/**
 * @brief 处理缓冲区滞留的日志。
 *        由系统主循环定期调用，将缓冲区日志写入VFS。
 */
void data_logger_process(void);

#endif /* APPLICATION_HOME_SYSTEM_DATA_LOGGER_H_ */
