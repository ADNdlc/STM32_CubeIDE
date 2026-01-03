#include "elog.h"
/**
 * @brief
 * 测试sys_config是否能成功从外部flash加载配置信息或在失败时保障配置信息不为空
 */
#include "elog.h"
#include "flash_handler.h"
#include "home/System/sys_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define LOG_TAG "SYS_CFG_TEST"

/**
 * @brief
 * 测试sys_config是否能成功从外部flash加载配置信息或在失败时保障配置信息不为空
 */
void sys_config_test_run(void) {
  log_i("Starting sys_config test...");

  // 1. 初始化并检查默认值 (Case 1: Default Values)
  // 假设此测试在干净的系统或手动损坏文件后运行
  log_i("Case 1: Check default values on fresh init");
  sys_config_init();
  const sys_config_t *cfg = sys_config_get();

  // 检查默认 SSID (对应 sys_config.c 中的 DEFAULT_WIFI_SSID "test1")
  if (strcmp(cfg->net.ssid, "test1") == 0) {
    log_i("Default config check PASSED (SSID: %s)", cfg->net.ssid);
  } else {
    log_w("Default config mismatch (Current SSID: %s), might have existing "
          "config.",
          cfg->net.ssid);
  }

  // 2. 持久化测试 (Case 2: Persistence)
  log_i("Case 2: Persistence Test (Update -> Save -> Reload)");
  net_config_t test_net = {.ssid = "Antigravity_AP",
                           .password = "supersecret",
                           .auto_connect = false};
  cloud_config_t test_cloud = {.platform = CLOUD_PLATFORM_ALIYUN,
                               .device_id = "dev_12345",
                               .product_id = "prod_abcde",
                               .device_secret = "sec_987654321"};

  sys_config_set_net(&test_net);
  sys_config_set_cloud(&test_cloud);

  if (sys_config_save() != 0) {
    log_e("sys_config_save failed!");
    return;
  }
  log_i("Config saved to flash.");

  // 模拟重启：重新加载
  if (sys_config_init() != 0) {
    log_e("sys_config_init reload failed!");
    return;
  }

  cfg = sys_config_get();
  if (strcmp(cfg->net.ssid, "Antigravity_AP") == 0 &&
      cfg->net.auto_connect == false &&
      cfg->cloud.platform == CLOUD_PLATFORM_ALIYUN &&
      strcmp(cfg->cloud.device_secret, "sec_987654321") == 0) {
    log_i("Persistence verification PASSED");
  } else {
    log_e("Persistence verification FAILED!");
    log_e("Loaded SSID: %s, Platform: %d", cfg->net.ssid, cfg->cloud.platform);
  }

  // 3. 容错与恢复测试 (Case 3: Robustness & Recovery)
  log_i("Case 3: Robustness Test (Corrupt file -> Reload)");
  // 故意写入完全不可解析的垃圾数据
  const char *corrupt_json =
      "!!INVALID_JSON_DATA!!_CORRUPTED_BY_TEST_1234567890";
  // 注意: 这里由于加了 LFS_O_TRUNC，写入 offset 0 会清空文件
  if (flash_handler_write("/sys/sys_cfg.json", 0, (const uint8_t *)corrupt_json,
                          strlen(corrupt_json)) == 0) {
    log_i("Simulated config corruption by writing garbage data.");
  }

  // 重新加载，此时应该解析失败并回滚到默认值
  sys_config_init();
  cfg = sys_config_get();

  if (strcmp(cfg->net.ssid, "test1") == 0) {
    log_i("Recovery to default values PASSED");
  } else {
    log_e("Recovery FAILED! Current SSID: %s", cfg->net.ssid);
  }

  log_i("sys_config test completed.");
}
