#include "test_config.h"
#if ENABLE_TEST_WIFI // sntp tests usually need wifi

#define LOG_TAG "TEST_SNTP"
#include "elog.h"
#include "Sys.h"
#include "test_framework.h"

// 驱动相关
#include "esp_8266/at_controller.h"
#include "esp_8266/esp8266_sntp_driver.h"
#include "esp_8266/esp8266_wifi_driver.h"
#include "usart_factory.h"
#include "gpio_factory.h"
#include "uart_queue/uart_queue.h"
#include "usart_hal/usart_hal.h"
#include "rtc_driver.h"
#include "rtc_factory.h"

// 交互相关
#include "gpio_key/gpio_key.h"

// 全局实例
static at_controller_t at_ctrl;
static uart_queue_t at_uart_q;
static esp8266_wifi_driver_t wifi_drv;
static esp8266_sntp_driver_t sntp_drv;
static rtc_driver_t *rtc_drv = NULL;

static gpio_key_t* key_ui;
static KeyObserver key_observer;

// 缓冲区
static uint8_t at_tx_buf[256];
static uint8_t at_rx_buf[512];

#include "sys_config.h"

// SNTP 回调
static void on_sntp_sync(void *arg, int result, sntp_time_t *time) {
    if (result == 0) {
        log_i("SNTP Sync Success!");
        log_i("Time: %02d:%02d:%02d, Date: 20%02d-%02d-%02d (Week:%d)", 
              time->time.hour, time->time.minute, time->time.second,
              time->date.year, time->date.month, time->date.day, time->date.week);
        
        if (rtc_drv) {
            log_i("Updating RTC...");
            RTC_SET_TIME(rtc_drv, &time->time);
            RTC_SET_DATE(rtc_drv, &time->date);
            log_i("RTC Updated.");
        }
    } else {
        log_e("SNTP Sync Failed!");
    }
}

static void on_key_event(gpio_key_t *key, KeyEvent event) {
    switch (event) {
        case KeyEvent_LongPress:
            log_i("Action: Connecting WiFi for SNTP test...");
            WIFI_CONNECT((wifi_driver_t*)&wifi_drv, sys_config_get_wifi_ssid(), sys_config_get_wifi_password());
            break;

        case KeyEvent_SinglePress: {
            log_i("Action: Configuring SNTP (NTP Server)...");
            SNTP_DRV_CONFIG((sntp_driver_t*)&sntp_drv, 8, "ntp.aliyun.com");
            break;
        }
            
        case KeyEvent_DoublePress:
            log_i("Action: Starting SNTP Sync...");
            SNTP_DRV_START_SYNC((sntp_driver_t*)&sntp_drv, on_sntp_sync, NULL);
            break;
            
        case KeyEvent_TriplePress: {
            if (rtc_drv) {
                rtc_time_t curr_time;
                rtc_date_t curr_date;
                RTC_GET_TIME(rtc_drv, &curr_time);
                RTC_GET_DATE(rtc_drv, &curr_date);
                log_i("Current RTC: 20%02d-%02d-%02d %02d:%02d:%02d", 
                      curr_date.year, curr_date.month, curr_date.day,
                      curr_time.hour, curr_time.minute, curr_time.second);
            }
            break;
        }
            
        default: break;
    }
}

static void test_sntp_setup(void) {
    log_i("SNTP Integration Test Setup...");
    
    // 1. 获取硬件资源
    usart_driver_t *u_drv = usart_driver_get(USART_ID_ESP8266);
    gpio_driver_t *rst_pin = gpio_driver_get(TOUCH_RST); // 假设复位引脚
    // Wait, PH3 is KEY0. Use that for interaction.
    gpio_driver_t *key_pin = gpio_driver_get(GPIO_ID_KEY0);
    
    // RTC
    rtc_drv = rtc_driver_get(RTC_ID_INTERNAL);
    
    if (!u_drv || !key_pin) {
        log_e("Hardware drivers missing!");
        return;
    }
    
    // 2. 初始化 AT 通信
    usart_hal_t *hal = usart_hal_create(u_drv);
    uart_queue_init(&at_uart_q, hal, at_tx_buf, sizeof(at_tx_buf), at_rx_buf, sizeof(at_rx_buf));
    uart_queue_start_receive(&at_uart_q);
    at_controller_init(&at_ctrl, &at_uart_q, rst_pin);
    
    // 3. 初始化驱动 (WiFi + SNTP)
    esp8266_wifi_driver_init(&wifi_drv, &at_ctrl);
    WIFI_SET_MODE((wifi_driver_t*)&wifi_drv, WIFI_MODE_STATION);
    
    esp8266_sntp_driver_init(&sntp_drv, &at_ctrl);
    
    // 4. 初始化按键
    key_ui = Key_Create(key_pin, 0);
    key_observer.callback = on_key_event;
    Key_RegisterObserver(key_ui, &key_observer);
    
    log_i("SNTP Test Ready. Long:WiFiConnect, Single:Config, Double:Sync, Triple:ReadRTC");
}

static void test_sntp_loop(void) {
    at_controller_process(&at_ctrl);
    Key_Update(key_ui);
}

static void test_sntp_teardown(void) {
    log_i("SNTP Integration Test Teardown.");
}

REGISTER_TEST(SNTP_Integration, "Interactive ESP8266 SNTP Test",
              test_sntp_setup, test_sntp_loop, test_sntp_teardown);

#endif
