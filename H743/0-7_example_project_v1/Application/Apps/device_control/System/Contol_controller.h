#ifndef __Contol_controller_H__
#define __Contol_controller_H__

#include "lvgl.h"
#include "thing_model.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_ID_LENGTH
#define MAX_ID_LENGTH 32
#endif

// 控制事件上下文结构体 (用于存储控件与物模型属性的绑定关系)
typedef struct {
  char deviceID[MAX_ID_LENGTH];
  char propID[MAX_ID_LENGTH];
  lv_obj_t *target_obj; // 指向触发事件的控件
} control_event_ctx_t;

// UI 注册和状态同步接口
void controller_register_ui_control(const char *deviceID, const char *propID,
                                    lv_obj_t *obj);
void controller_clear_ui_map(void);
lv_obj_t *controller_find_ui_control(const char *deviceID, const char *propID);

// 核心初始化函数
void controller_init_main_tab(lv_obj_t *tab_main);
void controller_init_user_tab(lv_obj_t *tab_user);
void controller_init_add_tab(lv_obj_t *tab_add);

// 事件回调
void generic_control_event_cb(lv_event_t *e);
void ui_state_update_cb(const thing_model_event_t *event, void *user_data);

#ifdef __cplusplus
}
#endif

#endif
