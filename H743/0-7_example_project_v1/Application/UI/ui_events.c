#include "System/app_manager.h" // Update path if needed, strictly it's ../System/app_manager.h but include paths might vary
#include "ui.h"


void ui_event_app_icon_click(lv_event_t *e) {
  // The user_data is now expected to be the App Name (char *) or the app_def_t
  // In previous refactor we passed app_def_t*
  // Let's assume user_data is app_def_t*

  // However, since we are moving to a new registry, we should probably pass the
  // Name. For compatibility with the ui_screen_home refactor, let's see what it
  // passes. It passes `const app_def_t *`.

  void *data = lv_event_get_user_data(e);
  if (data) {
    // We can just use the name to start it via Manager
    // Note: We need a common type.
    // If the 'data' is the old app_def_t or new app_def_t, assuming it has a
    // .name field.
    const app_def_t *app = (const app_def_t *)data;
    app_manager_start_app(app->name);
  }
}
