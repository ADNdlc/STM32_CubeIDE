#include "project_cfg.h"
#if USE_Simulator
#include "assets_handler/res_manager.h"
#include "elog.h"
#include "thing_model.h"
#include "virtual_device.h"


#define DEV_ID "test2"

static bool device_prop_set_cb(struct thing_device_t *dev, const char *prop_id,
                               thing_value_t value) {
  if (strcmp(dev->device_id, DEV_ID) == 0) {
    if (strcmp(prop_id, "led0") == 0) {
      log_i("Hardware: led0 set to %d", value.b);
      return true;
    } else if (strcmp(prop_id, "led1") == 0) {
      log_i("Hardware: led1 set to %d", value.b);
      return true;
    }
  }
  return false;
}

// 温湿度传感器设备回调
static bool sensor_prop_set_cb(struct thing_device_t *dev, const char *prop_id,
                               thing_value_t value) {
  if (strcmp(dev->device_id, "sensor001") == 0) {
    if (strcmp(prop_id, "switch") == 0) {
      log_i("Sensor: Power set to %d", value.b);
      return true;
    }
  }
  return false;
}

// 智能插座设备回调
static bool socket_prop_set_cb(struct thing_device_t *dev, const char *prop_id,
                               thing_value_t value) {
  if (strcmp(dev->device_id, "socket001") == 0) {
    if (strcmp(prop_id, "switch") == 0) {
      log_i("Socket: Power set to %d", value.b);
      return true;
    } else if (strcmp(prop_id, "power") == 0) {
      log_i("Socket: Power consumption is %d W", value.i);
      return true;
    } else if (strcmp(prop_id, "voltage") == 0) {
      log_i("Socket: Voltage set to %d V", value.i);
      return true;
    }
  }
  return false;
}

// 智能灯泡设备回调
static bool light_prop_set_cb(struct thing_device_t *dev, const char *prop_id,
                              thing_value_t value) {
  if (strcmp(dev->device_id, "light001") == 0) {
    if (strcmp(prop_id, "switch") == 0) {
      log_i("Light: Power set to %d", value.b);
      return true;
    } else if (strcmp(prop_id, "brightness") == 0) {
      log_i("Light: Brightness set to %d%%", value.i);
      return true;
    } else if (strcmp(prop_id, "color") == 0) {
      log_i("Light: Color set to 0x%x", value.i);
      return true;
    }
  }
  return false;
}

// 全类型设备回调 - 支持所有thing_prop_type_t类型
static bool full_type_prop_set_cb(struct thing_device_t *dev, const char *prop_id,
                                  thing_value_t value) {
  if (strcmp(dev->device_id, "full_type_dev") == 0) {
    if (strcmp(prop_id, "switch_prop") == 0) {
      log_i("FullType: Switch property set to %s", value.b ? "true" : "false");
      return true;
    } else if (strcmp(prop_id, "int_prop") == 0) {
      log_i("FullType: Integer property set to %d", value.i);
      return true;
    } else if (strcmp(prop_id, "float_prop") == 0) {
      log_i("FullType: Float property set to %.2f", value.f);
      return true;
    } else if (strcmp(prop_id, "string_prop") == 0) {
      log_i("FullType: String property set to %s", value.s);
      return true;
    } else if (strcmp(prop_id, "enum_prop") == 0) {
      log_i("FullType: Enum property set to %d", value.i);
      return true;
    }
  }
  return false;
}

void devices_init(void) {
  log_i("Initializing hardware devices for Thing Model...");

  // 原有设备属性
  static thing_property_t device_props[] = {
      {.id = "led0",
       .name = "Light0",
       .type = THING_PROP_TYPE_SWITCH,
       .cloud_sync = true},
      {.id = "led1",
       .name = "Light1",
       .type = THING_PROP_TYPE_SWITCH,
       .cloud_sync = true},
  };

  thing_device_t device_tmpl = {.device_id = DEV_ID,
                                .name = "TestDev",
                                .prop_count = 2,
                                .icon = res_get_src(RES_IMG_IMG_LIGHT),
                                .properties = device_props,
                                .on_prop_set = device_prop_set_cb};

  // 3. 注册物模型
  thing_model_register(&device_tmpl);

  // 温湿度传感器设备
  static thing_property_t sensor_props[] = {
      {.id = "switch",
       .name = "Power Switch",
       .type = THING_PROP_TYPE_SWITCH,
       .cloud_sync = true},
      {.id = "temperature",
       .name = "Temperature",
       .type = THING_PROP_TYPE_FLOAT,
       .unit = "C",
       .min = -40,
       .max = 80,
       .cloud_sync = true},
      {.id = "humidity",
       .name = "Humidity",
       .type = THING_PROP_TYPE_FLOAT,
       .unit = "%",
       .min = 0,
       .max = 100,
       .cloud_sync = true},
  };

  thing_device_t sensor_tmpl = {.device_id = "sensor001",
                                .name = "Temperature and Humidity Sensor",
                                .prop_count = 3,
                                .icon = res_get_src(RES_IMG_ICON_CONTROL),
                                .properties = sensor_props,
                                .on_prop_set = sensor_prop_set_cb};

  thing_model_register(&sensor_tmpl);

  // 智能插座设备
  static thing_property_t socket_props[] = {
      {.id = "switch",
       .name = "Power Switch",
       .type = THING_PROP_TYPE_SWITCH,
       .cloud_sync = true},
      {.id = "power",
       .name = "Power Consumption",
       .type = THING_PROP_TYPE_INT,
       .unit = "W",
       .min = 0,
       .max = 3500,
       .cloud_sync = true},
      {.id = "voltage",
       .name = "Voltage",
       .type = THING_PROP_TYPE_INT,
       .unit = "V",
       .min = 0,
       .max = 380,
       .cloud_sync = true},
      {.id = "electricity",
       .name = "Electricity",
       .type = THING_PROP_TYPE_FLOAT,
       .unit = "A",
       .min = 0.0,
       .max = 16.0,
       .cloud_sync = true},
  };

  thing_device_t socket_tmpl = {.device_id = "socket001",
                                .name = "Smart Socket",
                                .prop_count = 4,
                                .icon = res_get_src(RES_IMG_ICON_BRIGHT),
                                .properties = socket_props,
                                .on_prop_set = socket_prop_set_cb};

  thing_model_register(&socket_tmpl);

  // 智能灯泡设备
  static thing_property_t light_props[] = {
      {.id = "switch",
       .name = "Power Switch",
       .type = THING_PROP_TYPE_SWITCH,
       .cloud_sync = true},
      {.id = "brightness",
       .name = "Brightness",
       .type = THING_PROP_TYPE_INT,
       .unit = "%",
       .min = 0,
       .max = 100,
       .cloud_sync = true},
      {.id = "color",
       .name = "Color",
       .type = THING_PROP_TYPE_INT,
       .min = 0,
       .max = 0xFFFFFF,
       .cloud_sync = true},
  };

  thing_device_t light_tmpl = {.device_id = "light001",
                               .name = "Smart Light Bulb",
                               .prop_count = 3,
                               .icon = res_get_src(RES_IMG_IMG_LIGHT),
                               .properties = light_props,
                               .on_prop_set = light_prop_set_cb};

  thing_model_register(&light_tmpl);

  // 全类型设备 - 包含所有thing_prop_type_t类型的属性
  static thing_property_t full_type_props[] = {
      {.id = "switch_prop",
       .name = "Switch Property",
       .type = THING_PROP_TYPE_SWITCH,
       .cloud_sync = true},
      {.id = "int_prop",
       .name = "Integer Property",
       .type = THING_PROP_TYPE_INT,
       .unit = "",
       .min = 0,
       .max = 100,
       .cloud_sync = true},
      {.id = "float_prop",
       .name = "Float Property",
       .type = THING_PROP_TYPE_FLOAT,
       .unit = "",
       .min = 0.0,
       .max = 100.0,
       .cloud_sync = true},
      {.id = "string_prop",
       .name = "String Property",
       .type = THING_PROP_TYPE_STRING,
       .max = 64,
       .cloud_sync = true},
      {.id = "enum_prop",
       .name = "Enum Property",
       .type = THING_PROP_TYPE_ENUM,
       .unit = "",
       .min = 0,
       .max = 4,  // 枚举值范围 0~4
       .cloud_sync = true},
  };

  thing_device_t full_type_tmpl = {.device_id = "full_type_dev",
                                   .name = "Full Type Virtual Device",
                                   .prop_count = 5,  // 包含全部5种类型
                                   .icon = res_get_src(RES_IMG_ICON_BRIGHT),
                                   .properties = full_type_props,
                                   .on_prop_set = full_type_prop_set_cb};

  thing_model_register(&full_type_tmpl);

  log_i("Device registration completed.");
}

#endif