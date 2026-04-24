#include "test_framework.h"
#include "BSP_init.h"
#include "test_config.h"
#include <string.h>

#if TEST_ENABLE

// 使用链接器自动生成的符号
extern const TestCase_t Image$$ER_TEST_REGISTRY$$Base;
extern const TestCase_t Image$$ER_TEST_REGISTRY$$Limit;

// 当前活动的测试用例
static TestCase_t *current_test = NULL;

void Test_List_All(void) {
  const TestCase_t *iter = &Image$$ER_TEST_REGISTRY$$Base;
  const TestCase_t *end = &Image$$ER_TEST_REGISTRY$$Limit;
  int idx = 0;

  log_i("========================================");
  log_i("       Test Framework Menu             ");
  log_i("========================================");
  for (; iter < end; iter++) {
    log_i("[%d] %-10s : %s", idx++, iter->name, iter->desc);
  }
  log_i("----------------------------------------");
  log_i("Press [0-9] to select a test case.");
  log_i("Press [m] to show this menu. [s] to stop.");
  log_i("========================================");
}

void Test_Framework_Init(void) {
  log_i("Test Framework Initializing...");
  Test_List_All();
}

void Stop_Current_Test(void) {
  if (current_test && current_test->teardown) {
    log_i("Teardown test: %s", current_test->name);
    current_test->teardown();
    current_test = NULL;
  }
}

// 根据索引查找并运行测试
void Test_Select_And_Run(int index) {
  const TestCase_t *iter = &Image$$ER_TEST_REGISTRY$$Base;
  const TestCase_t *end = &Image$$ER_TEST_REGISTRY$$Limit;
  int total_tests = end - iter;

  if (index < 0 || index >= total_tests) {
    log_w("Invalid Index: %d (Total: %d)", index, total_tests);
    return;
  }

  // 1. 停止上一个测试
  Stop_Current_Test();

  // 2. 切换当前测试
  current_test = (TestCase_t *)(iter + index);
  log_i("Switch to test: [%s]", current_test->name);

  // 3. 启动新测试
  if (current_test->setup) {
    current_test->setup();
  }
}

void Test_Framework_HandleInput(uint8_t cmd) {
  // 调试：输出接收到的实际值
  log_i("Received cmd: 0x%02X (%d), '0'=0x%02X, '9'=0x%02X", 
        cmd, cmd, (uint8_t)'0', (uint8_t)'9');
  
  // 忽略回车符和换行符
  if (cmd == '\r' || cmd == '\n') {
    return;
  }
  
  if (cmd == 'm') {
    Test_List_All();
  } else if (cmd == 's') {
    Stop_Current_Test();
  } else if (cmd >= '0' && cmd <= '9') {
    Test_Select_And_Run(cmd - '0');
  } else {
    log_w("Unknown command: 0x%02X ('%c')", cmd, cmd);
  }
}

// 在 main while(1) 中调用
void Test_Framework_Run(void) {
  uint8_t rx_char;

  // 1. 串口指令解析：使用统一的全局队列获取数据
  if (g_debug_queue) {
    while (uart_queue_getdata(g_debug_queue, &rx_char, 1) > 0) {
      Test_Framework_HandleInput(rx_char);
    }
  }

  // 2. 执行当前选中的测试循环逻辑
  if (current_test && current_test->loop) {
    current_test->loop();
  }
}

#endif // TEST_ENABLE
