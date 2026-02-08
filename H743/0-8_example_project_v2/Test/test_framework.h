/*
 * test_framework.h
 *
 *  Created on: Feb 5, 2026
 *      Author: 12114
 */
#ifndef TEST_FRAMEWORK_H_
#define TEST_FRAMEWORK_H_

#include <stdint.h>
#include <stdio.h>
#include "Project_cfg.h"

#if TEST_ENABLE
typedef void (*TestFunc_t)(void);

typedef struct
{
  const char *name;
  const char *desc;
  TestFunc_t setup;
  TestFunc_t loop;
  TestFunc_t teardown;
} TestCase_t;

/**
 * 核心宏定义
 * 1. __attribute__((section(".test_registry"))) : 告诉编译器把变量放到名为
 * .test_registry 的段中
 * 2. __attribute__((used)) : 告诉编译器即使没人显式调用，也不要优化掉它
 */
#define REGISTER_TEST(test_name, description, setup_fn, loop_fn, teardown_fn) \
  const TestCase_t test_##test_name __attribute__((                           \
      section(".test_registry"), used)) = {.name = #test_name,                \
                                           .desc = description,               \
                                           .setup = setup_fn,                 \
                                           .loop = loop_fn,                   \
                                           .teardown = teardown_fn}
#else
// 当TEST_ENABLE未定义时，将宏和函数替换为空操作
#define REGISTER_TEST(test_name, description, setup_fn, loop_fn, teardown_fn) ((void)0)
static inline void Test_Framework_Init(void) { (void)0; }
static inline void Test_Framework_Run(void) { (void)0; }
static inline void Test_Framework_HandleInput(uint8_t cmd) { (void)cmd; (void)0; }
static inline void Test_List_All(void) { (void)0; }
#endif

void Test_Framework_Init(void);
void Test_Framework_Run(void);
void Test_Framework_HandleInput(uint8_t cmd);
void Test_List_All(void);

#endif /* TEST_FRAMEWORK_H_ */
