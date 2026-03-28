#include "device_handle.h"
#include "Sys.h"
#include "gpio_factory.h"
#include "gpio_key/gpio_key.h"
#include "gpio_led/gpio_led.h"
#include "humiture_factory.h"
#include "illuminance_factory.h"
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
// 光照传感器对象指针
static illuminance_driver_t *illuminance_sensor = NULL;
// 传感器读数缓存
static int32_t temperature = 0; // 温度 (0.1°C)
static int32_t humidity = 0;    // 湿度 (0.1%)
static int32_t illuminance = 0; // 照度 (Lux)
// 光照阈值滤波计数器(滞回滤波)
static uint8_t illuminance_low_count = 0;
#define ILLUMINANCE_THRESHOLD_LOW 70   // 开灯阈值(Lux)
#define ILLUMINANCE_THRESHOLD_HIGH 100 // 关灯阈值(Lux)
#define ILLUMINANCE_FILTER_COUNT 3     // 连续检测次数

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
                         (thing_value_t){.b = true},
                         THING_SOURCE_LOCAL); // 来源:本地
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
  gpio_driver_t *driver0 = gpio_driver_get(GPIO_ID_KEY0); // 获取硬件驱动
  gpio_driver_t *driver1 = gpio_driver_get(GPIO_ID_KEY1);
  key0 = Key_Create(driver0, 0); // 创建按键
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
  // 更新按键状态
  Key_Update(key0);
  Key_Update(key1);

  // 周期性采集传感器数据
  static uint32_t last_sample_time = 0;
  uint32_t current_time = sys_get_systick_ms();
  if (current_time - last_sample_time >= 2000) { // 每2秒采样一次
    last_sample_time = current_time;

    // 读取温湿度
    float temp_f, humi_f;
    if (humiture != NULL &&
        HUMITURE_READ_FLOAT(humiture, &temp_f, &humi_f) == 0) {
      log_i("Humiture: %.1f°C, %.1f%%", temp_f, humi_f);
      // 转换为int32 (乘以10保存0.1度/0.1%精度)
      temperature = (int32_t)(temp_f);
      humidity = (int32_t)(humi_f);

      // 更新物模型
      thing_value_t value = {.i = temperature};
      thing_model_set_prop_by_name("HumitureDev", "temperature", value,
                                   THING_SOURCE_DRV);

      value.i = humidity;
      thing_model_set_prop_by_name("HumitureDev", "humidity", value,
                                   THING_SOURCE_DRV);
    }

    log_d("Temp: %d.%dC, Humi: %d.%d%%", temperature / 10,
          abs(temperature % 10), humidity / 10, abs(humidity % 10));

    // 读取光照
    float lux_f;
    if (illuminance_sensor != NULL &&
        ILLUMINANCE_READ_LUX(illuminance_sensor, &lux_f) == 0) {
      log_i("Illuminance: %.1f Lux", lux_f);
      illuminance = (int32_t)lux_f;

      // 更新物模型(本地使用name调用)
      thing_value_t value = {.i = illuminance};
      thing_model_set_prop_by_name("LuxDev", "illuminance", value,
                                   THING_SOURCE_DRV);

      log_d("Illuminance: %d Lux", illuminance);

      // 光照阈值判断与LED控制（带滤波）
      if (illuminance < ILLUMINANCE_THRESHOLD_LOW) {
        if (illuminance_low_count < ILLUMINANCE_FILTER_COUNT) {
          illuminance_low_count++;
        }
      } else if (illuminance > ILLUMINANCE_THRESHOLD_HIGH) {
        illuminance_low_count = 0;
      }

      // 通过物模型控制LED
      bool should_turn_on = (illuminance_low_count >= ILLUMINANCE_FILTER_COUNT);
      thing_value_t led_value = {.b = should_turn_on};
      thing_model_set_prop_by_name("LEDDev", "led0", led_value,
                                   THING_SOURCE_LOCAL);
    }
  }
}
#endif

void dev_register_SWITCH() {
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
      {
          .id = "led0", // id必须和云平台定义的一样
          .name = "Switch",
          .type = THING_PROP_TYPE_SWITCH,
          .cloud_sync = true,
          .writable = true,
          .readable = true,
      },
      {
          .id = "led1",
          .name = "Switch",
          .type = THING_PROP_TYPE_SWITCH,
          .cloud_sync = true,
          .writable = true,
		  .readable = true,
      },
  };

  thing_device_t device_tmpl = {.device_id = sys_config_get_cloud_device_id(),
                                .name = "LEDDev",
                                .prop_count = 2,
                                .properties = device_props,
                                .on_prop_set = device_prop_set_cb};

  // 3. 注册物模型
  thing_model_register(&device_tmpl);
}

void dev_register_INT() {
  // 初始化温湿度传感器
  humiture = humiture_driver_get(TH_SENSOR_ID_AMBIENT);
  if (humiture != NULL && humiture->ops->init(humiture) == 0) {
    log_i("Humiture sensor initialized");
  } else {
    log_e("Failed to initialize humiture sensor");
  }

  // 初始化光照传感器
  illuminance_sensor = illuminance_driver_get(LIGHT_SENSOR_ID_AMBIENT);
  if (illuminance_sensor != NULL &&
      illuminance_sensor->ops->init(illuminance_sensor) == 0) {
    log_i("Illuminance sensor initialized");
  } else {
    log_e("Failed to initialize illuminance sensor");
  }

  // 定义温湿度属性
  static thing_property_t sensor_props[] = {
      {.id = "temperature",
       .name = "Temperature",
       .type = THING_PROP_TYPE_INT,
       .cloud_sync = true,
       .writable = false,
       .readable = true,
       .unit = "0.1C"},
      {.id = "humidity",
       .name = "Humidity",
       .type = THING_PROP_TYPE_INT,
       .cloud_sync = true,
       .writable = false,
       .readable = true,
       .unit = "0.1%"},
  };

  // 创建温湿度设备模板
  thing_device_t sensor_tmpl = {.device_id = sys_config_get_cloud_device_id(),
                                .name = "HumitureDev",
                                .prop_count = 2,
                                .properties = sensor_props};

  // 注册温湿度设备
  thing_model_register(&sensor_tmpl);
  log_i("Sensor device registered: %s", sensor_tmpl.name);

  // 定义光照属性
  static thing_property_t light_props[] = {{.id = "illuminance",
                                            .name = "Illuminance",
                                            .type = THING_PROP_TYPE_INT,
                                            .cloud_sync = true,
                                            .writable = false,
                                            .readable = true,
                                            .unit = "Lux"}};

  // 创建光照设备模板
  thing_device_t light_tmpl = {.device_id = sys_config_get_cloud_device_id(),
                               .name = "LuxDev",
                               .prop_count = 1,
                               .properties = light_props};

  // 注册光照设备
  thing_model_register(&light_tmpl);
  log_i("Light sensor device registered: %s", light_tmpl.name);
}
