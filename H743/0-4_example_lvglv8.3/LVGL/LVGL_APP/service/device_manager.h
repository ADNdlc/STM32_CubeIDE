#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include <stdint.h>
#include <stdbool.h>
#include "../lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// --- 配置常量 ---
#define MAX_DEVICES                 10  // 系统支持的最大设备数量
#define MAX_PROPERTIES_PER_DEVICE   5   // 每个设备的最大属性数量
#define MAX_ID_LENGTH               32  // ID和名称字符串的最大长度
#define MAX_OBSERVERS               5   // 最大观察者数量

// --- 数据类型定义 ---

// 设备属性的数据类型
    typedef enum {
        PROP_TYPE_SWITCH,    // 开关 (bool)
        PROP_TYPE_SLIDER,    // 滑块 (int32_t, with min/max)
        PROP_TYPE_COLOR,     // 颜色 (lv_color_t)
        PROP_TYPE_READONLY,  // 只读文本 (const char*)
        PROP_TYPE_ENUM,      // 枚举 (int32_t, UI上可以表现为下拉列表)
    } property_type_t;

    // 存储不同类型的值
    typedef union {
        bool b;
        int32_t i;
        lv_color_t c;
        const char* s;
    } property_value_t;

    struct device_data_t; // 前向声明主设备结构体

    /* --- 设备驱动回调函数原型(device_manager调用) --- */

    /**
     * @brief 设置设备属性的回调函数指针类型
     * @param device    指向被操作的设备对象的指针
     * @param prop_id   要设置的属性的ID
     * @param value     要设置的新值
     * @return true 操作成功, false 操作失败
     */
    typedef bool (*device_set_property_cb_t)(struct device_data_t* device, const char* prop_id, property_value_t value);

    /**
     * @brief 获取设备属性的回调函数指针类型（用于主动轮询）
     * @param device    指向被操作的设备对象的指针
     * @param prop_id   要获取的属性的ID
     * @param value     指向property_value_t的指针，用于返回获取到的值
     * @return true 操作成功, false 操作失败
     */
    typedef bool (*device_get_property_cb_t)(struct device_data_t* device, const char* prop_id, property_value_t* value);


    // 设备"属性"结构体
    typedef struct {
        char id[MAX_ID_LENGTH];      // 属性的唯一ID (功能标识符, 如 "power", "brightness")
        property_type_t type;        // 属性类型
        property_value_t value;      // 当前值 (由Manager维护的"唯一真实值")
        int32_t min, max;            // 范围 (如果需要)
    } device_property_t;

    // 设备数据结构体 (这是注册到Manager的模板)
    typedef struct device_data_t {
        // --- 静态信息 ---
        char deviceID[MAX_ID_LENGTH];   // 设备的唯一ID
        char custom_name[MAX_ID_LENGTH];// 用户自定义的名字
        const void* dev_img;            // 指向设备图标的指针(非必要,ui使用)

        // --- 属性 ---
        device_property_t properties[MAX_PROPERTIES_PER_DEVICE]; //属性数组
        uint8_t property_count;         // 属性数量

        // --- 动态行为 (回调函数) ---
        device_set_property_cb_t set_property; // 设置属性的函数指针
        device_get_property_cb_t get_property; // 获取属性的函数指针

        // --- 内部状态 ---
        void* user_data;                // 传递给回调的私有数据 (例如GPIO端口/引脚)
        bool is_active;                 // 标记该设备槽是否被使用
    } device_data_t;


    /* --- 观察者模式回调 --- */

    /**
     * @brief 当设备属性发生变化时，将调用的回调函数原型
     * @param device   状态发生变化的设备
     * @param prop_id  发生变化的属性的ID
     */
    typedef void (*device_change_cb_t)(const device_data_t* device, const char* prop_id);


    /* --- 公共API --- */

    /**
     * @brief 初始化设备管理器
     */
    void DeviceManager_Init(void);

    /**
     * @brief 向管理器注册一个新设备
     * @param device_template 指向一个填充了设备信息的 device_data_t 对象的指针。
     *                        管理器会拷贝这份数据，所以源对象可以是临时变量。
     * @return 成功则返回指向内部设备对象的指针，失败返回NULL
     */
    device_data_t* DeviceManager_RegisterDevice(const device_data_t* device_template);

    /**
     * @brief :注册一个观察者回调函数，当设备属性变化时调用
     * @param callback 要注册的回调函数
     * @return true 注册成功，false 观察者列表已满
     */
    bool DeviceManager_Subscribe(device_change_cb_t callback);

    /**
     * @brief 从管理器中注销一个设备
     * @param deviceID 要注销的设备的ID
     * @return true 注销成功, false 设备未找到
     */
    bool DeviceManager_UnregisterDevice(const char* deviceID);

    /**
     * @brief : 注销一个观察者回调函数
     * @param callback 要注销的回调函数
     */
    void DeviceManager_Unsubscribe(device_change_cb_t callback);


    /**
     * @brief 从管理器中注销一个设备
     * @param deviceID 要注销的设备的ID
     * @return true 注销成功, false 设备未找到
     */
    bool DeviceManager_UnregisterDevice(const char* deviceID);

    /**
     * @brief 更新一个设备的属性值 (所有控制逻辑的入口)
     *        此函数会触发硬件操作，并通知所有观察者,如UI
     * @param deviceID  设备ID
     * @param propID    属性ID
     * @param new_value 新的属性值
     * @return true 更新成功, false 更新失败
     */
    bool DeviceManager_UpdateProperty(const char* deviceID, const char* propID, property_value_t new_value);

    /**
     * @brief 获取当前注册的"设备数量"
     * @return 设备数量
     */
    uint8_t DeviceManager_GetDeviceCount(void);

    /**
     * @brief 根据"索引"获取设备
     * @param index 索引值 (0 to GetDeviceCount()-1)
     * @return 指向设备数据的常量指针，无效索引则返回NULL
     */
    const device_data_t* DeviceManager_GetDeviceByIndex(uint8_t index);

    /**
     * @brief 根据ID获取设备数据
     * @param deviceID 设备ID
     * @return 指向设备数据的常量指针，未找到则返回NULL
     */
    const device_data_t* DeviceManager_GetDeviceByID(const char* deviceID);

    /**
     * @brief : 根据设备ID和属性ID获取"属性对象"
     * @param deviceID 设备ID
     * @param propID 属性ID
     * @return 指向属性对象的常量指针，未找到则返回NULL
     */
    const device_property_t* DeviceManager_GetProperty(const char* deviceID, const char* propID);



#ifdef __cplusplus
}
#endif

#endif // __DEVICE_MANAGER_H__
