#ifndef __Contol_model_H__
#define __Contol_model_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "service/device_manager.h"


// --- Model层提供的接口 ---
void model_init(void);    // 从ROM加载所有设备数据
void model_deinit(void);  // 释放所有加载的数据
int model_get_device_count(void); // 获取设备数量
const device_data_t* model_get_device_by_index(int index); // 根据索引获取设备数据
void model_update_property(const char* device_id, const char* prop_id, property_value_t new_value); // 更新设备属性值
void model_update_device_name(const char* device_id, const char* new_name); // 更新设备名称

//--- 其他辅助函数 --- (测试)
device_data_t* model_get_light_device(void); // 获取模拟灯设备数据


#ifdef __cplusplus
}
#endif

#endif // __Contol_model_H__
