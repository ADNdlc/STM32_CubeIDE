/*
 * service_factory.c
 *
 *  Created on: Mar 17, 2026
 *      Author: 12114
 */

#include "esp_8266/core/at_controller.h"
#include "gpio_factory.h"
#include "usart_factory.h"


// 静态实例管理
static at_controller_t at_ctrl;
static bool initialized = false;

// 模拟 UART 队列缓冲区 (实际生产中可能由 dev_map 或专有逻辑提供)
static uint8_t at_tx_buf[512];
static uint8_t at_rx_buf[512];
static uart_queue_t at_uart_q;

void service_factory_init(void) {
  if (!initialized) {
    // 1. 获取底层硬件资源 (根据 dev_map.h)
    usart_driver_t *u_drv = usart_driver_get(USART_ID_ESP8266);
    gpio_driver_t *rst_pin = gpio_driver_get(ESP_RST);

    if (u_drv && rst_pin) {
      // 2. 初始化 AT 控制器
      uart_queue_init(&at_uart_q, u_drv, at_tx_buf, sizeof(at_tx_buf),
                      at_rx_buf, sizeof(at_rx_buf));
      at_controller_init(&at_ctrl, &at_uart_q, rst_pin);
      uart_queue_start_receive(&at_uart_q);

      initialized = true;
    }
  }
}

// 供其他 Factory 使用 AT 控制器 (Internal)
at_controller_t *service_factory_get_at_controller(void) {
  service_factory_init();
  return &at_ctrl;
}

void service_factory_process(void) {
  if (initialized) {
    at_controller_process(&at_ctrl);
  }
}
