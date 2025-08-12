/*
 * esp_parser.c
 *
 *  Created on: Aug 10, 2025
 *      Author: 12114
 */

#include "esp_at/at_parser.h"
#include <string.h>
#include <stdio.h> // for printf, for debug

// --- 私有变量 ---
static uint8_t line_buffer[LINE_BUF_SIZE];	//行缓冲
static size_t current_len = 0;				//已有长度(缓冲区可能剩下的非完整行)

// --- 外部回调函数 ---
// 当解析器找到一个完整的行时，会调用这个函数
// 需要在别处实现此函数
extern void AT_parser_line(const char* line);

/** 初始化清零整个缓冲区
 **/
void AT_parser_init(void) {
    current_len = 0;
    memset(line_buffer, 0, sizeof(line_buffer));
}

/**
 * @brief 向解析器输入新的数据
 * @param data 	数据源地址
 * @param len 	拷贝进来的数据长度
 **/
void AT_parser_input(const uint8_t* data, size_t len) {
    if (len == 0) {
        return;
    }
    // 拷贝或追加前检查是否会溢出(一般此判断不会触发除非接收到超长数据,一直没有检测到结束符而一直追加)
    if (current_len + len > LINE_BUF_SIZE) {
        // 错误处理：缓冲区溢出。可以清空缓冲区或者只追加部分数据。
        // 这里选择清空，防止错误数据累积。
        current_len = 0;
        return;
    }

    // 将新数据追加到行缓冲区的末尾
    memcpy(line_buffer + current_len, data, len);
    current_len += len;

    /* ---------------- 开始处理行缓冲 -----------------*/
#if 1
    size_t start_pos = 0;	//搜索起始位置
    while (start_pos < current_len) {
        // 查找 \n (通常 \r\n 一起出现，以 \n 为准)
        uint8_t* end_index = (uint8_t*)memchr(line_buffer + start_pos, '\n', current_len - start_pos);
        if (end_index) {
            // 找到了一个换行符
            size_t end_pos = end_index - line_buffer;//此行AT响应长度/位置
            // 将换行符替换为 \0，以形成一个标准的C字符串
            line_buffer[end_pos] = '\0';
            if (end_pos > 0 && line_buffer[end_pos - 1] == '\r') {
                 line_buffer[end_pos - 1] = '\0';
            }
            // 提取出这一行 (从start_pos开始)
            char* start_index = (char*)(line_buffer + start_pos);
            // 过滤掉空行
            if (strlen(start_index) > 0) {
                // 将完整的行交给上层处理
            	AT_parser_line(start_index);
            }
            // 计算下一轮搜索的起始位置
            start_pos = end_pos + 1;

        } else {
            break;// 在剩余的数据中没有找到换行符，退出循环
        }
    }
    /* ------- 退出循环后,清理已处理的数据,并将剩下的数据移动到缓冲区开头 ------- */
    if (start_pos > 0) {
        size_t remaining_len = current_len - start_pos;
        // 使用 memmove 而不是 memcpy，因为源和目标区域可能重叠
        memmove(line_buffer, line_buffer + start_pos, remaining_len);
        current_len = remaining_len;
    }
#endif

}

