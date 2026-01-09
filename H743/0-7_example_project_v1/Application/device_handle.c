#include "gpio_factory.h"
#include "gpio_key/gpio_key.h"
#include "gpio_led/gpio_led.h"
#include "sys_config.h"
#include "thing_model.h"
#include <device_handle.h>
#include <string.h>


#define LOG_TAG "DEV_HANDLE"
#include "elog.h"

static cloud_config_t *config = NULL;
static gpio_key_t *key0 = NULL;
static KeyObserver observer0;

// LED对象指针
static gpio_led_t *led0_obj = NULL;
static gpio_led_t *led1_obj = NULL;

#if 1
static bool device_prop_set_cb(struct thing_device_t *dev, const char *prop_id,
                               thing_value_t value) {
  if (strcmp(dev->device_id, config->device_id) == 0) {
    if (strcmp(prop_id, "led0") == 0) {
      log_i("Hardware: led0 set to %d", value.b);
      if (led0_obj)
    	  led_hal_set_data((led_hal_t *)led0_obj, value.b);
      return true;
    } else if (strcmp(prop_id, "led1") == 0) {
      log_i("Hardware: led1 set to %d", value.b);
      if (led1_obj)
    	  led_hal_set_data((led_hal_t *)led1_obj, value.b);
      return true;
    }
  }
  return false;
}

static void key0_event_callback(gpio_key_t *key, KeyEvent event) {
  switch (event) {
  case KeyEvent_SinglePress:
    log_d("Hardware: key0 pressed.");
    thing_model_set_prop(config->device_id, "led0", (thing_value_t){.b = true},
                         THING_SOURCE_LOCAL);
    break;
  case KeyEvent_DoublePress:
    log_d("Hardware: key0 double pressed.");
    thing_model_set_prop(config->device_id, "led0", (thing_value_t){.b = false},
                         THING_SOURCE_LOCAL);
    break;
  case KeyEvent_TriplePress:
    log_d("Hardware: key0 triple pressed.");
    thing_model_set_prop(config->device_id, "led1", (thing_value_t){.b = true},
                         THING_SOURCE_LOCAL);
    break;
  case KeyEvent_LongPress:
    log_d("Hardware: key0 long pressed.");
    thing_model_set_prop(config->device_id, "led1", (thing_value_t){.b = false},
                         THING_SOURCE_LOCAL);

    break;
  default:
    break;
  }
}

void sys_devices_init(void) {
  log_i("Initializing hardware devices for Thing Model...");
  sys_config_t *sys_config = sys_config_get();
  config = &(sys_config->cloud);

  // 获取GPIO驱动实例
  gpio_driver_t *led0_driver =
      gpio_driver_get(GPIO_LED_0); // 获取LED0的GPIO驱动
  gpio_driver_t *led1_driver =
      gpio_driver_get(GPIO_LED_1); // 获取LED1的GPIO驱动

  // 创建LED对象
  if (led0_driver != NULL) {
    led0_obj = gpio_led_create(led0_driver, 0); // 假设低电平有效
    log_i("LED0 object %s", led0_obj ? "created" : "creation failed");
  }

  if (led1_driver != NULL) {
    led1_obj = gpio_led_create(led1_driver, 0); // 假设低电平有效
    log_i("LED1 object %s", led1_obj ? "created" : "creation failed");
  }

  static thing_property_t device_props[] = {
      {.id = "led0",
       .name = "Switch",
       .type = THING_PROP_TYPE_SWITCH,
       .cloud_sync = true},
      {.id = "led1",
       .name = "Switch",
       .type = THING_PROP_TYPE_SWITCH,
       .cloud_sync = true},
  };

  thing_device_t device_tmpl = {.device_id = config->device_id,
                                .name = "TestDev",
                                .prop_count = 2,
                                .properties = device_props,
                                .on_prop_set = device_prop_set_cb};

  // 3. 注册物模型
  thing_model_register(&device_tmpl);

  // 其他设备
  // 1. 按键
  gpio_driver_t *driver0 = gpio_driver_get(GPIO_BUTTON_KEY0); // 获取硬件端口
  key0 = Key_Create(driver0, 0);                              // 创建按键
  observer0.callback = key0_event_callback;
  observer0.next = NULL;
  Key_RegisterObserver(key0, &observer0); // 注册事件回调
  Key_SetDebounce(key0, 20);              // 20ms去抖动
  Key_SetLongPress(key0, 800);            // 长按检测
  Key_SetClickTimeout(key0, 300);         // 300ms点击超时

  log_i("Device registration completed.");
}

void sys_devices_process(void) {
  Key_Update(key0); // 更新按键状态
}
#endif
