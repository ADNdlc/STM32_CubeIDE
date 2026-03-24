#include "thing_model.h"
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "THING_MODEL"
#include "elog.h"

#define MAX_THING_DEVICES 16
#define MAX_THING_OBSERVERS 4

/****************
 * 内部变量
 ****************/

static thing_device_t *g_devices[MAX_THING_DEVICES]; // 设备列表
static uint8_t g_device_count = 0;                   // 已注册的设备数

/**
 * @brief 物模型观察者
 *
 */
typedef struct {
  thing_model_event_cb cb; // 事件通知回调
  void *user_data;
} thing_observer_t;

static thing_observer_t g_observers[MAX_THING_OBSERVERS]; // 观察者列表
static uint8_t g_observer_count = 0;                      // 已注册的观察者数

/****************
 * 工具方法
 ****************/
/**
 * @brief 查找设备
 *
 * @param device_id 设备ID
 * @return thing_device_t* 设备
 */
thing_device_t *find_device_by_id(const char *device_id) {
  for (int i = 0; i < g_device_count; i++) {
    if (strcmp(g_devices[i]->device_id, device_id) == 0) {
      return g_devices[i];
    }
  }
  return NULL;
}

/**
 * @brief 查找属性
 *
 * @param dev 设备
 * @param prop_id 属性ID
 * @return thing_property_t* 属性
 */
thing_property_t *find_property_by_id(thing_device_t *dev,
                                      const char *prop_id) {
  for (int i = 0; i < dev->prop_count; i++) {
    if (strcmp(dev->properties[i].id, prop_id) == 0) {
      return &dev->properties[i];
    }
  }
  return NULL;
}

/****************
 * 外部api
 ****************/

void thing_model_init(void) {
  memset(g_devices, 0, sizeof(g_devices));
  memset(g_observers, 0, sizeof(g_observers));
  g_device_count = 0;
  g_observer_count = 0;
  log_i("Thing Model Manager initialized.");
}

/**
 * @brief 添加观察者
 *
 * @param cb 回调函数
 * @param user_data 用户数据
 */
void thing_model_add_observer(thing_model_event_cb cb, void *user_data) {
  if (g_observer_count < MAX_THING_OBSERVERS) {
    g_observers[g_observer_count].cb = cb;
    g_observers[g_observer_count].user_data = user_data;
    g_observer_count++;
  }
}

/**
 * @brief 注册设备到物模型
 * @param tmpl 设备物模型模板
 * @return thing_device_t* 注册的设备
 */
thing_device_t *thing_model_register(const thing_device_t *tmpl) {
  if (g_device_count >= MAX_THING_DEVICES) {
    log_e("Maximum device count reached.");
    return NULL;
  }

  // 分配设备结构体内存
  thing_device_t *dev = (thing_device_t *)malloc(sizeof(thing_device_t));
  if (!dev)
    return NULL;

  // 1. 复制基本结构
  memcpy(dev, tmpl, sizeof(thing_device_t));

  // 2. 深度拷贝关键字符串，防止外部指针失效
  if (tmpl->device_id) {
    dev->device_id = strdup(tmpl->device_id);
  }
  if (tmpl->name) {
    dev->name = strdup(tmpl->name);
  }

  // 3. 深拷贝属性列表
  if (tmpl->prop_count > 0 && tmpl->properties) {
    dev->properties =
        (thing_property_t *)malloc(sizeof(thing_property_t) * tmpl->prop_count);
    if (dev->properties) {
      memcpy(dev->properties, tmpl->properties,
             sizeof(thing_property_t) * tmpl->prop_count);
      
      // 进一步深拷贝属性 ID 和名称
      for(int i = 0; i < tmpl->prop_count; i++) {
        if(tmpl->properties[i].id) dev->properties[i].id = strdup(tmpl->properties[i].id);
        if(tmpl->properties[i].name) dev->properties[i].name = strdup(tmpl->properties[i].name);
      }
    } else {
      free((void*)dev->device_id);
      free((void*)dev->name);
      free(dev);
      return NULL;
    }
  }

  g_devices[g_device_count++] = dev;
  log_i("Registered device: %s (%s)", dev->name, dev->device_id);

  return dev;
}

/**
 * @brief 设置设备属性
 *
 * @param device_id 设备ID
 * @param prop_id 属性ID
 * @param value 属性值
 * @param source 源
 * @return true
 * @return false
 */
bool thing_model_set_prop(const char *device_id, const char *prop_id,
                          thing_value_t value, thing_source_t source) {
  // 1. 寻找目标设备
  thing_device_t *target_dev = NULL;
  target_dev = find_device_by_id(device_id);
  if (!target_dev) {
    log_e("Device not found: %s", device_id);
    return false;
  }

  // 2. 寻找目标属性
  thing_property_t *target_prop = NULL;
  target_prop = find_property_by_id(target_dev, prop_id);
  if (!target_prop) {
    log_e("Property not found: %s.%s", device_id, prop_id);
    return false;
  }

  // 3. 调用目标设备的属性设置回调并传递变更属性值
  if (source != THING_SOURCE_DRV && target_dev->on_prop_set) {
    if (!target_dev->on_prop_set(target_dev, prop_id, value)) {
      log_w("Driver rejected property update: %s.%s", device_id, prop_id);
      return false;
    }
  } else {
    log_d("%s's prop %s updated to %s by source %d", device_id, prop_id, value,
          source);
  }

  // 4. 更新内部属性值和状态
  target_prop->value = value;

  // 如果来源不是云端（即本地 UI 或硬件上报），则标记为“脏”，待后续同步
  if (source != THING_SOURCE_CLOUD && target_prop->cloud_sync) {
    target_prop->is_dirty = true;
  }

  // 5. 通知观察者
  thing_model_event_t evt = {.type = THING_EVENT_PROPERTY_CHANGED,
                             .device_id = device_id,
                             .prop_id = prop_id,
                             .value = value,
                             .source = source};

  for (int k = 0; k < g_observer_count; k++) {
    if (g_observers[k].cb) {
      g_observers[k].cb(&evt, g_observers[k].user_data);
    }
  }

  log_i("[Event] Prop %s.%s updated by source %d", device_id, prop_id, source);

  return true;
}

/**
 * @brief 获取设备
 *
 * @param index 设备索引
 * @return thing_device_t* 设备指针
 */
thing_device_t *thing_model_get_device(uint8_t index) {
  if (index < g_device_count)
    return g_devices[index];
  return NULL;
}

/**
 * @brief 获取已注册的设备数量
 *
 * @return uint8_t 设备数量
 */
uint8_t thing_model_get_count(void) { return g_device_count; }

void thing_model_clear_dirty(const char *device_id, const char *prop_id) {
  thing_device_t *dev = find_device_by_id(device_id);
  if (dev) {
    thing_property_t *prop = find_property_by_id(dev, prop_id);
    if (prop) {
      prop->is_dirty = false;
    }
  }
}
