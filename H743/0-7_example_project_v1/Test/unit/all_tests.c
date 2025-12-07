/*
 * all_tests.c
 *
 *  Created on: Dec 3, 2025
 *      Author: 12114
 */

#include "all_tests_config.h"
#include "led_test.h"
#include "key_test.h"
#include "uart_hal_test.h"
#include "uart_queue_test.h"


void run_all_tests(void)
{
#if led_test
    led_test_run();
#endif

#if key_test
    key_test_run();
#endif

#if uart_hal_test
    uart_hal_test_run();
#endif

#if uart_queue_test
    uart_queue_test_run();
#endif
}
