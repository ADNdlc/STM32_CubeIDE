#include "test_config.h"
#if ENABLE_TEST_WIFI

#define LOG_TAG "TEST_WIFI"
#include "elog.h"
#include "Sys.h"
#include "test_framework.h"

// 抽象服务层
#include "wifi_service/wifi_service.h"
#include "wifi_factory.h"
#include "gpio_factory.h"

// 交互相关
#include "gpio_key/gpio_key.h"

// 全局实例
static wifi_service_t wifi_svc;
static gpio_key_t* key_ui;
static KeyObserver key_observer;

static void scan_callback(void *arg, wifi_ap_list_t *ap_list) {
    log_i("Scan completed: %d APs found.", ap_list->count);
    for (int i = 0; i < ap_list->count; i++) {
        log_i("  AP[%d]: %s (RSSI: %d)", i, ap_list->ap_info[i].ssid, ap_list->ap_info[i].rssi);
    }
}

static void on_key_event(gpio_key_t *key, KeyEvent event) {
    switch (event) {
        case KeyEvent_SinglePress:
            log_i("Action: Scanning for WiFi...");
            wifi_svc_scan(&wifi_svc, scan_callback, NULL);
            break;
            
        case KeyEvent_DoublePress:
            log_i("Action: Connecting to WiFi [%s]...", "yu778866");
            wifi_svc_connect(&wifi_svc, "test2", "yu778866");
            break;
            
        case KeyEvent_TriplePress:
            log_i("Action: Disconnecting WiFi...");
            wifi_svc_disconnect(&wifi_svc);
            break;
            
        case KeyEvent_LongPress: {
            wifi_status_t status = wifi_svc_get_status(&wifi_svc);
            log_i("Action: Current WiFi Status: %d", status);
            break;
        }
        default: break;
    }
}

static void test_wifi_setup(void) {
    log_i("WiFi Integration Test Setup (Service Layer)...");
    
    // 1. 获取硬件资源 (按键部分)
    gpio_driver_t *key_pin = gpio_driver_get(GPIO_ID_KEY0);
    if (!key_pin) {
        log_e("Key driver missing!");
        return;
    }
    
    // 2. 初始化 WiFi 服务 (工厂会自动处理底层驱动和 AT 通信)
    wifi_svc_init(&wifi_svc, WIFI_ID_MAIN);
    wifi_svc_set_mode(&wifi_svc, WIFI_MODE_STATION);
    
    // 3. 初始化按键交互
    key_ui = Key_Create(key_pin, 0);
    key_observer.callback = on_key_event;
    Key_RegisterObserver(key_ui, &key_observer);
    
    log_i("WiFi Test Ready. Single:Scan, Double:Connect, Triple:Disconnect, Long:Status");
}

static void test_wifi_loop(void) {
    // 只需要轮询底层工厂(处理AT)和服务层(处理事件)
    wifi_factory_process();
    wifi_svc_process(&wifi_svc);
    Key_Update(key_ui);
}

static void test_wifi_teardown(void) {
    log_i("WiFi Integration Test Teardown.");
}

REGISTER_TEST(WiFi_Integration, "Interactive ESP8266 WiFi Test",
              test_wifi_setup, test_wifi_loop, test_wifi_teardown);

#endif

