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

// 定义测试函数的类型
typedef void (*TestFunc_t)(void);

// 测试用例结构体
typedef struct {
    const char* name;       // 测试名称
    const char* desc;       // 测试描述
    TestFunc_t setup;       // 资源初始化 (可为NULL)
    TestFunc_t loop;        // 循环执行逻辑 (可为NULL)
    TestFunc_t teardown;    // 资源释放 (可为NULL)
} TestCase_t;

/**
 * 核心宏定义
 * 1. __attribute__((section(".test_registry"))) : 告诉编译器把变量放到名为 .test_registry 的段中
 * 2. __attribute__((used)) : 告诉编译器即使没人显式调用，也不要优化掉它
 */
#define REGISTER_TEST(test_name, description, setup_fn, loop_fn, teardown_fn) \
    const TestCase_t test_##test_name \
    __attribute__((section(".test_registry"), used)) = { \
        .name = #test_name, \
        .desc = description, \
        .setup = setup_fn, \
        .loop = loop_fn, \
        .teardown = teardown_fn \
    }

// 公开给 main.c 的接口
void Test_Framework_Init(void);
void Test_Framework_Run(void);

#endif /* TEST_FRAMEWORK_H_ */
