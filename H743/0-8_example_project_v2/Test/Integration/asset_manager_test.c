#include "test_config.h"
#if ENABLE_TEST_ASSET_MANAGER
#define LOG_TAG "TEST_ASSET"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "Sys.h"
#include "asset_manager.h"
#include "elog.h"
#include "test_framework.h"

#define TEST_RES_A_ID 0xA1B2C3D4
#define TEST_RES_B_ID 0xB1C2D3E4
#define TEST_RES_C_ID 0xC1D2E3F4

static void asset_mgr_test_setup(void) {
  if (asset_manager_init() != 0) {
    log_e("Asset Manager init failed!");
  } else {
    log_i("Asset Manager initialized successfully.");
  }
}

static void asset_mgr_test_loop(void) {
  log_i("--- Start Asset Manager Test ---");

  // Phase 1: Normal Update (Write Res A and B)
  log_i("[Phase 1] Normal Atomic Update");
  if (asset_manager_begin_update(2) != 0) {
    log_e("Failed to begin update.");
    return;
  }

  uint8_t data_a[64];
  memset(data_a, 0xAA, sizeof(data_a));
  if (asset_manager_write_res(TEST_RES_A_ID, 1, data_a, sizeof(data_a)) != 0) {
    log_e("Failed to write Resource A");
    return;
  }

  uint8_t data_b[128];
  memset(data_b, 0xBB, sizeof(data_b));
  if (asset_manager_write_res(TEST_RES_B_ID, 1, data_b, sizeof(data_b)) != 0) {
    log_e("Failed to write Resource B");
    return;
  }

  log_i("Committing new TOC...");
  if (asset_manager_end_update() != 0) {
    log_e("Failed to end update.");
    return;
  }

  // Verify Read
  uint32_t size_a = 0, size_b = 0;
  const uint8_t *ptr_a = asset_manager_get_res(TEST_RES_A_ID, &size_a);
  const uint8_t *ptr_b = asset_manager_get_res(TEST_RES_B_ID, &size_b);

  if (ptr_a && size_a == sizeof(data_a) && ptr_a[0] == 0xAA) {
    log_i("Resource A retrieved & mapped correctly. (Addr: %p)", ptr_a);
  } else {
    log_e("Resource A retrieval failed.");
  }

  if (ptr_b && size_b == sizeof(data_b) && ptr_b[0] == 0xBB) {
    log_i("Resource B retrieved & mapped correctly. (Addr: %p)", ptr_b);
  } else {
    log_e("Resource B retrieval failed.");
  }

  // Phase 2: Interrupted Update Testing (Simulate Power Loss)
  log_i("[Phase 2] Interrupted Update (Simulating drop before committing TOC)");
  if (asset_manager_begin_update(1) != 0) {
    log_e("Failed to begin interrupted update.");
    return;
  }

  uint8_t data_c[32];
  memset(data_c, 0xCC, sizeof(data_c));
  if (asset_manager_write_res(TEST_RES_C_ID, 2, data_c, sizeof(data_c)) != 0) {
    log_e("Failed to write Resource C");
    return;
  }

  log_i("Intentionally SKIPPING asset_manager_end_update()...");

  // 手动恢复 XIP 模式，因为跳过了 end_update，底层仍处于 COMMAND 模式
  // 如果不恢复，下面解引用 ptr_a 和 ptr_c 将会引发 MCU 的 PRECISERR 总线异常
  log_i("Simulating reboot / re-init after power loss...");
  asset_manager_force_reset_state();
  asset_manager_init();

  // Verify C isolation (Should be NULL since TOC was never committed)
  uint32_t size_c = 0;
  const uint8_t *ptr_c = asset_manager_get_res(TEST_RES_C_ID, &size_c);
  if (ptr_c == NULL) {
    log_i("Resource C properly isolated (Not in TOC).");
  } else {
    log_e("Resource C incorrectly mapped despite incomplete update!");
  }

  // Verify A and B are still fully valid mapped in XIP from previous run
  ptr_a = asset_manager_get_res(TEST_RES_A_ID, &size_a);
  if (ptr_a && size_a == sizeof(data_a) && ptr_a[0] == 0xAA) {
    log_i("Resource A still valid after interrupted update.");
  } else {
    log_e("Resource A corrupted or lost after interrupted update!");
  }

  log_i("--- Asset Manager Test Finished ---");
  Stop_Current_Test();
}

static void asset_mgr_test_teardown(void) { log_i("Asset test teardown."); }

REGISTER_TEST(asset_mgr, "Asset Manager Integration Test", asset_mgr_test_setup,
              asset_mgr_test_loop, asset_mgr_test_teardown);

#endif
