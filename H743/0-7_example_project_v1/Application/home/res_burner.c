#include "res_burner.h"
#include "block_device.h"
#include "elog.h"
#include "factory/flash_factory.h"
#include "res_manager.h"
#include <string.h>
#include "lvgl.h"

#define LOG_TAG "RES_BURNER"

// Include LVGL assets
LV_IMG_DECLARE(wallpaper);
LV_IMG_DECLARE(icon_wifi);
LV_IMG_DECLARE(icon_bright);
LV_IMG_DECLARE(icon_colorwheel);
LV_IMG_DECLARE(icon_Contol);

typedef struct {
  res_id_t id;
  uint32_t offset;
  const lv_img_dsc_t *dsc;
  const char *name;
} burn_item_t;

// Note: Offsets must match res_manager.h
static const burn_item_t burn_list[] = {
    {RES_IMG_WALLPAPER, RES_OFFSET_WALLPAPER, &wallpaper, "Wallpaper"},
    {RES_IMG_ICON_WIFI, RES_OFFSET_ICON_WIFI, &icon_wifi, "WiFi Icon"},
    {RES_IMG_ICON_BRIGHT, RES_OFFSET_ICON_BRIGHT, &icon_bright,
     "Brightness Icon"},
    {RES_IMG_ICON_COLORWHEEL, RES_OFFSET_ICON_COLORWHEEL, &icon_colorwheel,
     "Colorwheel Icon"},
    {RES_IMG_ICON_CONTROL, RES_OFFSET_ICON_CONTROL, &icon_Contol,
     "Control Icon"},
};

static uint32_t align_up(uint32_t val, uint32_t align) {
  return (val + align - 1) & ~(align - 1);
}

void res_burner_run(void) {
  log_i("Resource Burner: Starting (Raw Mode)...");

  // 1. 获取 QSPI 设备
  block_device_t *dev = flash_factory_get(FLASH_EXT_QSPI);
  if (!dev) {
    log_e("Failed to get QSPI device!");
    return;
  }

  // 2. 获取设备信息以进行对齐擦除
  block_dev_info_t info;
  if (BLOCK_DEV_GET_INFO(dev, &info) != 0) {
    log_e("Failed to get device info");
    return;
  }
  log_i("Flash Info: Sector Size: %d, Page Size: %d", info.sector_size,
        info.page_size);

  // 3. 遍历烧录
  for (size_t i = 0; i < sizeof(burn_list) / sizeof(burn_list[0]); i++) {
    const burn_item_t *item = &burn_list[i];

    // 计算物理地址
    uint32_t phys_addr = RES_PARTITION_OFFSET + item->offset;
    uint32_t data_size = item->dsc->data_size;
    uint32_t total_size = 4 + data_size; // LVGL Header (4 bytes) + Data

    log_i("Burning %s to 0x%X (Size: %d)...", item->name, phys_addr,
          total_size);

    // A. 擦除区域
    // 注意：如果是连续写入，优化策略应该是先擦除整个 Partition 区域。
    // 这里为了安全起见，对每个资源进行按需擦除 (可能会有 Sector 重叠，需小心)
    // 假设资源偏移量已经按照 Sector 对齐设计，或者资源之间有足够间隔。
    // 为简单起见，这里假设每个资源独占 Sector，或调用者已规划好。
    // *重要*：如果两个小资源在同一个 Sector，单独擦除第二个会擦掉第一个。
    // 鉴于 res_manager.h 中的偏移量间隔较大 (50000 -> 320KB,
    // 足够包含普通图标)， 我们在此简单执行擦除。

    // 计算擦除起始和大小 (Sector 对齐)
    uint32_t erase_addr =
        (phys_addr / info.block_size) *
        info.block_size; // 假设 block_size 是擦除粒度(通常4K或64K)
    // 注意：w25q_adapter 实现中 block_size 可能是 64K，sector_size 可能是
    // 4K。需确认 block_device 定义。 这里的 info.block_size
    // 命名可能引起歧义，复查 block_device.h: block_size is "Erase Block Size".

    uint32_t erase_end = align_up(phys_addr + total_size, info.block_size);
    uint32_t erase_len = erase_end - erase_addr;

    log_d("  Erasing from 0x%X, len: %d", erase_addr, erase_len);
    if (BLOCK_DEV_ERASE(dev, erase_addr, erase_len) != 0) {
      log_e("  Erase failed!");
      continue;
    }

    // B. 写入 Header
    if (BLOCK_DEV_PROGRAM(dev, phys_addr, (const uint8_t *)&item->dsc->header,
                          4) != 0) {
      log_e("  Write header failed!");
      continue;
    }

    // C. 写入数据
    if (BLOCK_DEV_PROGRAM(dev, phys_addr + 4, item->dsc->data, data_size) !=
        0) {
      log_e("  Write data failed!");
      continue;
    }

    log_i("  Success.");
  }

  log_i("Resource Burner: All resources written.");
}
