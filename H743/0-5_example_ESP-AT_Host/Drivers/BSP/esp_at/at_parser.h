/*
 * esp_parser.h
 *
 *  Created on: Aug 10, 2025
 *      Author: 12114
 */

#ifndef BSP_ESP_AT_AT_PARSER_H_
#define BSP_ESP_AT_AT_PARSER_H_

#include <stddef.h>
#include <stdint.h>

// 定义解析器行缓冲区的最大大小
// 应该大于任何可能收到的最长的一行AT响应
#define LINE_BUF_SIZE  256

// 初始化AT解析器
void AT_parser_init(void);

/**
 * @brief 向解析器输入新的数据
 * @param data 从驱动层读取到的数据指针
 * @param len 数据长度
 */
void AT_parser_input(const uint8_t* data, size_t len);



#endif /* BSP_ESP_AT_AT_PARSER_H_ */
