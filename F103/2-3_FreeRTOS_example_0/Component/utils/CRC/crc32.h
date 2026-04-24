/*
 * crc32.h
 *
 *  Created on: Feb 19, 2026
 *      Author: Antigravity
 */

#ifndef COMPONENT_UTILS_CRC32_H_
#define COMPONENT_UTILS_CRC32_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief 初始化 CRC32 表 (动态构建)
 */
void utils_crc32_init(void);

/**
 * @brief 计算 CRC32 校验码
 * @param data 数据指针
 * @param length 数据长度
 * @param previous_crc 上一次计算的 CRC (初始通常传入 0)
 * @return 计算得到的 CRC32 结果
 */
uint32_t utils_crc32_calc(const uint8_t *data, size_t length,
                          uint32_t previous_crc);

#endif /* COMPONENT_UTILS_CRC32_H_ */
