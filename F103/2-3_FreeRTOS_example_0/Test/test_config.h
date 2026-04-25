/*
 * test_config.h
 *
 *  Created on: Feb 5, 2026
 *      Author: 12114
 */

#ifndef TEST_CONFIG_H_
#define TEST_CONFIG_H_
#include "Project_cfg.h"

//#include "MemPool.h"
#include "Sys.h"
#include "elog.h"

#include "interface_inc.h"
#include "factory_inc.h"

#if TEST_ENABLE
#define ENABLE_TEST_LED 1
#define ENABLE_TEST_PWM 0
#define ENABLE_TEST_PWM_LED 0
#endif

#endif /* TEST_CONFIG_H_ */
