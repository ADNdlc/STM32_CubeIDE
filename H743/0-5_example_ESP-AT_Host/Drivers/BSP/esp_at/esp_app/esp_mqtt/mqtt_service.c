/*
 * mqtt_service.c
 *
 *  Created on: Oct 31, 2025
 *      Author: 12114
 */

#include "mqtt_service.h"
#include "services/device_manager.h"
#include "esp_mqtt.h"
#include "dht11/dht11.h" // 仅用于读取数据

// 当设备状态改变时的回调
static void on_device_state_changed_cb(const device_data_t* device, const char* prop_id) {
    // 业务逻辑：只有灯的开关状态改变时，才主动上报
    if (strcmp(device->deviceID, "light") == 0 && strcmp(prop_id, "power") == 0) {
        printf("[MQTT Service] Light state changed, publishing to cloud...\r\n");

        // 【注意】这里可以调用一个通用的发布函数
        // MQTT_publish_device_data(device);
    }
}

// 将 device_data_t 转换为 Sensor 模型并发布
static void publish_device_data(const device_data_t* device) {
    if (!device) return;

    // 1. 在栈上创建临时的 Sensor 对象
    Sensor sensor_obj;
    sensor_obj.Message_ID = "auto_report"; // or generate a real one
    sensor_obj.version = "1.0";
    sensor_obj.count = device->property_count;

    // 2. 在栈上创建临时的 DataPoint 数组
    DataPoint points[MAX_PROPERTIES_PER_DEVICE];
    sensor_obj.data_points = points;

    // 3. 数据模型转换
    for (int i = 0; i < device->property_count; ++i) {
        const device_property_t* prop = &device->properties[i];
        points[i].name = prop->id;
        points[i].type = DATA_int; // 【简化】假设所有都是int，实际需要映射
        points[i].timestamp = 1755761472; // TODO: get real timestamp

        switch (prop->type) {
            case PROP_TYPE_SWITCH:
                points[i].value.int_value = prop->value.b;
                points[i].type = DATA_int; // onenet often uses int for bool
                break;
            case PROP_TYPE_SLIDER:
                points[i].value.int_value = prop->value.i;
                points[i].type = DATA_int;
                break;
            // ...
        }
    }

    // 4. 调用底层的发布接口
    MQTT_publish_sensor_data(&sensor_obj);
}

void MqttService_Init(void) {
    DeviceManager_Subscribe(on_device_state_changed_cb);
    printf("[MQTT Service] Initialized and subscribed to DeviceManager.\r\n");
}

void MqttService_PeriodicTask(void) {
    // 业务逻辑：周期性地读取DHT11传感器并上报

    // 1. 读取硬件
    if (DHT11_ReadData() == 0) {
        property_value_t temp_val, hum_val;
        temp_val.i = DHT11_GetTemperature();
        hum_val.i = DHT11_GetHumidity();

        // 2. 更新DeviceManager中的状态（这会自动触发UI更新）
        DeviceManager_UpdateProperty("dht11_sensor", "temperature", temp_val);
        DeviceManager_UpdateProperty("dht11_sensor", "humidity", hum_val);

        // 3. 主动上报DHT11的完整状态
        const device_data_t* dht11_dev = DeviceManager_GetDeviceByID("dht11_sensor");
        if(dht11_dev) {
            printf("[MQTT Service] Publishing periodic DHT11 data...\r\n");
            publish_device_data(dht11_dev);
        }
    }
}
