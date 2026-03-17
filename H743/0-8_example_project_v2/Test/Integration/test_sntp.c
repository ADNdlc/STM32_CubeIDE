#include "test_config.h"
#if ENABLE_TEST_SNTP // sntp tests usually need wifi

#define LOG_TAG "TEST_SNTP"
#include "Sys.h"
#include "elog.h"
#include "test_framework.h"

// 抽象服务层
#include "gpio_factory.h"
#include "rtc_driver.h"
#include "rtc_factory.h"
#include "sntp_factory.h"
#include "sntp_service/sntp_service.h"
#include "wifi_factory.h"
#include "wifi_service/wifi_service.h"

// 交互相关
#include "gpio_key/gpio_key.h"

// 全局实例
static wifi_service_t wifi_svc;
static sntp_service_t sntp_svc;
static rtc_driver_t *rtc_drv = NULL;

static gpio_key_t *key_ui;
static KeyObserver key_observer;

// SNTP 同步结果回调 (sntp_service 内部逻辑可能需要对齐)
static void on_sync_done(void *arg, int result) {
  if (result == 0) {
    log_i("SNTP Sync Success via Service!");
    // 时间已由 sntp_service 内部逻辑同步或在此处理
  } else {
    log_e("SNTP Sync Failed via Service!");
  }
}

static void on_key_event(gpio_key_t *key, KeyEvent event) {
  switch (event) {
  case KeyEvent_LongPress:
    log_i("Action: Connecting WiFi for SNTP test...");
    wifi_svc_connect(&wifi_svc, "test2", "yu778866");
    break;

  case KeyEvent_SinglePress:
    log_i("Action: Configuring SNTP (UTC+8)...");
    // sntp_service 可能有自己的配置逻辑
    sntp_svc_set_config(&sntp_svc, 8, "ntp.aliyun.com");
    break;

  case KeyEvent_DoublePress:
    log_i("Action: Starting SNTP Sync...");
    if (sntp_svc_start_sync(&sntp_svc) != 0) {
      log_e("SNTP Sync Failed via Service!");
    }
    break;

  case KeyEvent_TriplePress: {
    if (rtc_drv) {
      rtc_time_t curr_time;
      rtc_date_t curr_date;
      RTC_GET_TIME(rtc_drv, &curr_time);
      RTC_GET_DATE(rtc_drv, &curr_date);
      log_i("Current RTC: 20%02d-%02d-%02d %02d:%02d:%02d", curr_date.year,
            curr_date.month, curr_date.day, curr_time.hour, curr_time.minute,
            curr_time.second);
    }
    break;
  }

  default:
    break;
  }
}

static void test_sntp_setup(void) {
  log_i("SNTP Integration Test Setup (Service Layer)...");

  // 1. 获取硬件资源
  gpio_driver_t *key_pin = gpio_driver_get(GPIO_ID_KEY0);
  rtc_drv = rtc_driver_get(RTC_ID_INTERNAL);

  if (!key_pin) {
    log_e("Key driver missing!");
    return;
  }

  // 2. 初始化核心服务 (使用工厂)
  wifi_svc_init(&wifi_svc, wifi_driver_get(WIFI_ID_MAIN));
  wifi_svc_set_mode(&wifi_svc, WIFI_MODE_STATION);

  sntp_svc_init(&sntp_svc, sntp_driver_get(SNTP_ID_MAIN));

  // 3. 初始化按键
  key_ui = Key_Create(key_pin, 0);
  key_observer.callback = on_key_event;
  Key_RegisterObserver(key_ui, &key_observer);

  log_i("SNTP Test Ready. Long:WiFiConnect, Single:NetReady, Double:Sync, "
        "Triple:ReadRTC");
}

static void test_sntp_loop(void) {
  wifi_svc_process(&wifi_svc);
  Key_Update(key_ui);
}

static void test_sntp_teardown(void) {
  log_i("SNTP Integration Test Teardown.");
}

REGISTER_TEST(SNTP_Integration, "Interactive ESP8266 SNTP Test",
              test_sntp_setup, test_sntp_loop, test_sntp_teardown);

#endif
