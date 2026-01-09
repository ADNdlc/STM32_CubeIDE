#ifndef __Contol_view_H__
#define __Contol_view_H__

#include "lvgl.h"

#if 1

#ifdef __cplusplus
extern "C" {
#endif

void create_main(lv_obj_t *tabview);
void create_add(lv_obj_t *tabview);
void create_user(lv_obj_t *tabview);

typedef struct thing_device_t thing_device_t;
lv_obj_t *view_create_device_card(lv_obj_t *parent,
                                  const thing_device_t *device, uint8_t row,
                                  uint8_t col);

#ifdef __cplusplus
}
#endif

#endif

#endif
