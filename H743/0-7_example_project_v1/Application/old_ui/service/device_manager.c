#include "device_manager.h"
#include <string.h>
#include <stdio.h>

// --- 私有全局变量 ---
static device_data_t g_devices[MAX_DEVICES]; // 设备数组
static uint8_t g_device_count = 0;           // 当前设备数量

static device_change_cb_t g_observers[MAX_OBSERVERS]; // 观察者数组
static uint8_t g_observer_count = 0;          // 当前观察者数量


// --- 私有辅助函数 ---
static device_data_t* find_device_by_id(const char* deviceID) {
    for (int i = 0; i < g_device_count; ++i) {
        if (g_devices[i].is_active && strcmp(g_devices[i].deviceID, deviceID) == 0) {
            return &g_devices[i];
        }
    }
    return NULL;
}

static device_property_t* find_property_by_id(device_data_t* device, const char* propID) {
    if (!device) return NULL;
    for (int i = 0; i < device->property_count; ++i) {
        if (strcmp(device->properties[i].id, propID) == 0) {
            return &device->properties[i];
        }
    }
    return NULL;
}

static void notify_observers(const device_data_t* device, const char* prop_id) {
    for (int i = 0; i < g_observer_count; ++i) {
        if (g_observers[i]) {
            g_observers[i](device, prop_id);
        }
    }
}

// --- 公共API实现 ---

void DeviceManager_Init(void) {
    memset(g_devices, 0, sizeof(g_devices));
    g_device_count = 0;

    memset(g_observers, 0, sizeof(g_observers));
    g_observer_count = 0;

    printf("[Info]DeviceManager: Initialized.\r\n");

}

device_data_t* DeviceManager_RegisterDevice(const device_data_t* device_template) {
    if (g_device_count >= MAX_DEVICES) {

        printf("[Err]DeviceManager: Dev list full, unregister '%s'.\r\n", device_template->deviceID);

        return NULL;
    }
    // 拷贝模板数据到设备列表中
    g_devices[g_device_count] = *device_template;
    g_devices[g_device_count].is_active = true;

    printf("[Info]DeviceManager:Device '%s' registered.\r\n", g_devices[g_device_count].custom_name);

    // 返回指向内部存储的设备对象的指针
    return &g_devices[g_device_count++];
}

bool DeviceManager_UpdateProperty(const char* deviceID, const char* propID, property_value_t new_value) {
    device_data_t* device = find_device_by_id(deviceID);
    if (!device) {

        printf("[Err]DeviceManager: Dev '%s' not found for [UpdateProperty].\r\n", deviceID);

        return false;
    }

    device_property_t* prop = find_property_by_id(device, propID);
    if (!prop) {

        printf("[Err]DeviceManager: property '%s' not found in '%s' for [UpdateProperty].\r\n", propID, deviceID);

        return false;
    }

    // 调用设备驱动的回调函数来执行物理操作
    if (device->set_property) {
        // 如果驱动层执行失败，则不更新状态也不通知UI
        if (!device->set_property(device, propID, new_value)) {

            printf("[Err]DeviceManager: [Driver failed] to set prop'%s' for device '%s'.\r\n", propID, deviceID);

            return false;
        }
    }

    // 物理操作成功，更新管理器内部的状态（单一数据源）
    // 注意：这里可以添加值是否真的变化的检查，避免不必要的回调
    prop->value = new_value;

    printf("[Info]DeviceManager: Dev '%s' prop '%s' updated.\r\n", deviceID, propID);

    // 通知所有注册的观察者
    notify_observers(device, propID);

    return true;
}

uint8_t DeviceManager_GetDeviceCount(void) {
    return g_device_count;
}

const device_data_t* DeviceManager_GetDeviceByIndex(uint8_t index) {
    if (index >= g_device_count) {
        return NULL;
    }
    return &g_devices[index];
}

const device_data_t* DeviceManager_GetDeviceByID(const char* deviceID) {
    return find_device_by_id(deviceID);
}

const device_property_t* DeviceManager_GetProperty(const char* deviceID, const char* propID) {
    device_data_t* device = find_device_by_id(deviceID);
    if (!device) return NULL;
    return find_property_by_id(device, propID);
}

bool DeviceManager_Subscribe(device_change_cb_t callback) {
    if (g_observer_count >= MAX_OBSERVERS) {

        printf("[Err]DeviceManager: Observer list full, cannot subscribe.\r\n");

        return false;
    }
    // 避免重复添加
    for (int i = 0; i < g_observer_count; ++i) {
        if (g_observers[i] == callback) return true;
    }
    g_observers[g_observer_count++] = callback;

    printf("[Info]DeviceManager: Observer subscribed. Total: %d\r\n", g_observer_count);

    return true;
}

void DeviceManager_Unsubscribe(device_change_cb_t callback) {
    int found_index = -1;
    for (int i = 0; i < g_observer_count; ++i) {
        if (g_observers[i] == callback) {
            found_index = i;
            break;
        }
    }
    if (found_index != -1) {
        // 移除此观察者
        for (int i = found_index; i < g_observer_count - 1; ++i) {
            g_observers[i] = g_observers[i + 1];
        }
        g_observer_count--;
        g_observers[g_observer_count] = NULL; // 清除最后一个位置
        printf("An observer has unsubscribed. Total: %d\r\n", g_observer_count);
    }
}

bool DeviceManager_UnregisterDevice(const char* deviceID) {
    int found_index = -1;
    for (int i = 0; i < g_device_count; i++) {
        if (g_devices[i].is_active && strcmp(g_devices[i].deviceID, deviceID) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        if (found_index < g_device_count - 1) {
            g_devices[found_index] = g_devices[g_device_count - 1];
        }
        g_device_count--;
        printf("Device '%s' unregistered. Total: %d\r\n", deviceID, g_device_count);
        return true;
    }
    return false;
}

