#ifndef __File_H__
#define __File_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ui/Act_manager.h"

    void File_test();
    app_def_t* File_def_get();

    lv_obj_t* File_create_cb(void);
    lv_obj_t* File_destroy_cb(struct activity_t* activity);
    lv_obj_t* File_pause_cb(struct activity_t* activity);
    lv_obj_t* File_resume_cb(struct activity_t* activity);

#ifdef __cplusplus
}
#endif

#endif
