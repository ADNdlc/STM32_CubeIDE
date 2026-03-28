#define LOG_TAG "CTRL_CTRL"
#include "Contol_controller.h"
#include "../UI/screens/Contol_view.h"
#include "../device_control.h"
#include "elog.h"
#include "home/System/sys_config.h"
#include "lvgl_resource.h"
#include "thing_model/thing_model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>


#define DISPERSE 1

extern thing_device_t *find_device_by_id(const char *id);
extern thing_property_t *find_property_by_id(thing_device_t *dev,
                                             const char *id);

// 定义一个映射结构体和数组来存储控件
#define MAX_UI_CONTROLS 32
typedef struct {
  char deviceName[MAX_ID_LENGTH];
  char propID[MAX_ID_LENGTH];
  lv_obj_t *obj; // 指向对应的LVGL控件
} ui_control_map_t;

static ui_control_map_t g_ui_map[MAX_UI_CONTROLS];
static uint8_t g_ui_map_count = 0;

// 辅助函数，用于在创建UI时注册控件
void controller_register_ui_control(const char *deviceName, const char *propID,
                                    lv_obj_t *obj) {
  if (g_ui_map_count < MAX_UI_CONTROLS) {
    strncpy(g_ui_map[g_ui_map_count].deviceName, deviceName, MAX_ID_LENGTH);
    strncpy(g_ui_map[g_ui_map_count].propID, propID, MAX_ID_LENGTH);
    g_ui_map[g_ui_map_count].obj = obj;
    log_d("Registered UI control: [%s/%s] -> %p", deviceName, propID, obj);
    g_ui_map_count++;
  } else {
    log_w("UI control map is full! Cannot register [%s/%s]", deviceName, propID);
  }
}

// 注销时清除UI映射表
void controller_clear_ui_map(void) {
  memset(g_ui_map, 0, sizeof(g_ui_map));
  g_ui_map_count = 0;
  log_i("Controller UI map cleared, total 0 controls.");
}

// 辅助函数，用于在回调中查找控件
lv_obj_t *controller_find_ui_control(const char *deviceName, const char *propID) {
  for (int i = 0; i < g_ui_map_count; i++) {
    if (strcmp(g_ui_map[i].deviceName, deviceName) == 0 &&
        strcmp(g_ui_map[i].propID, propID) == 0) {
      return g_ui_map[i].obj;
    }
  }
  return NULL;
}

/**
 * @brief 物模型状态更新回调 (观察者回调)
 */
void ui_state_update_cb(const thing_model_event_t *event, void *user_data) {
  if (event->type != THING_EVENT_PROPERTY_CHANGED)
    return;

  // 仅忽略由本 UI 自身发起的更新，允许本地硬件 (LOCAL) 和驱动同步 (DRV) 更新 UI
  if (event->source == THING_SOURCE_UI) {
    log_v("Ignore self UI change for [%s/%s]", event->device_id, event->prop_id);
    return;
  }

  log_d("Syncing UI for [%s/%s] from source %d", event->device->name,
        event->prop_id, event->source);
  
  // 查找对应的控件 (通过设备名称查找，解决 ID 相同的问题)
  lv_obj_t *target_obj =
      controller_find_ui_control(event->device->name, event->prop_id);
  if (!target_obj) {
    log_v("No UI mapping for [%s/%s], skipping sync.", event->device_id,
          event->prop_id);
    return;
  }

  // 根据控件类型更新其状态
  if (lv_obj_check_type(target_obj, &lv_switch_class)) {
    bool is_checked = lv_obj_has_state(target_obj, LV_STATE_CHECKED);
    if (is_checked != event->value.b) {
      if (event->value.b) {
        lv_obj_add_state(target_obj, LV_STATE_CHECKED);
      } else {
        lv_obj_clear_state(target_obj, LV_STATE_CHECKED);
      }
      log_d("Updated Switch [%s/%s]: %s", event->device_id, event->prop_id,
            event->value.b ? "ON" : "OFF");
    }
  } else if (lv_obj_check_type(target_obj, &lv_slider_class)) {
    int32_t cur_val = lv_slider_get_value(target_obj);
    if (cur_val != event->value.i) {
      lv_slider_set_value(target_obj, event->value.i, LV_ANIM_ON);
      log_d("Updated Slider [%s/%s]: %d", event->device_id, event->prop_id,
            event->value.i);
    }
  } else if (lv_obj_check_type(target_obj, &lv_label_class)) {
    // 处理只读数值显示 (FLOAT 等)
    thing_device_t *dev = event->device; // 直接使用事件中的设备指针
    thing_property_t *prop = find_property_by_id(dev, event->prop_id);
    if (prop) {
      char buf[64];
      if (prop->type == THING_PROP_TYPE_FLOAT) {
        snprintf(buf, sizeof(buf), "%.1f %s", event->value.f,
                 prop->unit ? prop->unit : "");
        lv_label_set_text(target_obj, buf);
        log_d("Updated Label [%s/%s]: %s", event->device_id, event->prop_id,
              buf);
      } else if (prop->type == THING_PROP_TYPE_INT) {
        snprintf(buf, sizeof(buf), "%ld %s", event->value.i,
                 prop->unit ? prop->unit : "");
        lv_label_set_text(target_obj, buf);
        log_d("Updated Label [%s/%s]: %s", event->device_id, event->prop_id,
              buf);
      } else if (prop->type == THING_PROP_TYPE_STRING) {
        lv_label_set_text(target_obj, event->value.s ? event->value.s : "N/A");
        log_d("Updated Label [%s/%s]: %s", event->device_id, event->prop_id,
              event->value.s ? event->value.s : "N/A");
      }
    }
  }
}

// 通用控件事件回调函数 (UI -> Thing Model)
void generic_control_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  control_event_ctx_t *ctx = (control_event_ctx_t *)lv_event_get_user_data(e);

  if (code == LV_EVENT_VALUE_CHANGED) {
    thing_value_t new_value;
    lv_obj_t *target = ctx->target_obj;

    // 根据控件类型获取新值
    if (lv_obj_check_type(target, &lv_switch_class)) {
      new_value.b = lv_obj_has_state(target, LV_STATE_CHECKED);
      log_i("UI Event: Switch [%s/%s] -> %s", ctx->deviceName, ctx->propID,
            new_value.b ? "ON" : "OFF");
    } else if (lv_obj_check_type(target, &lv_slider_class)) {
      new_value.i = lv_slider_get_value(target);
      log_i("UI Event: Slider [%s/%s] -> %d", ctx->deviceName, ctx->propID,
            new_value.i);
    } else {
      log_w("UI Event: Unknown control type for [%s/%s]", ctx->deviceName,
            ctx->propID);
      return;
    }

    // 更新物模型属性 (使用名称更新，来源标记为 UI)
    if (!thing_model_set_prop_by_name(ctx->deviceName, ctx->propID, new_value,
                                     THING_SOURCE_UI)) {
      log_e("Failed to update thing model for [%s/%s]", ctx->deviceName,
            ctx->propID);
    }
  }
}

// 初始化主页Tab
void controller_init_main_tab(lv_obj_t *tab) {
  log_i("Initializing Device Control Main Tab...");

  // 1. 注册 UI 观察者，以便在模型状态变化时自动更新 UI
  thing_model_add_observer(ui_state_update_cb, NULL);

  // 2. 清除之前的映射表（以防万一）
  controller_clear_ui_map();

  // 3. 从 thing_model 获取已注册设备并创建 UI
  uint8_t device_count = thing_model_get_count();
  log_i("Found %d devices in thing model. Generating UI cards...",
        device_count);

  uint32_t dispersed = get_self_settings()->configs[0].i_val;
  log_d("Current dispersed mode: %d", dispersed);

  uint32_t card_index = 0;
  for (uint8_t i = 0; i < device_count; i++) { // 遍历所有设备
    thing_device_t *device = thing_model_get_device(i);
    if (!device)
      continue;

    if (dispersed == UI_COMPACT_MODE) { // 分散控件模式
      log_d("Generating dispersed cards for device: %s", device->device_id);
      for (uint32_t p = 0; p < device->prop_count; p++) {
        uint8_t row = 1 + (card_index / 3);
        uint8_t col = card_index % 3;
        view_create_property_card(tab, device, p, row, col);
        card_index++;
      }
    } else if (dispersed == UI_FULL_MODE) {
      log_d("Creating compact card %d: %s (%s)", i, device->name,
            device->device_id);
      // 布局计算：每行放 3 个卡片，跳过第一行
      uint8_t row = 1 + (i / 3);
      uint8_t col = i % 3;

      // 创建设备控制卡片
      view_create_device_card(tab, device, row, col);
    }
  }
  log_i("Main Tab UI generation complete. Mode: %s",
        dispersed ? "Dispersed" : "Compact");
}

void controller_init_user_tab(lv_obj_t *tab_user) {
  // TODO: 实现用户页面逻辑
}

void controller_init_add_tab(lv_obj_t *tab_add) {
  // TODO: 实现添加设备页面逻辑
}
