/*
 * Sensor_Data.h
 *
 *  Created on: May 31, 2025
 *      Author: 12114
 */

#ifndef ESP32_AT_SENSOR_DATA_H_
#define ESP32_AT_SENSOR_DATA_H_

//#include "Timestamp.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/*
上报JSON格式
{
	"id":"123","version":"1.0","params":{
		"currentTemperature":{"value":22,"time":1747458287111},
		"currenthumidity":{"value":33,"time":1747458287111},
		"标识符":{"value":<指定的数据类型>,"time":<毫秒时间戳>}
		...(一次可上报多个数据点 中间用','隔开)
	}
}
*/


#define message_reallocInfo		1
#define message_point_type		1

/*=========================================数据点定义=============================================*/
// 数据类型枚举
typedef enum {
    DATA_int,
    DATA_float,
    DATA_double,
    DATA_string
	//...
} DataType;


// 数据点结构 "标识符":{"value":<数据类型>,"time":<毫秒时间戳>}
typedef struct {
    const char* name;      		// 数据点<标识符>(如 "currentTemperature")这是在网站定义的
    DataType type;         		// 此数据点数据<类型>

    union {
        int int_value;
        float float_value;
        double double_value;
        const char* string_value;
        //...
    } value;	//数据点<值>
    uint32_t timestamp;  //<时间戳>(毫秒)
} DataPoint;

/*=========================================传感器设备设备定义=============================================*/

// 传感器数据{ "id":"123","version":"1.0","params":{<...>} }
typedef struct {
    const char* device_id;     // 设备ID
    const char* version;       // 固件版本
    /* 此指针用于访问一个DataPoint数组 */
    DataPoint* data_points;    // 数据点数组指针(一个传感器可能有多个数据点)
    uint8_t count;             // 数据点数量(DataPoint数组大小)
} Sensor;

/*============================================== 数据点函数 ====================================================*/
DataPoint Sensor_Create_Point(const char* name, DataType type);
void Sensor_Set_PointValue(DataPoint* point, ...);

/*=============================================== 设备函数 ====================================================*/
Sensor* Sensor_Init(const char* device_id, const char* version);
bool Sensor_Add_Point(Sensor* SenIndex, DataPoint point);
uint8_t Sensor_Change_PointValue(Sensor* S,const char* m_name, uint32_t time, DataType type, ...);
void Sensor_Data_free(Sensor* SenIndex);

/* ============================================= 序列化器 ===================================================== */
/**
 * @brief 将一个Sensor对象序列化为符合OneNet物模型格式的JSON字符串
 * @param sensor 		指向要序列化的Sensor对象的指针
 * @param buffer 		用于存放生成的JSON字符串的缓冲区
 * @param buffer_size 	缓冲区的最大大小
 * @return 		成功则返回生成的JSON字符串长度，失败则返回-1
 */
int Sensor_Data_to_json_string(const Sensor* sensor, char* buffer, size_t buffer_size);


#endif /* ESP32_AT_SENSOR_DATA_H_ */
