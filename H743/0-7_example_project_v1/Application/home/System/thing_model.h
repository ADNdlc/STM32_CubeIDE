#ifndef APPLICATION_HOME_SYSTEM_THING_MODEL_H_
#define APPLICATION_HOME_SYSTEM_THING_MODEL_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 物模型属性类型定义
 */
typedef enum {
  THING_PROP_TYPE_SWITCH = 0, // Boolean
  THING_PROP_TYPE_INT,        // Integer (Slider)
  THING_PROP_TYPE_FLOAT,      // Float
  THING_PROP_TYPE_STRING,     // String (ReadOnly or Input)
  THING_PROP_TYPE_ENUM        // Enumeration
} thing_prop_type_t;

/**
 * @brief 属性值定义
 */
typedef union {
  bool b;
  int32_t i;
  float f;
  const char *s;
} thing_value_t;

/**
 * @brief 物模型属性定义
 */
typedef struct {
  const char *id;         // 属性识别ID (e.g., "pwr", "temp")
  const char *name;       // 属性显示名称 (e.g., "Main Light", "Temperature")
  thing_prop_type_t type; // 数据类型
  thing_value_t value;    // 当前值

  // 约束条件 (可选)
  int32_t min;
  int32_t max;
  const char *unit; // e.g., "C", "%"

  // 读写类型和云同步
  bool readable;
  bool writable;
  bool cloud_sync; // 是否同步到云端
  bool is_dirty;   // 是否需要同步
} thing_property_t;

typedef struct thing_device_t thing_device_t;

/**
 * @brief 属性设置回调 (硬件驱动实现)
 */
typedef bool (*thing_on_prop_set_cb)(thing_device_t *device,
                                     const char *prop_id, thing_value_t value);

/**
 * @brief 物模型设备定义
 */
struct thing_device_t {
  const char *device_id; // 云端设备识别ID
  const char *name;      // 显示名称
  const void *icon;      // LVGL图标

  thing_property_t *properties; // 属性列表
  uint8_t prop_count;           // 属性数量

  thing_on_prop_set_cb on_prop_set; // 属性设置回调
  void *user_data;                  // 私有数据
};

/**
 * @brief 物模型事件类型
 */
typedef enum {
  THING_EVENT_PROPERTY_CHANGED,  // 属性更新 (事件中包含源信息)
  THING_EVENT_DEVICE_REGISTERED, // 设备注册
} thing_event_type_t;

/**
 * @brief 物模型属性更新来源
 */
typedef enum {
  THING_SOURCE_LOCAL = 0, // UI or Local Logic
  THING_SOURCE_CLOUD = 1, // Cloud Command
  THING_SOURCE_DRV = 2    // Hardware Driver Sync
} thing_source_t;

/**
 * @brief 物模型事件
 */
typedef struct {
  thing_event_type_t type; // 事件类型
  const char *device_id;   // 设备ID
  const char *prop_id;     // 属性ID
  thing_value_t value;     // 属性值
  thing_source_t source;   // 事件来源
} thing_model_event_t;

/**
 * @brief 物模型事件回调原型
 */
typedef void (*thing_model_event_cb)(const thing_model_event_t *event,
                                     void *user_data);

/**
 * @brief 初始化物模型管理器
 */
void thing_model_init(void);

/**
 * @brief 注册一个全局事件观察者
 */
void thing_model_add_observer(thing_model_event_cb cb, void *user_data);

/**
 * @brief 注册一个设备到物模型
 */
thing_device_t *thing_model_register(const thing_device_t *tmpl);

/**
 * @brief 更新一个属性值 (入口点为UI或云控制)
 * @param source 更新来源
 */
bool thing_model_set_prop(const char *device_id, const char *prop_id,
                          thing_value_t value, thing_source_t source);

/**
 * @brief 通过索引获取一个设备
 */
thing_device_t *thing_model_get_device(uint8_t index);

/**
 * @brief 获取设备总数
 */
uint8_t thing_model_get_count(void);

#endif /* APPLICATION_HOME_SYSTEM_THING_MODEL_H_ */

/**
 * @brief 查找属性
 */
thing_property_t *find_property_by_id(thing_device_t *dev, const char *prop_id);

/**
 * @brief 查找设备
 */
thing_device_t *find_device_by_id(const char *device_id);
