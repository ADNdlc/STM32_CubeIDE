#include "test_config.h"
#if ENABLE_TEST_MQTT

#define LOG_TAG "TEST_MQTT"
#include "Sys.h"
#include "elog.h"
#include "test_framework.h"

// 抽象服务层
#include "gpio_factory.h"
#include "mqtt_factory.h"
#include "mqtt_service/adapters/onenet_adapter.h"
#include "mqtt_service/mqtt_service.h"
#include "wifi_factory.h"
#include "wifi_service/wifi_service.h"
// 交互相关
#include "gpio_key/gpio_key.h"

#define DEFAULT_CLOUD_PRODUCT_ID "SQKg9n0Ii0"
#define DEFAULT_CLOUD_DEVICE_ID "test2"
#define DEFAULT_CLOUD_DEVICE_SECRET                                            \
  "version=2018-10-31&res=products%2FSQKg9n0Ii0%2Fdevices%2Ftest2&et="         \
  "1855499668539&method=md5&sign=%2FHVmg4Xz2RfTRWEu44mApQ%3D%3D"

// 全局实例
static wifi_service_t wifi_svc;
static mqtt_service_t mqtt_svc;
static gpio_key_t *key_ui;
static KeyObserver key_observer;

// MQTT 回调
static void mqtt_event_callback(mqtt_service_t *self,
                                const mqtt_drv_event_t *event,
                                void *user_data) {
  switch (event->type) {
  case MQTT_DRV_EVENT_CONNECTED:
    log_w("MQTT Connected via Service.");
    break;
  case MQTT_DRV_EVENT_DISCONNECTED:
    log_w("MQTT Disconnected via Service.");
    break;
  case MQTT_DRV_EVENT_DATA:
    log_w("MQTT Data Recv via Service on [%s]: %s", event->topic,
          event->payload);
    break;
  }
}

static void on_key_event(gpio_key_t *key, KeyEvent event) {
  switch (event) {
  case KeyEvent_LongPress:
    log_i("Action: Disconnecting MQTT...");
    mqtt_svc_disconnect(&mqtt_svc);
    break;

  case KeyEvent_SinglePress:
    log_i("Action: Connecting MQTT...");
    mqtt_svc_connect(&mqtt_svc);
    break;

  case KeyEvent_DoublePress:
    log_i("Action: Subscribing to '../property/set'...");
    mqtt_svc_subscribe(
        &mqtt_svc, "$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/set",
        0);
    break;

  case KeyEvent_TriplePress:
    log_i("Action: Publishing to 'test/topic'...");
    // 直接发布测试主题
    MQTT_DRV_PUBLISH(mqtt_svc.drv, "test/topic", "Hello from Abstraction!", 0);
    break;

  default:
    break;
  }
}

static void test_mqtt_setup(void) {
  log_i("MQTT Integration Test Setup (Service Layer)...");

  // 1. 获取硬件资源
  gpio_driver_t *key_pin = gpio_driver_get(GPIO_ID_KEY0);
  if (!key_pin) {
    log_e("Key driver missing!");
    return;
  }

  // 2. 初始化驱动和服务 (使用工厂)
  wifi_svc_init(&wifi_svc, wifi_driver_get(WIFI_ID_MAIN));
  wifi_svc_set_mode(&wifi_svc, WIFI_MODE_STATION);
  wifi_svc_connect(&wifi_svc, "test2", "yu778866");
  onenet_config_t config = {
      .product_id = DEFAULT_CLOUD_PRODUCT_ID,
      .device_id = DEFAULT_CLOUD_DEVICE_ID,
      .device_secret = DEFAULT_CLOUD_DEVICE_SECRET,
  };
  onenet_adapter_init(&config);

  mqtt_svc_init(&mqtt_svc, mqtt_driver_get(MQTT_ID_MAIN), &g_onenet_adapter);
  mqtt_svc_register_callback(&mqtt_svc, mqtt_event_callback, NULL);

  // 3. 初始化按键
  key_ui = Key_Create(key_pin, 0);
  key_observer.callback = on_key_event;
  Key_RegisterObserver(key_ui, &key_observer);

  log_i("MQTT Test Ready. Long:MQTTdisConnect, Single:MQTTConnect, Double:Sub, "
        "Triple:Pub");
}

static void test_mqtt_loop(void) {
  wifi_svc_process(&wifi_svc);
  mqtt_svc_process(&mqtt_svc);
  Key_Update(key_ui);
}

static void test_mqtt_teardown(void) {
  log_i("MQTT Integration Test Teardown.");
}

REGISTER_TEST(MQTT_Integration, "Interactive ESP8266 MQTT Test",
              test_mqtt_setup, test_mqtt_loop, test_mqtt_teardown);

#endif
