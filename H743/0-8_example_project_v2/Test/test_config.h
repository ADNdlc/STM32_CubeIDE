/*
 * test_config.h
 *
 *  Created on: Feb 5, 2026
 *      Author: 12114
 */

#ifndef TEST_CONFIG_H_
#define TEST_CONFIG_H_
#include "Project_cfg.h"

#include "MemPool.h"
#include "Sys.h"
#include "elog.h"

#if TEST_ENABLE
#define ENABLE_TEST_LED 1
#define ENABLE_TEST_SDRAM 0
#define ENABLE_TEST_HUMITURE 0
#define ENABLE_TEST_USART_QUEUE 0
#define ENABLE_TEST_ILLUMINANCE 0
#define ENABLE_TEST_TOUCH 0
#define ENABLE_TEST_RTC 0
#define ENABLE_TEST_FLASH 0
#define ENABLE_TEST_FLASH_HAL 0
#define ENABLE_TEST_POWER_MONITOR 0
#define ENABLE_TEST_TIMER 0
#define ENABLE_TEST_ASSET_MANAGER 0
#define ENABLE_TEST_LCD 0
#define ENABLE_TEST_KEY 0
#define ENABLE_TEST_LVGL 0
#define ENABLE_TEST_WIFI 0
#define ENABLE_TEST_MQTT 1
#define ENABLE_TEST_SNTP 0
#endif

#endif /* TEST_CONFIG_H_ */
