#include "test_config.h"
#if ENABLE_TEST_WIFI

#define LOG_TAG "TEST_WIFI"
#include "elog.h"
#include "Sys.h"
#include "test_framework.h"

// 驱动相关
#include "esp_8266/at_controller.h"
#include "esp_8266/esp8266_wifi_driver.h"
#include "usart_factory.h"
#include "gpio_factory.h"
#include "uart_queue/uart_queue.h"
#include "usart_hal/usart_hal.h"

// 交互相关
#include "gpio_key/gpio_key.h"

// 全局实例
static at_controller_t at_ctrl;
static uart_queue_t at_uart_q;
static esp8266_wifi_driver_t wifi_drv;
static gpio_key_t* key_ui;
static KeyObserver key_observer;

// 缓冲区
static uint8_t at_tx_buf[256];
static uint8_t at_rx_buf[512];

// SSID/PWD (可以从 sys_config 获取，这里预留占位符)
#include "sys_config.h"

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
            WIFI_SCAN((wifi_driver_t*)&wifi_drv, scan_callback, NULL);
            break;
            
        case KeyEvent_DoublePress:
            log_i("Action: Connecting to WiFi [%s]...", sys_config_get_wifi_ssid());
            WIFI_CONNECT((wifi_driver_t*)&wifi_drv, sys_config_get_wifi_ssid(), sys_config_get_wifi_password());
            break;
            
        case KeyEvent_TriplePress:
            log_i("Action: Disconnecting WiFi...");
            WIFI_DISCONNECT((wifi_driver_t*)&wifi_drv);
            break;
            
        case KeyEvent_LongPress: {
            wifi_status_t status = WIFI_GET_STATUS((wifi_driver_t*)&wifi_drv);
            log_i("Action: Current WiFi Status: %d", status);
            break;
        }
        default: break;
    }
}

static void test_wifi_setup(void) {
    log_i("WiFi Integration Test Setup...");
    
    // 1. 获取硬件资源
    usart_driver_t *u_drv = usart_driver_get(USART_ATCMD);
    gpio_driver_t *rst_pin = gpio_driver_get(GPIO_ESP_RST);
    gpio_driver_t *key_pin = gpio_driver_get(GPIO_ID_KEY0);
    
    if (!u_drv || !rst_pin || !key_pin) {
        log_e("Hardware drivers missing!");
        return;
    }
    
    // 2. 初始化 AT 通信
    usart_hal_t *hal = usart_hal_create(u_drv);
    uart_queue_init(&at_uart_q, hal, at_tx_buf, sizeof(at_tx_buf), at_rx_buf, sizeof(at_rx_buf));
    uart_queue_start_receive(&at_uart_q);
    
    at_controller_init(&at_ctrl, &at_uart_q, rst_pin);
    
    // 3. 初始化 WiFi 驱动
    esp8266_wifi_driver_init(&wifi_drv, &at_ctrl);
    WIFI_SET_MODE((wifi_driver_t*)&wifi_drv, WIFI_MODE_STATION);
    
    // 4. 初始化按键交互
    key_ui = Key_Create(key_pin, 0);
    key_observer.callback = on_key_event;
    Key_RegisterObserver(key_ui, &key_observer);
    
    log_i("WiFi Test Ready. Single:Scan, Double:Connect, Triple:Disconnect, Long:Status");
}

static void test_wifi_loop(void) {
    at_controller_process(&at_ctrl);
    Key_Update(key_ui);
}

static void test_wifi_teardown(void) {
    log_i("WiFi Integration Test Teardown.");
}

REGISTER_TEST(WiFi_Integration, "Interactive ESP8266 WiFi Test",
              test_wifi_setup, test_wifi_loop, test_wifi_teardown);

#endif
