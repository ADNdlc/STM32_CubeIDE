#ifndef COMPONENT_ASSET_MANAGER_H_
#define COMPONENT_ASSET_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

#define ASSET_MAGIC 0x30545341 // "AST0"
#define ASSET_TOC_SIZE 4096

// 资源类型定义
typedef enum {
    ASSET_TYPE_RAW   = 0,
    ASSET_TYPE_IMAGE = 1, // LVGL Image
    ASSET_TYPE_FONT  = 2, // LVGL Font
    ASSET_TYPE_AUDIO = 3, // PCM/MP3 Audio
} asset_type_t;

#pragma pack(push, 1)

// 多态元数据联合体 (4 字节)
typedef union {
    uint32_t raw;
    uint32_t img_header;     // 存储 lv_img_header_t (宽高/格式)
    struct {
        uint16_t line_height;
        uint16_t base_line;
    } font;                  // 存储字体基础参数
    struct {
        uint16_t sample_rate;
        uint8_t  channels;
        uint8_t  bit_depth;
    } audio;                 // 存储音频参数
} asset_meta_t;

// 资源项描述符 (32 bytes 固定对齐)
typedef struct {
  uint32_t id;
  uint32_t offset;
  uint32_t size;
  uint32_t item_crc;
  uint16_t type;      // asset_type_t
  uint16_t flags;     // 预留标记
  asset_meta_t meta;  // 多态元数据
  uint8_t reserved[8];
} res_item_t;

// TOC Header (32 bytes)
typedef struct {
  uint32_t magic;
  uint32_t sequence_num;
  uint32_t item_count;
  uint32_t data_offset;
  uint32_t toc_crc;
  uint32_t reserved[3];
} res_header_t;

#pragma pack(pop)

int asset_manager_init(void);
// 获取资源的完整映射信息
const void *asset_manager_get_res_info(uint32_t id, uint16_t *type, asset_meta_t *meta, uint32_t *size);

int asset_manager_begin_update(uint32_t item_count);
int asset_manager_write_res(uint32_t id, uint16_t type, asset_meta_t meta, const uint8_t *data, uint32_t size);
int asset_manager_end_update(void);

#endif
