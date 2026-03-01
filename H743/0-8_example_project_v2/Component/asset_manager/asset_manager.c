/*
 * asset_manager.c
 *
 *  Created on: Feb 26, 2026
 *      Author: Antigravity
 */
#define LOG_TAG "ASSET_MGR"

#include "asset_manager.h"
#include "MemPool.h"
#include "Sys.h"
#include "elog.h"
#include "nor_flash_factory.h"
#include "utils/CRC/crc32.h"
#include <string.h>

// TOC 默认分布在一二号扇区 (4KB)
#define ASSET_TOC_A_ADDR 0x00000000
#define ASSET_TOC_B_ADDR 0x00001000

// 数据区分为 A/B 两个区块 (W25Q128 为 16MB，我们按 8MB 为界限平分)
// Data A 使用从 0x00002000 到 0x007FFFFF
// Data B 使用从 0x00800000 到 0x00FFFFFF
#define ASSET_DATA_A_ADDR 0x00002000
#define ASSET_DATA_B_ADDR 0x00800000

// XIP 在 H7 上的映射起始地址 (由 QSPI 硬件或 MPU 决定，H7 默认为 0x90000000)
#define XIP_BASE_ADDRESS 0x90000000

static nor_flash_driver_t *asset_flash = NULL;
// 0xFFFFFFFF 代表尚未加载任何有效表
static uint32_t active_toc_addr = 0xFFFFFFFF;
static res_header_t active_header = {0};
static res_item_t *active_items = NULL; // 动态分配的 TOC 内存缓存

/**
 * @brief 计算头文件的 TOC CRC，排除 toc_crc 字段本身
 */
static uint32_t calc_toc_crc(const res_header_t *header,
                             const res_item_t *items) {
  uint32_t crc = utils_crc32_calc((const uint8_t *)header,
                                  offsetof(res_header_t, toc_crc), 0);
  // 加上后半部分 reserved
  uint32_t reserved_offset = offsetof(res_header_t, reserved);
  uint32_t reserved_len = sizeof(res_header_t) - reserved_offset;
  crc = utils_crc32_calc(((const uint8_t *)header) + reserved_offset,
                         reserved_len, ~crc);

  // 加上所有 items
  if (header->item_count > 0 && items != NULL) {
    crc = utils_crc32_calc((const uint8_t *)items,
                           header->item_count * sizeof(res_item_t), ~crc);
  }
  return crc;
}

/**
 * @brief 校验单个 TOC 的完整性
 */
static bool validate_toc(uint32_t toc_addr, res_header_t *header_out,
                         res_item_t **items_out) {
  res_header_t hdr;
  *items_out = NULL;

  if (NOR_FLASH_READ(asset_flash, toc_addr, (uint8_t *)&hdr,
                     sizeof(res_header_t)) != 0) {
    return false;
  }

  if (hdr.magic != ASSET_MAGIC) {
    log_d("TOC at 0x%08X: Magic incorrect (0x%08X)", toc_addr, hdr.magic);
    return false;
  }

  uint32_t max_items =
      (ASSET_TOC_SIZE - sizeof(res_header_t)) / sizeof(res_item_t);
  if (hdr.item_count > max_items) {
    log_e("TOC at 0x%08X: Item count (%d) out of bounds", toc_addr,
          hdr.item_count);
    return false;
  }

  res_item_t *items = NULL;
  if (hdr.item_count > 0) {
    items = (res_item_t *)sys_malloc(SYS_MEM_INTERNAL,
                                     hdr.item_count * sizeof(res_item_t));
    if (!items) {
      log_e("TOC at 0x%08X: OOM allocating items", toc_addr);
      return false;
    }

    if (NOR_FLASH_READ(asset_flash, toc_addr + sizeof(res_header_t),
                       (uint8_t *)items,
                       hdr.item_count * sizeof(res_item_t)) != 0) {
      sys_free(SYS_MEM_INTERNAL, items);
      return false;
    }
  }

  // 校验 CRC
  uint32_t calc_crc = calc_toc_crc(&hdr, items);
  if (calc_crc != hdr.toc_crc) {
    log_e("TOC at 0x%08X: CRC mismatch! Expected: 0x%08X, Calc: 0x%08X",
          toc_addr, hdr.toc_crc, calc_crc);
    if (items)
      sys_free(SYS_MEM_INTERNAL, items);
    return false;
  }

  *header_out = hdr;
  *items_out = items;
  return true;
}

int asset_manager_init(void) {
  if (asset_flash != NULL)
    return 0; // 已经初始化过了

  utils_crc32_init();

  asset_flash = nor_flash_factory_creat(NOR_FLASH_SYS);
  if (!asset_flash) {
    log_e("Failed to acquire NOR Flash driver via factory");
    return -1;
  }

  // 一定要先切回命令模式，以防软重启后仍处于 XIP 模式
  NOR_FLASH_SET_MODE(asset_flash, NOR_FLASH_MODE_COMMAND);

  res_header_t header_a, header_b;
  res_item_t *items_a = NULL, *items_b = NULL;

  bool a_valid = validate_toc(ASSET_TOC_A_ADDR, &header_a, &items_a);
  bool b_valid = validate_toc(ASSET_TOC_B_ADDR, &header_b, &items_b);

  log_i("TOC A valid: %d, TOC B valid: %d", a_valid, b_valid);

  // 双表裁决逻辑
  if (a_valid && b_valid) {
    // 判断谁较新 (利用无符号回绕安全比较：a - b < 0x80000000 意味着 a > b)
    if (header_a.sequence_num > header_b.sequence_num) {
      active_toc_addr = ASSET_TOC_A_ADDR;
      active_header = header_a;
      active_items = items_a;
      if (items_b)
        sys_free(SYS_MEM_INTERNAL, items_b);
    } else {
      active_toc_addr = ASSET_TOC_B_ADDR;
      active_header = header_b;
      active_items = items_b;
      if (items_a)
        sys_free(SYS_MEM_INTERNAL, items_a);
    }
  } else if (a_valid) {
    active_toc_addr = ASSET_TOC_A_ADDR;
    active_header = header_a;
    active_items = items_a;
  } else if (b_valid) {
    active_toc_addr = ASSET_TOC_B_ADDR;
    active_header = header_b;
    active_items = items_b;
  } else {
    log_w("No valid TOC found. Asset Manager requires formatting/update.");
    // 这里只是抛出警告，并不返回错误，因为新板子肯定没有内容
    // 我们等待 update 的调用
    return 0;
  }

  log_i("Asset manager bound to TOC at 0x%08X (Seq: %d, Items: %d)",
        active_toc_addr, active_header.sequence_num, active_header.item_count);

  // 成功加载 TOC 后，将 Flash 挂载到 XIP 模式，供整个系统使用
  if (NOR_FLASH_SET_MODE(asset_flash, NOR_FLASH_MODE_XIP) != 0) {
    log_e("Failed to mount Flash to XIP mode!");
    return -1;
  }

  log_i("Asset manager XIP mode activated.");
  return 0;
}

const void *asset_manager_get_res(uint32_t id, uint32_t *size) {
  if (active_items == NULL || active_header.item_count == 0 ||
      asset_flash == NULL) {
    return NULL;
  }

  for (uint32_t i = 0; i < active_header.item_count; i++) {
    if (active_items[i].id == id) {
      if (size)
        *size = active_items[i].size;
      return (const void *)(uintptr_t)(XIP_BASE_ADDRESS +
                                       active_header.data_offset +
                                       active_items[i].offset);
    }
  }
  return NULL;
}

// ---------------- 更新逻辑 -------------------

typedef struct {
  bool is_updating;
  uint32_t target_toc_addr;
  res_header_t new_header;
  res_item_t *new_items;
  uint32_t current_item_idx;
  uint32_t current_data_offset; // 相对于 ASSET_DATA_ADDR
  uint32_t erased_data_offset;  // 已擦除的边界
} update_ctx_t;

static update_ctx_t update_ctx = {0};

int asset_manager_begin_update(uint32_t item_count) {
  if (asset_flash == NULL || update_ctx.is_updating)
    return -1;
  if (item_count == 0 ||
      item_count > (ASSET_TOC_SIZE - sizeof(res_header_t)) / sizeof(res_item_t))
    return -2;

  // 1. 退出 XIP，进入命令模式
  if (NOR_FLASH_SET_MODE(asset_flash, NOR_FLASH_MODE_COMMAND) != 0)
    return -3;

  update_ctx.is_updating = true;
  update_ctx.target_toc_addr = (active_toc_addr == ASSET_TOC_A_ADDR)
                                   ? ASSET_TOC_B_ADDR
                                   : ASSET_TOC_A_ADDR;
  if (active_toc_addr == 0xFFFFFFFF)
    update_ctx.target_toc_addr = ASSET_TOC_A_ADDR; // Initial case

  // 2. 擦除目标 TOC 扇区
  nor_flash_info_t info;
  NOR_FLASH_GET_INFO(asset_flash, &info);
  if (NOR_FLASH_ERASE_SECTOR(asset_flash, update_ctx.target_toc_addr) != 0)
    goto error;

  // 3. 准备 Header
  memset(&update_ctx.new_header, 0, sizeof(res_header_t));
  update_ctx.new_header.magic = ASSET_MAGIC;
  update_ctx.new_header.sequence_num = active_header.sequence_num + 1;
  update_ctx.new_header.item_count = item_count;

  // A/B 分区数据隔离
  if (update_ctx.target_toc_addr == ASSET_TOC_A_ADDR) {
    update_ctx.new_header.data_offset = ASSET_DATA_A_ADDR;
  } else {
    update_ctx.new_header.data_offset = ASSET_DATA_B_ADDR;
  }

  // 4. 分配新 Items 内存
  update_ctx.new_items = (res_item_t *)sys_malloc(
      SYS_MEM_INTERNAL, item_count * sizeof(res_item_t));
  if (!update_ctx.new_items)
    goto error;
  memset(update_ctx.new_items, 0, item_count * sizeof(res_item_t));

  update_ctx.current_item_idx = 0;
  update_ctx.current_data_offset = 0;
  update_ctx.erased_data_offset = 0;

  log_i("Update started. Target TOC: 0x%08X, Seq: %d, Count: %d",
        update_ctx.target_toc_addr, update_ctx.new_header.sequence_num,
        item_count);
  return 0;

error:
  update_ctx.is_updating = false;
  NOR_FLASH_SET_MODE(asset_flash, NOR_FLASH_MODE_XIP); // 尝试恢复
  return -4;
}

int asset_manager_write_res(uint32_t id, uint32_t type, const uint8_t *data,
                            uint32_t size) {
  if (!update_ctx.is_updating ||
      update_ctx.current_item_idx >= update_ctx.new_header.item_count)
    return -1;
  if (data == NULL || size == 0)
    return -2;

  nor_flash_info_t info;
  NOR_FLASH_GET_INFO(asset_flash, &info);

  uint32_t abs_write_addr =
      update_ctx.new_header.data_offset + update_ctx.current_data_offset;

  // 按需擦除目标数据区
  while (update_ctx.current_data_offset + size >
         update_ctx.erased_data_offset) {
    uint32_t erase_addr =
        update_ctx.new_header.data_offset + update_ctx.erased_data_offset;
    if (NOR_FLASH_ERASE_SECTOR(asset_flash, erase_addr) != 0)
      return -3;
    update_ctx.erased_data_offset += info.sector_size;
  }

  // 写入数据
  if (NOR_FLASH_WRITE(asset_flash, abs_write_addr, data, size) != 0)
    return -4;

  // 记录 Item
  res_item_t *item = &update_ctx.new_items[update_ctx.current_item_idx];
  item->id = id;
  item->offset = update_ctx.current_data_offset;
  item->size = size;
  item->type = type;
  item->item_crc = utils_crc32_calc(data, size, 0);

  log_d("Wrote Resource ID: 0x%08X, Offset: 0x%08X, Size: %d", id, item->offset,
        size);

  update_ctx.current_data_offset += size;
  update_ctx.current_item_idx++;
  return 0;
}

int asset_manager_end_update(void) {
  if (!update_ctx.is_updating)
    return -1;

  // 如果未写满预期数量，视为失败
  if (update_ctx.current_item_idx != update_ctx.new_header.item_count) {
    log_e("Update incomplete! Expected %d items, got %d",
          update_ctx.new_header.item_count, update_ctx.current_item_idx);
    sys_free(SYS_MEM_INTERNAL, update_ctx.new_items);
    update_ctx.is_updating = false;
    NOR_FLASH_SET_MODE(asset_flash, NOR_FLASH_MODE_XIP);
    return -2;
  }

  // 计算 TOC CRC
  update_ctx.new_header.toc_crc =
      calc_toc_crc(&update_ctx.new_header, update_ctx.new_items);

  // 提交(写入) Header 和 Items
  if (NOR_FLASH_WRITE(asset_flash, update_ctx.target_toc_addr,
                      (uint8_t *)&update_ctx.new_header,
                      sizeof(res_header_t)) != 0) {
    goto error;
  }

  if (NOR_FLASH_WRITE(
          asset_flash, update_ctx.target_toc_addr + sizeof(res_header_t),
          (uint8_t *)update_ctx.new_items,
          update_ctx.new_header.item_count * sizeof(res_item_t)) != 0) {
    goto error;
  }

  // 更新成功，应用到运行时状态
  log_i("Update committed successfully to TOC at 0x%08X",
        update_ctx.target_toc_addr);
  active_toc_addr = update_ctx.target_toc_addr;
  active_header = update_ctx.new_header;
  if (active_items)
    sys_free(SYS_MEM_INTERNAL, active_items);
  active_items = update_ctx.new_items;

  update_ctx.is_updating = false;
  NOR_FLASH_SET_MODE(asset_flash, NOR_FLASH_MODE_XIP);
  return 0;

error:
  log_e("Failed to commit TOC to Flash!");
  sys_free(SYS_MEM_INTERNAL, update_ctx.new_items);
  update_ctx.is_updating = false;
  NOR_FLASH_SET_MODE(asset_flash, NOR_FLASH_MODE_XIP);
  return -3;
}

// 仅用于测试/模拟掉电重启
int asset_manager_force_reset_state(void) {
  if (update_ctx.is_updating) {
    update_ctx.is_updating = false;
    if (update_ctx.new_items) {
      sys_free(SYS_MEM_INTERNAL, update_ctx.new_items);
      update_ctx.new_items = NULL;
    }
  }
  if (active_items) {
    sys_free(SYS_MEM_INTERNAL, active_items);
    active_items = NULL;
  }
  memset(&active_header, 0, sizeof(res_header_t));
  active_toc_addr = 0xFFFFFFFF;
  if (asset_flash) {
    uint8_t d;
    NOR_FLASH_READ(asset_flash, 0, &d, 1);
  }
  asset_flash = NULL;
  return 0;
}
