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
#define ENABLE_TEST_LED 0
#define ENABLE_TEST_ILLUMINANCE 0
#define ENABLE_TEST_RTC 0
#define ENABLE_TEST_POWER 0
#define ENABLE_TEST_TIMER 1
#endif

#endif /* TEST_CONFIG_H_ */
