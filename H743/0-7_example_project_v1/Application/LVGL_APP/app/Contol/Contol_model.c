#include "Contol_model.h"
#include <stdlib.h>
#include <string.h>

//声明测试图标
LV_IMG_DECLARE(img_light);

static device_property_t light_properties = {   //定义一项属性
    .id = "brightness",
    .type = PROP_TYPE_SLIDER,
    .value.i = 50,  // 初次亮度50%
    .min = 0,
    .max = 100
};
static device_property_t test1 = {   //定义一项属性
    .id = "test1",
    .type = PROP_TYPE_SLIDER,
    .value.i = 50,
    .min = 0,
    .max = 100
};static device_property_t test2 = {
    .id = "test2",
    .type = PROP_TYPE_SLIDER,
    .value.i = 50,
    .min = 0,
    .max = 100
};
static device_data_t light_ex = {0};  // 实际的设备数据

// 初始化模型数据
void model_init(void) {
    // 模拟加载一个灯设备
    strcpy(light_ex.deviceID, "light_001");
    strcpy(light_ex.custom_name, "Living Room Light");
    light_ex.dev_img = &img_light;
    light_ex.property_count = 3;
    
    light_ex.properties[0] = light_properties;
    light_ex.properties[1] = test1;
    light_ex.properties[2] = test2;
}

device_data_t* model_get_light_device(void) {
    return &light_ex;
}
