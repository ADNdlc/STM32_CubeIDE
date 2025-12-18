#ifndef __test1_H__
#define __test1_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "old_ui/ui/Act_manager.h"

    void test1_test();
    app_def_t* test1_def_get();

    lv_obj_t* test1_create_cb(void);
    lv_obj_t* test1_destroy_cb(struct activity_t* activity);
    lv_obj_t* test1_pause_cb(struct activity_t* activity);
    lv_obj_t* test1_resume_cb(struct activity_t* activity);

#ifdef __cplusplus
}
#endif

#endif
