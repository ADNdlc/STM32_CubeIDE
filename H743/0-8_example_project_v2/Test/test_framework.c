/*
 * test_framework.c
 *
 *  Created on: Feb 5, 2026
 *      Author: 12114
 */

#include "test_framework.h"
#include <string.h>

// 引用链接脚本中的符号
extern const TestCase_t _test_start;
extern const TestCase_t _test_end;

// 输入缓冲区 (用于接收串口指令)
static TestCase_t* current_test = NULL;

void Test_List_All(void) {
    const TestCase_t* iter = &_test_start;
    const TestCase_t* end = &_test_end;
    int idx = 0;

    printf("\r\n=== Test Framework Menu ===\r\n");
    for (; iter < end; iter++) {
        printf("[%d] %s : %s\r\n", idx++, iter->name, iter->desc);
    }
    printf("Enter ID to run test.\r\n");
}

void Test_Framework_Init(void) {
    // 可以在这里做一些基础初始化
    Test_List_All();
}

// 根据索引查找并运行测试
void Test_Select_And_Run(int index) {
    const TestCase_t* iter = &_test_start;
    const TestCase_t* end = &_test_end;

    if (index >= (end - iter)) {
        printf("Invalid Index!\r\n");
        return;
    }

    // 清理上一个测试的资源
    if (current_test && current_test->teardown) {
        printf("Stopping: %s\r\n", current_test->name);
        current_test->teardown();
    }

    // 切换当前测试
    current_test = (TestCase_t*)(iter + index);
    printf("Starting: %s\r\n", current_test->name);

    // 初始化新测试
    if (current_test->setup) {
        current_test->setup();
    }
}

// 在 main while(1) 中调用
void Test_Framework_Run(void) {
    // 执行当前选中的测试循环
    if (current_test && current_test->loop) {
        current_test->loop();
    }
}
