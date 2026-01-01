#ifndef APPLICATION_HOME_SYSTEM_THING_MODEL_H_
#define APPLICATION_HOME_SYSTEM_THING_MODEL_H_

#include <stdbool.h>
#include <stdint.h>


/**
 * @brief Thing Model Property Types
 */
typedef enum {
  THING_PROP_TYPE_SWITCH = 0, // Boolean
  THING_PROP_TYPE_INT,        // Integer (Slider)
  THING_PROP_TYPE_FLOAT,      // Float
  THING_PROP_TYPE_STRING,     // String (ReadOnly or Input)
  THING_PROP_TYPE_ENUM        // Enumeration
} thing_prop_type_t;

/**
 * @brief Thing Model Property Value
 */
typedef union {
  bool b;
  int32_t i;
  float f;
  const char *s;
} thing_value_t;

/**
 * @brief Thing Model Property Definition
 */
typedef struct {
  const char *id;         // Property ID (e.g., "led0", "temp")
  const char *name;       // Display Name (e.g., "Main Light", "Temperature")
  thing_prop_type_t type; // Data Type
  thing_value_t value;    // Current Value

  // Constraints (optional)
  int32_t min;
  int32_t max;
  const char *unit; // e.g., "C", "%"

  // Flags
  bool readable;
  bool writable;
  bool cloud_sync; // Whether to report to cloud
} thing_property_t;

/**
 * @brief Thing Model Device Definition
 */
typedef struct thing_device_t thing_device_t;

/**
 * @brief Callback for property changes (hardware driver implementation)
 */
typedef bool (*thing_on_prop_set_cb)(thing_device_t *device,
                                     const char *prop_id, thing_value_t value);

struct thing_device_t {
  const char *device_id; // Cloud Device ID
  const char *name;      // Display Name
  const void *icon;      // Pointer to LVGL image source

  thing_property_t *properties;
  uint8_t prop_count;

  thing_on_prop_set_cb on_prop_set;
  void *user_data; // Private driver data
};

/**
 * @brief Initialize the Thing Model Manager
 */
void thing_model_init(void);

/**
 * @brief Register a device with the Thing Model
 */
thing_device_t *thing_model_register(const thing_device_t *template);

/**
 * @brief Update a property value (Entry point for UI or Cloud control)
 * @param source 0 for Local(UI), 1 for Cloud, 2 for Driver
 */
bool thing_model_set_prop(const char *device_id, const char *prop_id,
                          thing_value_t value, int source);

/**
 * @brief Get a device by index
 */
thing_device_t *thing_model_get_device(uint8_t index);

/**
 * @brief Get total device count
 */
uint8_t thing_model_get_count(void);

#endif /* APPLICATION_HOME_SYSTEM_THING_MODEL_H_ */
