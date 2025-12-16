#include "System/app_manager.h" // Update path if needed, strictly it's ../System/app_manager.h but include paths might vary
#include "ui.h"


void ui_event_app_icon_click(lv_event_t *e) {

  void *data = lv_event_get_user_data(e); // 获取绑定的 app_def_t 指针
  if (data) {
    const app_def_t *app = (const app_def_t *)data;
    app_manager_start_app(app->name);  // 启动对应的app
  }
}
