#ifndef __Contol_H__
#define __Contol_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ui/Act_manager.h"

    app_def_t* Contol_def_get();

    lv_obj_t* Contol_create_cb(void);
    void Contol_destroy_cb(struct activity_t* activity);
    void Contol_pause_cb(struct activity_t* activity);
    void Contol_resume_cb(struct activity_t* activity);

#ifdef __cplusplus
}
#endif

#endif
