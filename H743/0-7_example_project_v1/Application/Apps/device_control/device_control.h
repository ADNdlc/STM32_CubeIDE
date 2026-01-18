#ifndef APPS_DEVICE_CONTROL_DEVICE_CONTROL_H_
#define APPS_DEVICE_CONTROL_DEVICE_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

// 默认配置值
#define UI_DISPLAY_MODE_KEY 1
#define UI_COMPACT_MODE 1	// 分散显示
#define UI_FULL_MODE 2		// 紧凑显示

#include "app_settings.h"
/**
 * @brief 注册 dvice_control 应用
 */
void device_control_app_register(int page_index);

app_settings_t * get_self_settings(void);

#ifdef __cplusplus
}
#endif

#endif /* APPS_DEVICE_CONTROL_DEVICE_CONTROL_H_ */
