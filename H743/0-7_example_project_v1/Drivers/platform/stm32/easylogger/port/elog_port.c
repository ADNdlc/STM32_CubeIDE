/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */

#include <elog.h>
#include <stdio.h>
// 依赖
#include "device_mapping.h"
#include "stm32h7xx_hal.h"
#include "uart_queue/uart_queue.h"
#include "usart_factory.h"
#include "usart_hal/usart_hal.h"


// EasyLogger 使用的 UART 队列缓冲区
static uint8_t elog_tx_buffer[2096];
static uint8_t elog_rx_buffer[128]; // 日志通常只发送，接收缓冲区给小点

static usart_hal_t *g_elog_hal = NULL;
static uart_queue_t *g_elog_queue = NULL;

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
  ElogErrCode result = ELOG_NO_ERR;

  usart_driver_t *driver = usart_driver_get(USART_LOGGER);
  if (driver) {
    g_elog_hal = usart_hal_create(driver);
    if (g_elog_hal) {
      g_elog_queue =
          uart_queue_create(g_elog_hal, elog_tx_buffer, sizeof(elog_tx_buffer),
                            elog_rx_buffer, sizeof(elog_rx_buffer));
      if (!g_elog_queue) {
        result = ELOG_ERR;
      }
    } else {
      result = ELOG_ERR;
    }
  } else {
    result = ELOG_ERR;
  }

  return result;
}

/**
 * EasyLogger port deinitialize
 *
 */
void elog_port_deinit(void) {
  if (g_elog_queue) {
    uart_queue_destroy(g_elog_queue);
    g_elog_queue = NULL;
  }
  if (g_elog_hal) {
    usart_hal_destroy(g_elog_hal);
    g_elog_hal = NULL;
  }
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
  if (g_elog_queue) {
    uart_queue_send(g_elog_queue, (const uint8_t *)log, size);
  }
}

/**
 * output lock
 */
void elog_port_output_lock(void) { /* add your code here */ }

/**
 * output unlock
 */
void elog_port_output_unlock(void) { /* add your code here */ }

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
  static char cur_system_time[24] = {0};
  snprintf(cur_system_time, 24, "%lu", HAL_GetTick());
  return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {

  /* add your code here */
  return ""; //当前没有使用系统
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {

  /* add your code here */
  return ""; //当前没有使用系统
}
