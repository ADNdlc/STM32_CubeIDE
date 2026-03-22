#ifndef _UI_H
#define _UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/**
 * @brief home
 *
 * 这里作为所有UI程序的入口
 *
 */


void home_init(void); // ui功能初始化(初始化后注册)
void UI_Start(void);  // ui启动

// 资源声明 (已迁移至资源管理器)

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
