/*
 * at_dispatcher.h
 *
 *  Created on: Aug 11, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_AT_DISPATCHER_H_
#define BSP_ESP_AT_AT_DISPATCHER_H_

/**
 * @brief 处理由解析器传来的一行完整的AT消息
 *        它会查找匹配的处理程序并调用它。
 * @param line 一个指向以NULL结尾的字符串的指针，代表一行消息。
 */
void at_dispatcher_process_line(const char* line);


#endif /* BSP_ESP_AT_AT_DISPATCHER_H_ */
