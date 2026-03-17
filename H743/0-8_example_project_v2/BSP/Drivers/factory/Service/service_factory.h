/*
 * service_factory.h
 *
 *  Created on: Mar 17, 2026
 *      Author: 12114
 */

#ifndef DRIVERS_FACTORY_SERVICE_SERVICE_FACTORY_H_
#define DRIVERS_FACTORY_SERVICE_SERVICE_FACTORY_H_

void service_factory_init(void);

/**
 * @brief 处理服务相关的底层 URC处理 和 AT 状态机
 *        (应当在 main loop 中持续调用)
 */
void service_factory_process(void);

#endif /* DRIVERS_FACTORY_SERVICE_SERVICE_FACTORY_H_ */
