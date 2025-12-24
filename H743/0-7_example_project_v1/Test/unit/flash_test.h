/*
 * flash_test.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  Flash 驱动测试用例
 */

#ifndef UNIT_FLASH_TEST_H_
#define UNIT_FLASH_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

// 运行所有 Flash 测试
void flash_test_run(void);

// 单元测试
void flash_test_driver(void);   // 驱动基础测试
void flash_test_handler(void);  // Handler 测试
void flash_test_factory(void);  // 工厂模式测试
void flash_test_strategy(void); // 策略模式测试

// 集成测试
void flash_integration_test_run(void);

#ifdef __cplusplus
}
#endif

#endif /* UNIT_FLASH_TEST_H_ */
