#ifndef _UI_COMP_APP_ICON_H
#define _UI_COMP_APP_ICON_H

#include <home.h>

// 创建一个 APP 图标组件
// parent: 父对象
// icon_src: 图片资源
// name: APP 名称
// 返回: 组件容器对象
lv_obj_t *ui_comp_app_icon_create(lv_obj_t *parent, const void *icon_src,
                                  const char *name);

// 辅助: 设置 APP 图标大小配置 (根据行高动态调整字体等)
void ui_comp_app_icon_update_layout(lv_obj_t *comp, lv_coord_t row_height);

#endif
