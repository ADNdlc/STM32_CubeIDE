#UART 测试用例说明

本目录包含了 UART HAL 和 UART Queue 的完整测试用例。

        ##文件结构

``` Test /
        unit /
├── uart_hal_test.h #UART HAL 测试头文件
├── uart_hal_test.c #UART HAL 测试实现
├── uart_queue_test.h #UART Queue 测试头文件
├── uart_queue_test.c #UART Queue 测试实现
├── all_tests.h #统一测试入口头文件
├── all_tests.c #统一测试入口实现
└── README_UART_TEST.md #本说明文件
```

        ##测试用例说明

        ## #1. UART HAL
        测试(`uart_hal_test.c`)

            测试 USART 硬件抽象层的基本功能：

    - **同步发送测试 **(`test_uart_hal_sync_transmit`) - 测试阻塞式数据发送
    - 验证发送结果

    - **同步接收测试 **(`test_uart_hal_sync_receive`) - 测试阻塞式数据接收
    - 带超时机制 - 回显接收到的数据

    - **异步发送测试 **(`test_uart_hal_async_transmit`) - 测试非阻塞式数据发送
    - 使用回调函数通知发送完成 - 验证回调机制

    - **异步接收测试 **(`test_uart_hal_async_receive`) - 测试非阻塞式数据接收
    - 使用回调函数通知接收完成 -
    回显接收到的数据

    ## #2. UART Queue 测试(`uart_queue_test.c`)

        测试 UART 队列的高级功能：

    - **基本发送测试 **(`test_uart_queue_basic_send`) - 测试单条消息发送
    - 测试连续发送多条消息

    - **批量发送测试 **(`test_uart_queue_batch_send`) - 测试批量发送多条消息
    - 验证队列自动管理 - 检查队列状态

    - **缓冲区溢出测试 **(`test_uart_queue_overflow`) - 测试缓冲区满时的处理
    - 统计成功和失败次数 - 验证错误处理机制

    - **接收测试 **(`test_uart_queue_receive`) - 测试异步接收功能
    - 自动回显接收到的数据 - 需要外部发送数据配合

    - **状态查询测试 **(`test_uart_queue_status`) - 测试队列状态查询功能 -
    验证发送 / 接收计数器

               ##使用方法

               ## #方法 1 : 运行所有测试

                            在 `main.c` 中调用：

```c
#include "all_tests.h"

                            int main(void) {
  // ... HAL 初始化代码 ...

  // 运行所有测试
  run_all_tests();

  while (1) {
    // 主循环
  }
}
```

    ## #方法 2 : 单独运行某个测试

                     在 `main.c` 中调用：

```c
#include "uart_hal_test.h"
#include "uart_queue_test.h"

                 int main(void) {
  // ... HAL 初始化代码 ...

  // 只运行 UART HAL 测试
  uart_hal_test_run();

  // 或只运行 UART Queue 测试
  uart_queue_test_run();

  while (1) {
    // 主循环
  }
}
```

    ##硬件要求

    - STM32H743 开发板 - USART1 配置为调试串口(USART_DEBUG) - 串口工具（如 PuTTY
    ,
    Tera Term 等） - 波特率：115200（或根据实际配置）

            ##测试输出

            所有测试都会通过 USART_DEBUG 输出测试结果，示例输出：

``` == ==
        == == == UART HAL Test Start == == == == ==
        UART HAL Sync Transmit Test Sync transmit
    : OK UART HAL Async Transmit Test Async transmit
    : OK
      == ==
      == == == UART HAL Test End == == == == ==

      == == == == == UART Queue Test Start == == == == ==
      UART Queue Test : Message 1 UART Queue Test
    : Message 2 Batch message #1 Batch message #2 ... TX Queue remaining
      : 0 bytes
      == ==
      == == == UART Queue Test End == == == ==
      ==
```

              ##注意事项

              1. *
              *接收测试默认被注释 *
                  *：因为接收测试需要外部发送数据配合，默认情况下被注释掉。如需测试接收功能，请取消相关测试的注释。

              2. *
              *缓冲区大小 **：测试使用
              256 字节的发送和接收缓冲区，可根据需要调整 `TX_BUFFER_SIZE` 和 `RX_BUFFER_SIZE`。

              3. *
              *超时设置 **：接收测试默认等待 5 秒，可根据需要调整超时时间。

              4. *
              *中断配置 **：确保 USART 中断已正确配置并启用。

              5. *
              *DMA 配置 **：如果使用 DMA 模式，确保 DMA 通道已正确配置。

              ##故障排除

              ## #问题 1 : 没有输出
          - 检查 USART 硬件连接 - 检查波特率设置 -
          确认 USART 时钟已使能

          ## #问题 2 : 异步测试失败
                       -
                       检查中断是否正确配置 - 确认回调函数被正确调用 -
                       检查 NVIC 优先级设置

                       ## #问题 3
    : 队列溢出
      -
      增加缓冲区大小 - 检查发送速度是否过快 -
      确认硬件发送正常工作

      ##扩展测试

      可以根据需要添加更多测试用例：

      -
      错误处理测试 - 性能压力测试 - 多 USART 并发测试 - 不同波特率测试 -
      数据完整性测试

      ##作者

      -
      创建日期：2025 - 12 - 03 - 作者：12114
