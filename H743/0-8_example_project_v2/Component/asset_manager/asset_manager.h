#ifndef COMPONENT_ASSET_MANAGER_H_
#define COMPONENT_ASSET_MANAGER_H_

#include <stdbool.h>
#include <stdint.h>

#define ASSET_MAGIC 0x30545341 // "AST0"
#define ASSET_TOC_SIZE 4096    // TOC 表大小

// 资源类型定义
typedef enum {
  ASSET_TYPE_RAW = 0,   // 二进制
  ASSET_TYPE_IMAGE = 1, // LVGL Image
  ASSET_TYPE_FONT = 2,  // LVGL Font
  ASSET_TYPE_AUDIO = 3, // Audio
} asset_type_t;

#pragma pack(push, 1)

// 多态元数据联合体 (4 字节)
typedef union {
  uint32_t raw;
  uint32_t img_header; // 存储 lv_img_header_t (宽高/格式)
  struct {
    uint16_t line_height;
    uint16_t base_line;
  } font; // 存储字体基础参数
  struct {
    uint16_t sample_rate;
    uint8_t channels;
    uint8_t bit_depth;
  } audio; // 存储音频参数
} asset_meta_t;

// 资源项描述符 (32 bytes 固定对齐)
typedef struct {
  uint32_t id;         // 唯一ID
  uint32_t offset;     // 偏移
  uint32_t size;       // 大小
  uint32_t item_crc;   // 校验和
  uint16_t type;       // asset_type_t
  uint16_t flags;      // 预留标记
  asset_meta_t meta;   // 多态元数据
  uint8_t reserved[8]; // 保留
} res_item_t;

// TOC 表头 (32 bytes)
typedef struct {
  uint32_t magic;        // 魔法值
  uint32_t sequence_num; // 序列号
  uint32_t item_count;   // 资源项数量
  uint32_t data_offset;  // 数据偏移
  uint32_t toc_crc;      // TOC 校验和
  uint32_t reserved[3];  // 保留
} res_header_t;

#pragma pack(pop)

int asset_manager_init(void);
// 获取资源的完整映射信息
const void *asset_manager_get_res_info(uint32_t id, uint16_t *type,
                                       asset_meta_t *meta, uint32_t *size);

int asset_manager_begin_update(uint32_t item_count);
int asset_manager_write_res(uint32_t id, uint16_t type, asset_meta_t meta,
                            const uint8_t *data, uint32_t size);
int asset_manager_end_update(void);

#endif
