/*
 * all_tests.h
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */

#ifndef TEST_UNIT_ALL_TESTS_H_
#define TEST_UNIT_ALL_TESTS_H_

#include "sys.h"
#include <stdio.h>
#include <string.h>

#define _led_test_ 1
#define _key_test_ 1
#define _uart_hal_test_ 1
#define _uart_queue_test_ 1
#define _elog_test_ 1
#define _sdram_test_ 1


// 运行所有测试用例
void run_all_tests(void);

#endif /* TEST_UNIT_ALL_TESTS_H_ */
