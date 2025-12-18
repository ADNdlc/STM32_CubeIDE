#ifndef __Settings_H__
#define __Settings_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "Act_manager.h"

    app_def_t* Settings_def_get();

    lv_obj_t* Settings_create_cb(void);
    lv_obj_t* Settings_destroy_cb(struct activity_t* activity);
    lv_obj_t* Settings_pause_cb(struct activity_t* activity);
    lv_obj_t* Settings_resume_cb(struct activity_t* activity);


#ifdef __cplusplus
}
#endif

#endif
