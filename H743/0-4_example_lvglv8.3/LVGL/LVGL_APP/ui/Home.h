#ifndef __Home_H__
#define __Home_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ui/Act_Manager.h"

    /**
     * @brief 获取接口
     * @return home的接口
     */
    app_def_t* home_def_get();
    uint8_t get_TitleBar_height();
    uint8_t get_TitleBar_opa();

    /**
    * @brief 主页面创建
    */
    lv_obj_t* home_create_cb(void);

    /**
    * @brief 主页面不会被释放(快速返回桌面)
    */
    void home_destroy_cb(struct activity_t* activity);

    /**
    * @brief 主页面不会被释放(快速返回桌面)
    */
    void home_pause_cb(struct activity_t* activity);

    /**
    * @brief 主页面不会被释放(快速返回桌面)
    */
    void home_resume_cb(struct activity_t* activity);



#ifdef __cplusplus
}
#endif

#endif
