/*
 * esp_parser.c
 *
 *  Created on: Aug 10, 2025
 *      Author: 12114
 */

#include "esp_at/at_parser.h"
#include <string.h>
#include <stdio.h>
#if USE_MY_MALLOC
#include "malloc/malloc.h"
#endif

// --- 私有变量 ---
#if !USE_MY_MALLOC
static uint8_t line_buffer[LINE_BUF_SIZE];	//行缓冲
#else
static uint8_t* line_buffer = NULL;
#endif
static size_t current_len = 0;	//已有长度(缓冲区可能剩下的非完整行)

/* ==================== 外部回调函数 ==================== */
// 当解析器找到一个完整的行时，会调用这个函数
// 需要在别处实现此函数
extern void AT_parser_line(const char* line);

/* 初始化清零整个行缓冲区
 */
void AT_parser_init(void) {
#if USE_MY_MALLOC
	if(!line_buffer){ //还没有申请缓冲区
		line_buffer = mymalloc(SRAMDTCM, LINE_BUF_SIZE);
	}
#endif
    current_len = 0;
    memset(line_buffer, 0, LINE_BUF_SIZE);
}

/**
 * @brief 向解析器输入新的数据
 * @param data 	外部数据源地址
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
    size_t start_pos = 0;	//搜索起始位置
    while (start_pos < current_len) {
    	uint8_t* prompt_pos  = (uint8_t*)memchr(line_buffer + start_pos, '>', current_len - start_pos);
		uint8_t* newline_pos = (uint8_t*)memchr(line_buffer + start_pos, '\n', current_len - start_pos);
		uint8_t* first_token_pos = NULL;

		// 找出哪个分隔符先出现
		if (newline_pos && prompt_pos) {
			first_token_pos = (newline_pos < prompt_pos) ? newline_pos : prompt_pos;
		} else if (newline_pos) {
			first_token_pos = newline_pos;
		} else if (prompt_pos) {
			first_token_pos = prompt_pos;
		}

		if (first_token_pos) {
			// 找到了一个消息单元
			char* line_to_process = (char*)(line_buffer + start_pos);
			size_t token_end_index = first_token_pos - line_buffer;

			if (*first_token_pos == '\n') {
				// --- 情况1: 这是一个以换行符结尾的行 ---
				line_buffer[token_end_index] = '\0';
				if (token_end_index > 0 && line_buffer[token_end_index - 1] == '\r') {
					line_buffer[token_end_index - 1] = '\0';
				}

				if (strlen(line_to_process) > 0) {
					AT_parser_line(line_to_process);
				}

				start_pos = token_end_index + 1; // 移动到下一轮搜索的起点

			} else {
				// 情况2: 这是一个单独的 '>' 提示符(没有换行符)
				// 需要把它作为一个独立的行来处理
				if (first_token_pos > (line_buffer + start_pos)) {
					*first_token_pos = '\0';
					AT_parser_line(line_to_process);
					*first_token_pos = '>';
				}
				AT_parser_line(">");
				start_pos = token_end_index + 1; // 移动到下一轮搜索的起点
			}
		} else {
			break;// 在剩余的数据中没有找到任何消息单元，退出循环
		}
    }
    /* ------- 退出循环后,清理已处理的数据,并将剩下的数据移动到缓冲区开头 ------- */
    if (start_pos > 0) {
        size_t remaining_len = current_len - start_pos;
        // 使用 memmove 而不是 memcpy，因为源和目标区域可能重叠
        memmove(line_buffer, line_buffer + start_pos, remaining_len);
        current_len = remaining_len;
    }
}

