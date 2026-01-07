#include "Contol_controller.h"
#include "Contol_view.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>


// 定义一个映射结构体和数组来存储控件
#define MAX_UI_CONTROLS 32
typedef struct {
  char deviceID[MAX_ID_LENGTH];
  char propID[MAX_ID_LENGTH];
  lv_obj_t *obj; // 指向对应的LVGL控件
} ui_control_map_t;
static ui_control_map_t g_ui_map[MAX_UI_CONTROLS];
static uint8_t g_ui_map_count = 0;

// 辅助函数，用于在创建UI时注册控件
void controller_register_ui_control(const char *deviceID, const char *propID,
                                    lv_obj_t *obj) {
  if (g_ui_map_count < MAX_UI_CONTROLS) {
    strncpy(g_ui_map[g_ui_map_count].deviceID, deviceID, MAX_ID_LENGTH);
    strncpy(g_ui_map[g_ui_map_count].propID, propID, MAX_ID_LENGTH);
    g_ui_map[g_ui_map_count].obj = obj;
    g_ui_map_count++;
  }
}

// 注销时清除UI映射表
void controller_clear_ui_map(void) {
  memset(g_ui_map, 0, sizeof(g_ui_map));
  g_ui_map_count = 0;
  printf("Controller UI map cleared.\r\n");
}

// 辅助函数，用于在回调中查找控件
lv_obj_t *controller_find_ui_control(const char *deviceID, const char *propID) {
  for (int i = 0; i < g_ui_map_count; i++) {
    if (strcmp(g_ui_map[i].deviceID, deviceID) == 0 &&
        strcmp(g_ui_map[i].propID, propID) == 0) {
      return g_ui_map[i].obj;
    }
  }
  return NULL;
}

// 设备ui状态更新回调函数
void ui_state_update_cb(const device_data_t *device, const char *prop_id) {
  printf("[Info]Contol_controller: %s/%s ui update \r\n", device->deviceID,
         prop_id);

  // 查找对应的控件
  lv_obj_t *target_obj = controller_find_ui_control(device->deviceID, prop_id);
  if (!target_obj) {
    printf("[Err]Contol_controller: no target %s/%s for [ui_update_cb]\r\n",
           device->deviceID, prop_id);
    return;
  }

  // 获取最新的属性值
  const device_property_t *prop =
      DeviceManager_GetProperty(device->deviceID, prop_id);
  if (!prop)
    return;

  // 根据控件类型更新其状态
  if (lv_obj_get_class(target_obj) == &lv_switch_class) {
    if (prop->value.b) {
      lv_obj_add_state(target_obj, LV_STATE_CHECKED);
    } else {
      lv_obj_clear_state(target_obj, LV_STATE_CHECKED);
    }
  } else if (lv_obj_get_class(target_obj) == &lv_slider_class) {
    lv_slider_set_value(target_obj, prop->value.i, LV_ANIM_ON);
  }
  // ... 其他控件类型
}

// 通用控件事件回调函数
void generic_control_event_cb(lv_event_t *e) {

  lv_event_code_t code = lv_event_get_code(e);
  // 从事件中获取上下文
  control_event_ctx_t *ctx = (control_event_ctx_t *)lv_event_get_user_data(e);
  printf("[Info]generic_control_event_cb: [Dev='%s'] [Prop='%s'] update\r\n",
         ctx->deviceID, ctx->propID);

  if (code == LV_EVENT_VALUE_CHANGED) {

    property_value_t new_value;
    lv_obj_t *target = ctx->target_obj;

    // 根据控件类型获取新值
    if (lv_obj_get_class(target) == &lv_switch_class) {
      new_value.b = lv_obj_has_state(target, LV_STATE_CHECKED);
    } else if (lv_obj_get_class(target) == &lv_slider_class) {
      new_value.i = lv_slider_get_value(target);

    } else {
      return; // 其他控件类型
    }

    // 通过DeviceManager更新属性
    DeviceManager_UpdateProperty(ctx->deviceID, ctx->propID, new_value);
  }
}

extern void device_manager_state_sync(const device_data_t *device,
                                      const char *prop_id);

// 初始化主页Tab
void controller_init_main_tab(lv_obj_t *tab) {

  // ui需要监控设备状态变化
  DeviceManager_Subscribe(ui_state_update_cb);
  DeviceManager_Subscribe(device_manager_state_sync);

  // 2. 从DeviceManager获取设备并创建UI
  uint8_t device_count = DeviceManager_GetDeviceCount();
  for (uint8_t i = 0; i < device_count; i++) {
    const device_data_t *device = DeviceManager_GetDeviceByIndex(i);
    if (device) {
      // 布局：每行放3个卡片
      uint8_t row = 1 + (i / 3); // 从网格的第1行开始(0行空出)
      uint8_t col = i % 3;

      // 调用视图函数创建卡片，并传入布局信息
      view_create_device_card(tab, device, row, col);
    }
  }
}

void controller_init_user_tab(lv_obj_t *tab_user) {}

void controller_init_add_tab(lv_obj_t *tab_add) {}
