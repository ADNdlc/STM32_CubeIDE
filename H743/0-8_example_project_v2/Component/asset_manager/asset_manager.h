/*
 * asset_manager.h
 *
 *  Created on: Feb 26, 2026
 *      Author: Antigravity
 *
 *  基于 Dual TOC 的外部资源管理器 (Flash XIP 访问)
 */

#ifndef COMPONENT_ASSET_MANAGER_H_
#define COMPONENT_ASSET_MANAGER_H_

#include <stdbool.h>
#include <stdint.h>

#define ASSET_MAGIC 0x30545341 // "AST0" (ASCII: A S T 0)
#define ASSET_TOC_SIZE 4096    // 资源表大小 (1 个 Sector, 4KB)

#pragma pack(push, 1)

// 资源项描述符 (32 bytes)
typedef struct {
  uint32_t id;       // 资源唯一 ID
  uint32_t offset;   // 资源相对于 Data 区域起始的物理偏移
  uint32_t size;     // 资源大小 (字节)
  uint32_t item_crc; // 资源数据体的 CRC32
  uint32_t type;     // 资源类型 (如 IMAGE=1, FONT=2)
  uint8_t reserved[12];
} res_item_t;

// 资源表头 TOC Header (32 bytes)
typedef struct {
  uint32_t magic;        // 校验数 'AST0'
  uint32_t sequence_num; // 序列号，双表择优加载依据
  uint32_t item_count;   // 资源项总数
  uint32_t data_offset;  // 数据区起始物理地址 (绝对地址)
  uint32_t toc_crc;      // 本 Header(不含toc_crc) + 所有 Item 的 CRC32
  uint32_t reserved[3];
} res_header_t;

#pragma pack(pop)

/**
 * @brief 初始化资产管理器并加载双表 (Dual TOC)
 * @return 0 成功，-1 失败 (两个 TOC 均损坏或未格式化)
 */
int asset_manager_init(void);

/**
 * @brief 获取资源的内存映射指针 (XIP 直读)
 * @param id 资源唯一 ID
 * @param size [out] 传出资源大小
 * @return void* 资源的直接内部映射指针 (如果失败返回 NULL)
 */
const void *asset_manager_get_res(uint32_t id, uint32_t *size);

/**
 * @brief 开始更新资源
 * @param item_count 预计更新的资源总数
 * @return 0 成功，< 0 失败
 */
int asset_manager_begin_update(uint32_t item_count);

/**
 * @brief 写入单个资源数据 (自动追加到新 TOC 中，并写入 Flash 数据区)
 * @param id 资源唯一 ID
 * @param type 资源类型
 * @param data 资源数据指针
 * @param size 资源大小
 * @return 0 成功，< 0 失败
 */
int asset_manager_write_res(uint32_t id, uint32_t type, const uint8_t *data,
                            uint32_t size);

/**
 * @brief 完成更新，提交新 TOC 并切换回 XIP 模式
 * @return 0 成功，< 0 失败
 */
int asset_manager_end_update(void);

/**
 * @brief 软重置状态 (仅用于模拟掉电测试)
 */
int asset_manager_force_reset_state(void);

#endif /* COMPONENT_ASSET_MANAGER_H_ */
