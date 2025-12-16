#ifndef __COLORWHELL_H__
#define __COLORWHELL_H__

#ifdef __cplusplus
extern "C" {
#endif


    void colorwheel_test();

    app_def_t* colorwheel_def_get();
    /**
    * @brief 创建UI (返回屏幕对象)
    */
    lv_obj_t* colorwheel_create_cb(void);
    
    /**
    * @brief 销毁UI
    */
    void colorwheel_destroy_cb(struct activity_t* activity);

    /**
    * @brief 暂停 (当被新活动覆盖时调用)
    */
    void colorwheel_pause_cb(struct activity_t* activity);

    /**
    * @brief 恢复 (当返回到此活动时调用)
    */
    void colorwheel_resume_cb(struct activity_t* activity);

#ifdef __cplusplus
}
#endif

#endif
