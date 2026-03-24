#include "all_tests_config.h"
#if _sdcard_test_
#include "elog.h"
#include "factory/flash_factory.h"
#include "sys.h"
#include <string.h>

#define LOG_TAG "SDTest"

void sdcard_test_run(void) {
  log_i("Starting SD Card Test...");

  // 1. 获取 SD 卡设备
  block_device_t *sd_dev = flash_factory_get(FLASH_EXT_SDCARD);
  if (!sd_dev) {
    log_e("Failed to get SD card device from factory");
    return;
  }

  // 2. 初始化设备
  if (BLOCK_DEV_INIT(sd_dev) != 0) {
    log_e("SD Card initialization failed");
    return;
  }

  // 3. 获取信息
  block_dev_info_t info;
  if (BLOCK_DEV_GET_INFO(sd_dev, &info) == 0) {
    log_i("SD Card Info:");
    log_i("  Capacity: %u bytes (%u MB)", info.capacity,
          info.capacity / 1024 / 1024);
    log_i("  Block Size: %u", info.block_size);
  }

  // 4. 读写测试 (测试第 100 个块)
  uint32_t test_addr = 100 * info.block_size;
  uint8_t *write_buf = sys_malloc(SYS_MEM_INTERNAL, 512);
  uint8_t *read_buf = sys_malloc(SYS_MEM_INTERNAL, 512);

  if (!write_buf || !read_buf) {
    log_e("Failed to allocate test buffers");
    if (write_buf)
      sys_free(SYS_MEM_INTERNAL, write_buf);
    if (read_buf)
      sys_free(SYS_MEM_INTERNAL, read_buf);
    return;
  }

  for (int i = 0; i < 512; i++) {
    write_buf[i] = (uint8_t)(i & 0xFF);
  }

  log_i("Writing test data to block 100...");
  if (BLOCK_DEV_PROGRAM(sd_dev, test_addr, write_buf, 512) != 0) {
    log_e("Write failed");
    return;
  }

  log_i("Reading back test data...");
  memset(read_buf, 0, 512);
  if (BLOCK_DEV_READ(sd_dev, test_addr, read_buf, 512) != 0) {
    log_e("Read failed");
    return;
  }

  if (memcmp(write_buf, read_buf, 512) == 0) {
    log_i("SD Card Basic Read/Write Test PASSED!");
  } else {
    log_e("SD Card Data Mismatch! Test FAILED.");
    // Optional: Dump data for debugging
  }

  sys_free(SYS_MEM_INTERNAL, write_buf);
  sys_free(SYS_MEM_INTERNAL, read_buf);

  // 5. 多块读写测试 (测试 4 个块)
  uint32_t multi_size = 4 * info.block_size;
  uint8_t *multi_write_buf = sys_malloc(SYS_MEM_INTERNAL, multi_size);
  uint8_t *multi_read_buf = sys_malloc(SYS_MEM_INTERNAL, multi_size);

  if (multi_write_buf && multi_read_buf) {
    for (uint32_t i = 0; i < multi_size; i++) {
      multi_write_buf[i] = (uint8_t)((i + 0xAA) & 0xFF);
    }

    log_i("Multi-block Write (4 blocks)...");
    if (BLOCK_DEV_PROGRAM(sd_dev, test_addr + 10 * info.block_size,
                          multi_write_buf, multi_size) == 0) {
      log_i("Multi-block Read back...");
      if (BLOCK_DEV_READ(sd_dev, test_addr + 10 * info.block_size,
                         multi_read_buf, multi_size) == 0) {
        if (memcmp(multi_write_buf, multi_read_buf, multi_size) == 0) {
          log_i("Multi-block Test PASSED!");
        } else {
          log_e("Multi-block Data Mismatch!");
        }
      }
    }
  }

  if (multi_write_buf)
    sys_free(SYS_MEM_INTERNAL, multi_write_buf);
  if (multi_read_buf)
    sys_free(SYS_MEM_INTERNAL, multi_read_buf);

  log_i("SD Card Test Completed.");
}

#endif
