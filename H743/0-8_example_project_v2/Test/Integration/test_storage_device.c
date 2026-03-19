#include "test_config.h"
#if ENABLE_TEST_STORAGE
#include "Sys.h"
#include "elog.h"
#include "gpio_factory.h"
#include "gpio_key/gpio_key.h"
#include "storage_factory.h"
#include "storage_interface.h"
#include "test_framework.h"
#include <string.h>

#define LOG_TAG "TEST_STORAGE"
#define TEST_BLOCK_ADDR 0x00000000 // 测试起始地址 (0)
#define TEST_DATA_LEN 512          // 测试数据长度

static storage_device_t *storage_dev = NULL;
static uint8_t write_buf[TEST_DATA_LEN];
static uint8_t read_buf[TEST_DATA_LEN];
static gpio_key_t *key_main = NULL;
static KeyObserver s_key_observer; // 必须是静态或全局，防止栈溢出后指针失效
static volatile uint8_t key_event_type = 0; // 记录按键类型

static void print_storage_status(void);

static void on_key_event(gpio_key_t *key, KeyEvent event) {
  switch (event) {
  case KeyEvent_SinglePress:
    log_i("KeyEvent_SinglePress");

    // 获取当前状态
    storage_status_t status = STORAGE_CHECK_ALIVE(storage_dev);

    // 【新增热插拔恢复逻辑】
    if (status == STORAGE_STATUS_OFFLINE) {
        log_i("Detect offline! Attempting to re-initialize card...");

        // 必须先 DeInit 彻底释放底层资源
        STORAGE_DEINIT(storage_dev);

        // 重新启动整个 SD 卡枚举与寻址流程
        if (STORAGE_INIT(storage_dev) == 0) {
            log_i("Card re-inserted and initialized successfully!");
        } else {
            log_w("Re-init failed, card is still missing.");
        }
    }

    print_storage_status();
    break;
  case KeyEvent_DoublePress:
    log_i("Key0 Event: Double Press");
    // 准备测试数据
    for (int i = 0; i < TEST_DATA_LEN; i++) {
      write_buf[i] = (uint8_t)(sys_get_systick_ms() + i);
    }

    // 1. 擦除 (某些驱动可能不需要，但接口支持)
    log_d("Erasing...");
    if (STORAGE_ERASE(storage_dev, TEST_BLOCK_ADDR, TEST_DATA_LEN) != 0) {
      log_e("Erase failed!");
      return;
    }

    // 2. 写入
    log_d("Writing...");
    if (STORAGE_WRITE(storage_dev, TEST_BLOCK_ADDR, write_buf, TEST_DATA_LEN) !=
        0) {
      log_e("Write failed!");
      return;
    }
    log_d("Write successful!");
    break;
  case KeyEvent_TriplePress:
    log_i("Key0 Event: Triple Press");
    break;
  case KeyEvent_LongPress:
    log_i("Key0 Event: Long Press");
    // 3. 读取
    log_d("Reading...");
    memset(read_buf, 0, TEST_DATA_LEN);
    if (STORAGE_READ(storage_dev, TEST_BLOCK_ADDR, read_buf, TEST_DATA_LEN) !=
        0) {
      log_e("Read failed!");
      return;
    }

    // 4. 校验
    if (memcmp(write_buf, read_buf, TEST_DATA_LEN) == 0) {
      log_i("Verification SUCCESS!");
    } else {
      log_e("Verification FAILED! Data mismatch.");
      // 打印前几个字节以帮助调试
      log_w("First few bytes - Write: 0x%02X, 0x%02X, 0x%02X", write_buf[0],
            write_buf[1], write_buf[2]);
      log_w("First few bytes - Read: 0x%02X, 0x%02X, 0x%02X", read_buf[0],
            read_buf[1], read_buf[2]);
      return;
    }

    log_i("Read-Write-Verify cycle completed.");
    break;
  default:
    log_i("Key0 Event: Unknown %d", event);
    break;
  }
}

static void print_storage_status(void) {
  if (storage_dev != NULL) {
    storage_status_t status = STORAGE_CHECK_ALIVE(storage_dev);

    switch (status) {
    case STORAGE_STATUS_OK:
      log_i("Storage Status: OK");
      break;
    case STORAGE_STATUS_NOT_INIT:
      log_i("Storage Status: NOT INITIALIZED");
      break;
    case STORAGE_STATUS_OFFLINE:
      log_i("Storage Status: OFFLINE/NOT INSERTED");
      break;
    case STORAGE_STATUS_BUSY:
      log_i("Storage Status: BUSY");
      break;
    case STORAGE_STATUS_ERROR:
      log_i("Storage Status: ERROR");
      break;
    default:
      log_i("Storage Status: UNKNOWN (%d)", status);
      break;
    }
  } else {
    log_w("Storage device not initialized");
  }
}

static void test_storage_setup(void) {
  log_i("Storage Test Setup: Initializing SDMMC Storage...");

  // 1. 通过工厂获取设备
  storage_dev = storage_factory_get(STORAGE_ID_SD_CARD);
  if (storage_dev == NULL) {
    log_e("Failed to create SDMMC storage device from factory!");
    return;
  }

  // 2. 初始化设备
  if (STORAGE_INIT(storage_dev) != 0) {
    log_e("Storage initialization failed! Is SD card inserted?");
  }

  // 3. 获取并打印设备信息
  if(storage_dev){
	  storage_info_t info;
	  if (STORAGE_GET_INFO(storage_dev, &info) == 0) {
		log_i("Storage Info:");
		log_i("  Type: %d", info.type);
		log_i("  Total Size: %llu Bytes", info.total_size);
		log_i("  Erase Size: %lu Bytes", info.erase_size);
		log_i("  Write Size: %lu Bytes", info.write_size);
	  }
  }

  // 4. 初始化按键
  gpio_driver_t *key_gpio = gpio_driver_get(GPIO_ID_KEY0);
  if (key_gpio == NULL) {
    log_e("GPIO_ID_KEY0 not found");
    return;
  }

  key_main = Key_Create(key_gpio, 0);
  if (key_main != NULL) {
    s_key_observer.callback = on_key_event;
    Key_RegisterObserver(key_main, &s_key_observer);
    log_i("Key registered for storage testing");
  }

  log_i("Storage Test Setup Complete.");

  // 打印初始状态
  print_storage_status();
}

static void test_storage_loop(void) {
  // 更新按键状态
  if (key_main != NULL) {
    Key_Update(key_main);
  }
}

static void test_storage_teardown(void) {
  log_i("Storage Test Teardown.");
  if (storage_dev) {
    STORAGE_DEINIT(storage_dev);
    storage_dev = NULL;
  }

  if (key_main) {
    Key_Destroy(key_main);
    key_main = NULL;
  }
}

REGISTER_TEST(STORAGE,
              "Interactive SDMMC storage read/write test with button control",
              test_storage_setup, test_storage_loop, test_storage_teardown);

#endif
