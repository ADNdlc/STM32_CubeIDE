#include "test_config.h"
#if ENABLE_TEST_MQTT

#define LOG_TAG "TEST_MQTT"
#include "Sys.h"
#include "elog.h"
#include "test_framework.h"

// 驱动相关
#include "esp_8266/at_controller.h"
#include "esp_8266/esp8266_mqtt_driver.h"
#include "esp_8266/esp8266_wifi_driver.h"
#include "gpio_factory.h"
#include "uart_queue/uart_queue.h"
#include "usart_factory.h"
#include "usart_hal/usart_hal.h"

// 交互相关
#include "gpio_key/gpio_key.h"

// 全局实例
static at_controller_t at_ctrl;
static uart_queue_t at_uart_q;
static esp8266_wifi_driver_t wifi_drv;
static esp8266_mqtt_driver_t mqtt_drv;
static gpio_key_t *key_ui;
static KeyObserver key_observer;

// 缓冲区
static uint8_t at_tx_buf[256];
static uint8_t at_rx_buf[512];

#include "sys_config.h"

// MQTT 回调
static void mqtt_event_callback(void *arg, mqtt_drv_event_t *event) {
  switch (event->type) {
  case MQTT_DRV_EVENT_CONNECTED:
    log_i("MQTT Connected.");
    break;
  case MQTT_DRV_EVENT_DISCONNECTED:
    log_w("MQTT Disconnected.");
    break;
  case MQTT_DRV_EVENT_DATA:
    log_i("MQTT Data Recv on [%s]: %s", event->topic, event->payload);
    break;
  }
}

static void on_key_event(gpio_key_t *key, KeyEvent event) {
  switch (event) {
  case KeyEvent_LongPress:
    log_i("Action: Connecting WiFi for MQTT test...");
    WIFI_CONNECT((wifi_driver_t *)&wifi_drv, sys_config_get_wifi_ssid(),
                 sys_config_get_wifi_password());
    break;

  case KeyEvent_SinglePress: {
    log_i("Action: Connecting MQTT...");
    mqtt_driver_conn_params_t params = {0}; // 内容省略
    params.host = "183.230.40.39";          // OneNet example or placeholder
    params.port = 6002;
    params.client_id = "test_client";
    MQTT_DRV_CONNECT((mqtt_driver_t *)&mqtt_drv, &params);
    break;
  }

  case KeyEvent_DoublePress:
    log_i("Action: Subscribing to 'test/topic'...");
    MQTT_DRV_SUBSCRIBE((mqtt_driver_t *)&mqtt_drv, "test/topic", 0);
    break;

  case KeyEvent_TriplePress:
    log_i("Action: Publishing to 'test/topic'...");
    MQTT_DRV_PUBLISH((mqtt_driver_t *)&mqtt_drv, "test/topic",
                     "Hello from STM32!", 0);
    break;

  default:
    break;
  }
}

static void test_mqtt_setup(void) {
  log_i("MQTT Integration Test Setup...");

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
  uart_queue_init(&at_uart_q, hal, at_tx_buf, sizeof(at_tx_buf), at_rx_buf,
                  sizeof(at_rx_buf));
  uart_queue_start_receive(&at_uart_q);
  at_controller_init(&at_ctrl, &at_uart_q, rst_pin);

  // 3. 初始化驱动 (WiFi + MQTT)
  esp8266_wifi_driver_init(&wifi_drv, &at_ctrl);
  WIFI_SET_MODE((wifi_driver_t *)&wifi_drv, WIFI_MODE_STATION);

  esp8266_mqtt_driver_init(&mqtt_drv, &at_ctrl);
  MQTT_DRV_SET_CB((mqtt_driver_t *)&mqtt_drv, mqtt_event_callback, NULL);

  // 4. 初始化按键
  key_ui = Key_Create(key_pin, 0);
  key_observer.callback = on_key_event;
  Key_RegisterObserver(key_ui, &key_observer);

  log_i("MQTT Test Ready. Long:WiFiConnect, Single:MQTTConnect, "
        "Double:Subscribe, Triple:Publish");
}

static void test_mqtt_loop(void) {
  at_controller_process(&at_ctrl);
  Key_Update(key_ui);
}

static void test_mqtt_teardown(void) {
  log_i("MQTT Integration Test Teardown.");
}

REGISTER_TEST(MQTT_Integration, "Interactive ESP8266 MQTT Test",
              test_mqtt_setup, test_mqtt_loop, test_mqtt_teardown);

#endif
