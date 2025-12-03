/*
 * main_test_example.c
 *
 * 这是一个示例文件，展示如何在 main.c 中调用测试用例
 * 请将相关代码复制到你的 main.c 中
 */

// ============ 在 main.c 顶部添加头文件 ============
#include "all_tests.h"
// 或者单独包含需要的测试
// #include "uart_hal_test.h"
// #include "uart_queue_test.h"

// ============ 在 main() 函数中调用测试 ============

int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  // ... 其他初始化 ...

  /* USER CODE BEGIN 2 */

  // 方法 1: 运行所有测试（推荐）
  run_all_tests();

  // 方法 2: 只运行特定测试
  // uart_hal_test_run();
  // uart_queue_test_run();

  /* USER CODE END 2 */

  /* Infinite loop */
  while (1) {
    /* USER CODE BEGIN 3 */

    // 如果需要循环运行测试，可以在这里调用
    // HAL_Delay(5000);  // 每 5 秒运行一次
    // run_all_tests();

    /* USER CODE END 3 */
  }
}

// ============ 使用说明 ============
/*
 * 1. 确保 USART1 已正确配置为调试串口
 * 2. 确保中断已启用（如果使用异步模式）
 * 3. 连接串口工具（115200 波特率）
 * 4. 编译并下载程序
 * 5. 在串口工具中查看测试输出
 *
 * 预期输出：
 * ========== UART HAL Test Start ==========
 * UART HAL Sync Transmit Test
 * Sync transmit: OK
 * ...
 * ========== UART HAL Test End ==========
 *
 * ========== UART Queue Test Start ==========
 * UART Queue Test: Message 1
 * ...
 * ========== UART Queue Test End ==========
 */
