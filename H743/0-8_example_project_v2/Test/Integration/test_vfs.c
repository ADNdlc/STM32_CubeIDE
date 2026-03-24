#include "test_config.h"
#if ENABLE_TEST_VFS
#include "Sys.h"
#include "elog.h"
#include "sdmmc/sdmmc_storage_drv.h"
#include "strategy/fatfs_strategy.h"
#include "test_framework.h"
#include "vfs_manager.h"

// 交互相关
#include "gpio_factory.h"
#include "gpio_key/gpio_key.h"

#define LOG_TAG "TEST_VFS"

// 全局实例
static fs_strategy_t *fatfs_strat = NULL;
static storage_device_t *sd_storage = NULL;
static gpio_key_t *key_ui;
static KeyObserver key_observer;
static uint32_t write_count = 0; // 记录写入次数

static void vfs_event_handler(const char *prefix, dev_event_t event) {
  log_i("App Notify: Path [%s] event %d", prefix, event);
}

static void vfs_format_notify(const char *prefix, bool start) {
  if (start) {
    log_w("Format started for %s", prefix);
  } else {
    log_w("Format finished for %s", prefix);
  }
}

static void on_key_event(gpio_key_t *key, KeyEvent event) {
  switch (event) {
  case KeyEvent_SinglePress:
    log_i("Action: Writing to file...");

    // 写入文件 - 注意使用统一的 VFS_O_* 宏
    int fd =
        vfs_open("/sd/vfs_test.txt", VFS_O_WRONLY | VFS_O_CREAT | VFS_O_APPEND);
    if (fd >= 0) {
      write_count++;
      char buffer[128];
      snprintf(buffer, sizeof(buffer), "VFS Test Write #%lu at %lu ms\n",
               write_count, sys_get_systick_ms());
      vfs_write(fd, buffer, strlen(buffer));
      vfs_close(fd);
      log_i("Write success. Total writes: %lu", write_count);
    } else {
      log_e("Open failed, maybe SD not inserted or unmounted?");
    }
    break;

  case KeyEvent_DoublePress:
    log_i("Action: Reading file & listing dir...");
    // 读取文件
    fd = vfs_open("/sd/vfs_test.txt", VFS_O_RDONLY);
    if (fd >= 0) {
      char buffer[256];
      int bytes_read = vfs_read(fd, buffer, sizeof(buffer) - 1);
      if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        log_i("File content:\n%s", buffer);
      } else {
        log_w("File is empty or read error");
      }
      vfs_close(fd);
    } else {
      log_e("Cannot open file for reading");
    }

    // 测试读取目录
    vfs_dir_t d;
    if (vfs_opendir("/sd", &d) == VFS_OK) {
      log_i("--- Directory Listing of /sd ---");
      vfs_dirent_t ent;
      while (vfs_readdir(d, &ent) > 0) {
        log_i(" %s  %s  (%lu bytes)", ent.info.is_dir ? "<DIR>" : "     ",
              ent.name, ent.info.size);
      }
      vfs_closedir(d);
      log_i("--- End of Directory ---");
    } else {
      log_e("Cannot open /sd directory");
    }
    break;
  // 进行格式化
  case KeyEvent_TriplePress:
    log_i("Action: User requested Format...");
    int fmt_res = vfs_format("/sd", vfs_format_notify);
    if (fmt_res == VFS_OK) {
      log_i("Format Success! You can now Double-Click to read/write.");
    } else {
      log_e("Format Failed with code: %d", fmt_res);
    }
    break;

  case KeyEvent_LongPress:
    log_i("Action: Checking storage status...");
    // 检查存储状态
    vfs_storage_monitor_task();
    break;

  default:
    break;
  }
}

static void test_vfs_setup(void) {
  // 1. 初始化 VFS
  vfs_init();
  vfs_set_event_callback(vfs_event_handler);

  // 2. 获取SD存储设备
  extern SD_HandleTypeDef hsd1;
  sd_storage = sdmmc_storage_drv_get(&hsd1);
  if (sd_storage == NULL) {
    log_e("SD Storage not found");
    return;
  }

  // 3. 创建文件系统策略 (替换 LFS 为 FATFS)
  fatfs_strat = fatfs_strategy_create();
  if (fatfs_strat == NULL) {
    log_e("FATFS Strategy creation failed");
    return;
  }

  // 4. 挂载SD卡到/sd
  int ret = vfs_mount("sd", sd_storage, fatfs_strat);

  // 业务层接管错误，而不是直接退出
  if (ret == VFS_ERR_NO_FS) {
    log_w("======================================");
    log_w(" SD Card has NO valid filesystem!");
    log_w(" Press TRIPLE-CLICK to Format the card.");
    log_w("======================================");
  } else if (ret != VFS_OK) {
    log_e("VFS Mount failed with unknown error: %d", ret);
    return;
  } else {
    log_i("VFS Test Setup: VFS initialized and SD card mounted.");
  }

  // 5. 初始化按键交互
  gpio_driver_t *key_pin = gpio_driver_get(GPIO_ID_KEY0);
  if (!key_pin) {
    log_e("Key driver missing!");
    return;
  }

  key_ui = Key_Create(key_pin, 0);
  key_observer.callback = on_key_event;
  Key_RegisterObserver(key_ui, &key_observer);

  log_i("Press KEY0 to interact: Single-Write, Double-Read, Long-Check");
}

static void test_vfs_loop(void) {
  // 执行存储监控任务
  vfs_storage_monitor_task();
  // 更新按键状态
  Key_Update(key_ui);
}

static void test_vfs_teardown(void) {
  log_i("VFS Test Teardown: Unmounting and cleaning up.");

  // 卸载文件系统
  vfs_unmount("sd");

  // 销毁文件系统策略
  if (fatfs_strat != NULL) {
    fatfs_strategy_destroy((fatfs_strategy_t *)fatfs_strat);
    fatfs_strat = NULL;
  }

  // 清理存储设备引用
  sd_storage = NULL;

  // 清理按键资源
  if (key_ui) {
    Key_Destroy(key_ui);
    key_ui = NULL;
  }
}

REGISTER_TEST(VFS, "Interactive VFS operations with SD card", test_vfs_setup,
              test_vfs_loop, test_vfs_teardown);
#endif
