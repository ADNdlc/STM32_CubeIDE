#include "test_config.h"
#if ENABLE_TEST_SETTING
#include "Sys.h"
#include "app_settings.h"
#include "elog.h"
#include "fatfs_strategy.h"
#include "storage_factory.h"
#include "sys_config.h"
#include "test_framework.h"
#include "vfs_manager.h"

// 交互支持
#include "gpio_factory.h"
#include "gpio_key/gpio_key.h"
#include <string.h>

#define LOG_TAG "TEST_SET"

// 静态资源
static fs_strategy_t *fatfs_strat = NULL;
static storage_device_t *sd_storage = NULL;
static gpio_key_t *key_ui;
static KeyObserver key_observer;

static void on_key_event(gpio_key_t *key, KeyEvent event) {
  switch (event) {
  case KeyEvent_SinglePress:
    log_i("--- Config Verification ---");
    log_i("Current WiFi SSID: %s", sys_config_get_wifi_ssid());
    log_i("Current Platform:  %d", sys_config_get_cloud_platform());
    log_i("Current Product ID: %s", sys_config_get_cloud_product_id());
    log_i("Current Device ID: %s", sys_config_get_cloud_device_id());
    log_i("Current Device Secret: %s", sys_config_get_cloud_device_secret());
    log_i("--- End ---");
    break;

  case KeyEvent_DoublePress:
    log_i("Action: Modifying SSID to 'Antigravity_AP' and Saving...");
    sys_config_set_wifi_ssid("Antigravity_AP");
    sys_config_set_wifi_password("pass123456");

    if (sys_config_save() == 0) {
      log_i("Save successful to VFS.");
    } else {
      log_e("Save failed!");
    }
    break;

  case KeyEvent_LongPress:
    log_i("Action: Reloading from VFS to verify persistence...");
    // 强制调用 app_settings 从文件重载
    if (app_settings_load("system", "system") == 0) {
      log_i("Reload SUCCESS. Current SSID: %s", sys_config_get_wifi_ssid());
    } else {
      log_e("Reload FAILED!");
    }
    break;

  default:
    break;
  }
}

static void test_settings_setup(void) {
  // 1. 初始化核心管理器
  vfs_init();

  // 2. 获取存储抽象并挂载
  sd_storage = storage_factory_get(STORAGE_ID_SD_CARD);
  if (!sd_storage) {
    log_e("SD Card storage device not found in factory!");
    return;
  }

  fatfs_strat = fatfs_strategy_create();
  if (!fatfs_strat) {
    log_e("Failed to create FatFs strategy!");
    return;
  }

  // 挂载前缀为 "sys"，对应路径 "/sys/..."
  int res = vfs_mount("sys", sd_storage, fatfs_strat);
  if (res != VFS_OK) {
    log_e("VFS Mount for /sys failed: %d", res);
    return;
  }

  // 3. 初始化业务配置层
  if (sys_config_init() != 0) {
    log_e("SysConfig initialization failed!");
    return;
  }

  // 4. 初始化按键交互 (复用 KEY0)
  gpio_driver_t *pin = gpio_driver_get(GPIO_ID_KEY0);
  if (pin) {
    key_ui = Key_Create(pin, 0);
    key_observer.callback = on_key_event;
    Key_RegisterObserver(key_ui, &key_observer);
    log_i("Settings test ready. KEY0: Single=View, Double=Modify&Save, "
          "Long=Reload");
  } else {
    log_e("KEY0 driver not found!");
  }
}

static void test_settings_loop(void) {
  vfs_storage_monitor_task();
  if (key_ui) {
    Key_Update(key_ui);
  }
}

static void test_settings_teardown(void) {
  log_i("Tearing down settings test...");
  vfs_unmount("sys");
  if (fatfs_strat) {
    fatfs_strategy_destroy((fatfs_strategy_t *)fatfs_strat);
    fatfs_strat = NULL;
  }
  if (key_ui) {
    Key_Destroy(key_ui);
    key_ui = NULL;
  }
}

REGISTER_TEST(Settings, "System configuration and app_settings integration",
              test_settings_setup, test_settings_loop, test_settings_teardown);

#endif
