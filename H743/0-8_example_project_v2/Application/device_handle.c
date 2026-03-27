#include "device_handle.h"
#include "gpio_factory.h"
#include "humiture_factory.h"
#include "gpio_key/gpio_key.h"
#include "gpio_led/gpio_led.h"
#include "sys_config.h"
#include "thing_model/thing_model.h"
#include <string.h>


#define LOG_TAG "DEV_HANDLE"
#include "elog.h"

static gpio_key_t *key0 = NULL, *key1 = NULL;
static KeyObserver observer0, observer1;

/* 设备对象 */
// LED对象指针
static gpio_led_t *led0_obj = NULL;
static gpio_led_t *led1_obj = NULL;
static humiture_driver_t *humiture = NULL;
void dev_register_SWITCH(void);
void dev_register_INT(void);

#if 1
static bool device_prop_set_cb(struct thing_device_t *dev, const char *prop_id,
                               thing_value_t value) {
  if (strcmp(dev->device_id, sys_config_get_cloud_device_id()) == 0) {
    if (strcmp(prop_id, "led0") == 0) {
      log_i("Hardware: led0 set to %d", value.b);
      if (led0_obj)
        gpio_led_set(led0_obj, value.b);

      return true;
    } else if (strcmp(prop_id, "led1") == 0) {
      log_i("Hardware: led1 set to %d", value.b);
      if (led1_obj)
        gpio_led_set(led1_obj, value.b);

      return true;
    }
  }
  return false;
}

// 按钮事件回调(本地)
static void key0_event_callback(gpio_key_t *key, KeyEvent event) {
  switch (event) {
  case KeyEvent_SinglePress:
    log_d("Hardware: key0 pressed.");
    thing_model_set_prop(sys_config_get_cloud_device_id(), "led0",
                         (thing_value_t){.b = true}, THING_SOURCE_LOCAL); // 来源:本地
    break;
  case KeyEvent_DoublePress:
    log_d("Hardware: key0 double pressed.");
    thing_model_set_prop(sys_config_get_cloud_device_id(), "led0",
                         (thing_value_t){.b = false}, THING_SOURCE_LOCAL);
    break;
  case KeyEvent_TriplePress:
    log_d("Hardware: key0 triple pressed.");

    break;
  case KeyEvent_LongPress:
    log_d("Hardware: key0 long pressed.");


    break;
  default:
    break;
  }
}
// 按钮事件回调(本地)
static void key1_event_callback(gpio_key_t *key, KeyEvent event) {
  switch (event) {
  case KeyEvent_SinglePress:
    log_d("Hardware: key1 pressed.");
    thing_model_set_prop(sys_config_get_cloud_device_id(), "led1",
                         (thing_value_t){.b = true}, THING_SOURCE_LOCAL);
    break;
  case KeyEvent_DoublePress:
    log_d("Hardware: key1 double pressed.");
    thing_model_set_prop(sys_config_get_cloud_device_id(), "led1",
                         (thing_value_t){.b = false}, THING_SOURCE_LOCAL);
    break;
  case KeyEvent_TriplePress:
    log_d("Hardware: key1 triple pressed.");
    break;
  case KeyEvent_LongPress:
    log_d("Hardware: key1 long pressed.");
    break;
  default:
    break;
  }
}
// 注册设备物模型
void devices_init(void) {
  log_i("Initializing hardware devices for Thing Model...");
  const char *product_id = sys_config_get_cloud_product_id();
  if (product_id == NULL) {
    log_e("Cloud product id is NULL");
    return;
  }
  /* 注册各个类型的设备 */
  dev_register_SWITCH();
  dev_register_INT();

  /* 非物模型设备 */
  // 按键
  gpio_driver_t *driver0 =
      gpio_driver_get(GPIO_ID_KEY0); // 获取硬件驱动
  gpio_driver_t *driver1 =
      gpio_driver_get(GPIO_ID_KEY1);
  key0 = Key_Create(driver0, 0);     // 创建按键
  key1 = Key_Create(driver1, 0);
  observer0.callback = key0_event_callback;
  observer1.callback = key1_event_callback;
  observer0.next = NULL;
  observer1.next = NULL;
  Key_RegisterObserver(key0, &observer0); // 注册事件回调
  Key_RegisterObserver(key1, &observer1);
  // 触发参数
  Key_SetDebounce(key0, 10);
  Key_SetLongPress(key0, 500);
  Key_SetClickTimeout(key0, 200);
  Key_SetDebounce(key1, 10);
  Key_SetLongPress(key1, 500);
  Key_SetClickTimeout(key1, 200);

  log_i("Device registration completed.");
}

void devices_process(void) {
  Key_Update(key0); // 更新按键状态
  Key_Update(key1);
}
#endif

void dev_register_SWITCH(){
	  // 获取GPIO驱动实例
	  gpio_driver_t *led0_driver =
	      gpio_driver_get(GPIO_ID_LED0); // 获取LED0的GPIO驱动，修正为正确的ID
	  gpio_driver_t *led1_driver =
	      gpio_driver_get(GPIO_ID_LED1); // 获取LED1的GPIO驱动，修正为正确的ID

	  // 创建LED对象
	  if (led0_driver != NULL) {
	    led0_obj = gpio_led_create(led0_driver, 0); // 低电平有效
	    log_i("LED0 object %s", led0_obj ? "created" : "creation failed");
	  }

	  if (led1_driver != NULL) {
	    led1_obj = gpio_led_create(led1_driver, 0); // 低电平有效
	    log_i("LED1 object %s", led1_obj ? "created" : "creation failed");
	  }

	  static thing_property_t device_props[] = {
	      {.id = "led0",	// id必须和云平台定义的一样
	       .name = "Switch",
	       .type = THING_PROP_TYPE_SWITCH,
	       .cloud_sync = true},
	      {.id = "led1",
	       .name = "Switch",
	       .type = THING_PROP_TYPE_SWITCH,
	       .cloud_sync = true},
	  };

	  thing_device_t device_tmpl = {.device_id = sys_config_get_cloud_device_id(),
	                                .name = "LEDDev",
	                                .prop_count = 2,
	                                .properties = device_props,
	                                .on_prop_set = device_prop_set_cb};

	  // 3. 注册物模型
	  thing_model_register(&device_tmpl);
}

void dev_register_INT(){


}
