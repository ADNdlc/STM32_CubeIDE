#include <elog.h>
#include <stdio.h>
// 依赖
#include "BSP_init.h"
#include "Sys.h"
#include "shell.h"
#include "shell_port.h"
#include "shell_cfg_user.h"

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
  // 注意：g_debug_queue 在 bsp_init 中已经初始化好了，这里直接检测
  if (g_debug_queue == NULL) {
    return ELOG_ERR;
  }
  return ELOG_NO_ERR;
}

/**
 * EasyLogger port deinitialize
 */
void elog_port_deinit(void) {
  // 全局队列由 bsp 释放（如有必要），port 层不再销毁它
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
#if defined(SHELL_SUPPORT_END_LINE) && SHELL_SUPPORT_END_LINE == 1
  // 增加保护：仅在 shell 已初始化（write接口有效）时使用尾行模式
  if (shell.write != NULL) {
    shellWriteEndLine(&shell, (char *)log, size);
    return;
  }
#endif
  // 如果 shell 未初始化，或者未开启尾行模式，回退到原始 UART 队列输出
  if (g_debug_queue) {
    uart_queue_send(g_debug_queue, (const uint8_t *)log, size);
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
  snprintf(cur_system_time, 24, "%lu", sys_get_systick_ms());
  return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) { return ""; }

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) { return ""; }
