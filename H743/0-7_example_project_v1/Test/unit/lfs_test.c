#include "elog.h"
#include "flash_factory.h"
#include "flash_handler.h"
#include "lv_port_fs.h"
#include "res_manager.h"
#include "strategy/lfs_strategy.h"
#include "sys.h"
#include <string.h>

#define LOG_TAG "LFS_TEST"

#define TEST_DEV_QSPI 0
#define TEST_DEV_NAND 1
#define TEST_DEV_SELECT TEST_DEV_NAND // Switch here

#if TEST_DEV_SELECT == TEST_DEV_NAND
#include "../../Drivers/device/mt29f4g08/mt29f4g08.h"
#include "../../Drivers/device/mt29f4g08/transport/mt29f_fmc_adapter.h"
// External handle declaration or assume initialized via some mechanism
extern NAND_HandleTypeDef hnand1;
#endif

void lfs_integration_test(void) {
  log_a("Starting LittleFS Integration Test...");

  // 1. Initialize Handler
  flash_handler_init();

  block_device_t *dev = NULL;

#if TEST_DEV_SELECT == TEST_DEV_QSPI
  // 2. Get Device (QSPI)
  dev = flash_factory_get(FLASH_EXT_QSPI);
  if (!dev) {
    log_e("Flash Factory Failed for QSPI");
    return;
  }
#elif TEST_DEV_SELECT == TEST_DEV_NAND
  // 2. Get Device (NAND)
  // Create Adapter with external handle (assuming hnand1 is available from
  // main)
  mt29f_adapter_t *nand_adapter = mt29f_fmc_adapter_create(&hnand1);
  if (!nand_adapter) {
    log_e("NAND Adapter Create Failed");
    return;
  }

  if (nand_adapter->ops->init(nand_adapter) != 0) {
    log_e("NAND Adapter Init Failed");
    return;
  }

  dev = mt29f4g08_create(nand_adapter);
  if (!dev) {
    log_e("NAND Device Create Failed");
    return;
  }

  if (BLOCK_DEV_INIT(dev) != 0) {
    log_e("NAND Block Device Init Failed");
    return;
  }
#endif

  // 3. Create LittleFS Strategy
  // Adjust config based on selected device if needed, or use safe defaults
  lfs_strategy_config_t lfs_cfg = {
#if TEST_DEV_SELECT == TEST_DEV_NAND
      .read_size = 2048,
      .prog_size = 2048,
      .cache_size = 2048,
      .lookahead_size = 128, // Increase lookahead for larger device
      .block_cycles = 500,
#else
      .read_size = 16,
      .prog_size = 16,
      .cache_size = 256,
      .lookahead_size = 32,
      .block_cycles = 200,
#endif
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
  //lv_port_fs_init();

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
