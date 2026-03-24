/*
 * at_dispatcher.c
 *
 *  Created on: Aug 11, 2025
 *      Author: 12114
 */

#define LOG_TAG "at_dispatcher"
#include "elog.h"
#include "sys.h"
#include "at_controller.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief 将接收到的行分发到相应的处理程序
 * @return true 如果已经处理（匹配到 URC），false 如果未处理
 */
bool at_controller_dispatch(at_controller_t *self, const char *line) {
  if (!self || !line)
    return false;

  // 遍历注册的处理程序列表
  at_urc_node_t *current = self->urc_list;
  while (current != NULL) {
    if (strncmp(line, current->prefix, strlen(current->prefix)) ==
        0) { // 匹配处理项标识和输入行内容
      // 匹配成功，调用相应处理程序
      log_v("Match URC [%s] -> %s", current->prefix, line);
      if (current->handler) {
        current->handler(current->ctx, line); // 使用存储的上下文
      }
      return true;
    }
    current = current->next;
  }

  return false;
}

/**
 * @brief 注册URC处理，prefix要使用字面量或静态字符串
 * @param self    AT控制器实例
 * @param prefix  处理项标识字符串
 * @param handler 处理函数指针
 *
 */
int at_controller_register_handler(at_controller_t *self, const char *prefix,
                                   at_handler_func_t handler, void *ctx) {
  if (!self || !prefix || !handler)
    return -1;

  // Allocate new node
  at_urc_node_t *node =
      (at_urc_node_t *)sys_malloc(SYS_MEM_INTERNAL, sizeof(at_urc_node_t));
  if (!node) {
    log_e("URC registry: memory allocation failed");
    return -1;
  }

  node->prefix = prefix;
  node->handler = handler;
  node->ctx = ctx;
  node->next = NULL;

  // Prepend to list
  node->next = self->urc_list;
  self->urc_list = node;

  return 0;
}
