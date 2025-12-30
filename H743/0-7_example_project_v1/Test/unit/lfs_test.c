#include "elog.h"
#include "flash_factory.h"
#include "flash_handler.h"
#include "lv_port_fs.h"
#include "res_manager.h"
#include "strategy/lfs_strategy.h"
#include "sys.h"
#include <string.h>

#define LOG_TAG "LFS_TEST"

void lfs_integration_test(void) {
  log_a("Starting LittleFS Integration Test...");

  // 1. Initialize Handler
  flash_handler_init();

  // 2. Get Device (QSPI)
  block_device_t *dev = flash_factory_get(FLASH_EXT_QSPI);
  if (!dev) {
    log_e("Flash Factory Failed for QSPI");
    return;
  }

  // 3. Create LittleFS Strategy
  lfs_strategy_config_t lfs_cfg = {
      .read_size = 16,
      .prog_size = 16,
      .cache_size = 256,
      .lookahead_size = 32,
      .block_cycles = 200,
  };
  flash_strategy_t *lfs_strat = lfs_strategy_create(&lfs_cfg);
  if (!lfs_strat) {
    log_e("LFS Strategy Create Failed");
    return;
  }

  // 4. Register Device with /lfs prefix
  if (flash_handler_register("/lfs", dev, lfs_strat) != 0) {
    log_e("LFS Handler Register Failed");
    return;
  }

  // 5. Initialize LVGL FS Bridge
  lv_port_fs_init();

  // 6. Test Resource Manager 没有烧录资源暂不测试
  // const char *img_path = res_get_src(RES_IMG_WALLPAPER);
  // log_i("Resource Manager verify: ID=RES_IMG_WALLPAPER, Path=%s", img_path);

  // 7. Data Persistence Test
  const char *test_path = "/lfs/test.txt";
  const char *test_data = "LVGL FS Bridge Test Content";
  uint8_t read_buf[64] = {0};

  log_i("Writing to %s...", test_path);
  if (flash_handler_write(test_path, 0, (const uint8_t *)test_data,
                          strlen(test_data)) != 0) {
    log_e("LFS Write Failed");
  }

  log_i("Reading from %s...", test_path);
  if (flash_handler_read(test_path, 0, read_buf, strlen(test_data)) != 0) {
    log_e("LFS Read Failed");
  } else {
    log_i("LFS Read Success: %s", (char *)read_buf);
    if (strcmp(test_data, (char *)read_buf) == 0) {
      log_i("LFS Data Match!");
    } else {
      log_e("LFS Data Mismatch!");
    }
  }
}
