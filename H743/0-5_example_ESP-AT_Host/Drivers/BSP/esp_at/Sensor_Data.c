/*
 * Sensor_Data.c
 *
 *  Created on: May 31, 2025
 *      Author: 12114
 */
#define USE_MY_MALLOC	0
#if USE_MY_MALLOC
#include "malloc/malloc.h"
#endif
#include "Sensor_Data.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"

/*===============================================数据点函数====================================================*/
/*	@brief			构造并返回一个 数据点对象
 * 					这个函数创建的数据点仅作初始化,最终所有的数据点都会添加到Sensor_Add_Point创建的内存块中,
 * 					此内存块指针交由一个Sensor管理
 *
 *	@param name 	数据点"标识符"
 *	@param type 	数据类型
 *	@return			构造的 数据点对象
 *
 */
DataPoint Sensor_Create_Point(const char* name, DataType type) {
    DataPoint point = {0};
    point.name = name;
    point.type = type;
    point.timestamp = 0;//发生送时再设置时间
    return point;
}

/*	@brief			初始化一个 数据点对象
 *
 *	@param point 	数据点
 *	@param name 	数据点"标识符"
 *	@param type 	数据类型
 *
 */
void Sensor_init_Point(DataPoint* point, const char* name, DataType type) {
    point->name = name;
    point->type = type;
    point->timestamp = 0;//发生送时再设置时间
}

//内部调用
static void Set_PointValue(DataPoint* point, va_list args){
    switch(point->type) {
        case DATA_int:
            point->value.int_value = va_arg(args, int);
            break;
        case DATA_float:
            point->value.float_value = (float)va_arg(args, double);
            break;
        case DATA_double:
            point->value.double_value = va_arg(args, double);
            break;
        case DATA_string:
            // 如果是字符串类型且之前已有字符串，先释放
            if (point->type == DATA_string && point->value.string_value) {
                free((void*)point->value.string_value);
            }
            // 使用strdup复制一个独立副本(这里不释放strdup,而是让string_value指向其申请的内存)
            const char* new_str = va_arg(args, const char*);
            point->value.string_value = strdup(new_str);
            break;
        default:
#if message_point_type
printf("Data Point Type ERR!");
#endif
    	}
}

/*	@brief			单独设置数据点的 值
 *	@param point 	数据点
 *	@param ... 		可变参数传入(只能传DataType枚举出的类型)
 *
 */
void Sensor_Set_PointValue(DataPoint* point, ...) {
    va_list args;
    va_start(args, point);

    Set_PointValue(point, args);

    va_end(args);
}

/*===============================================设备函数====================================================*/

/*	@brief				构造并返回一个"设备对象"
 *	@param Message_ID 	设备ID
 *	@param version 		版本
 *	@return				构造的 设备对象
 *
 */
Sensor* Sensor_Init(const char* Message_ID, const char* version) {
	void * temp=NULL;
	temp = malloc(sizeof(Sensor));//不包含数组大小
	if(temp!=NULL){
		Sensor* sensor = temp;
		sensor->Message_ID = Message_ID;
		sensor->version   = version;
		/* 初始化数据点数组 */
		sensor->data_points = NULL;
		sensor->count = 0;
		return sensor;
	}
	return temp;
}


/*	@brief				添加数据点到设备(重分配内存并拷贝)
 *	@param set 			设备
 *	@param point 		数据点
 *
 *	@return			执行结果
 */
bool Sensor_Add_Point(Sensor* SenIndex, DataPoint point) {
	void * temp=NULL;
	//重新分配存放数据点内存大小(扩大count个DataPoint的大小)
	temp = realloc(SenIndex->data_points, (SenIndex->count+1) * sizeof(DataPoint));
	if(temp!=NULL){
		SenIndex->data_points = temp;//扩大后的数组头地址(可能移动)
		SenIndex->data_points[SenIndex->count] = point;//把数据点添加到数组末尾
		SenIndex->count	++;
		return true;
	}
	else{
#if(message_reallocInfo == 1)
		printf("\r\nAdd_Point: reallocFail!!!\r\n");
#endif
		return false;//如果失败原来的内存块仍然保持不变（并没有释放，相当于没有做本次添加的操作）
	}
}

/*	@brief			更新设备的一个数据点的值(需要指定原数据类型)和时间戳
 *
 *	@param S 		设备对象
 *	@param name 	数据点"标识符"
 *	@param time		时间戳
 *	@param type		数据类型(创建数据点时类型已定不允许需修改类型)
 *	@param ...		数据值
 *
 *	@return			0成功, 1未找到, 2类型不匹配
 *
 */
uint8_t Sensor_Change_PointValue(Sensor* S,const char* m_name, uint32_t time, DataType type, ...){
    va_list args;
    va_start(args, type);

    bool found = false;
    uint8_t location = 0;
    /*根据名字匹配数据点*/
    for(uint8_t i=0;i<S->count;i++){
    	if(!strcmp(S->data_points[i].name,m_name)){
    		location = i;
    		found = true;
    		if(S->data_points[location].type != type){
    			return 2;//错误，数据点为设备属性，创建后即不可修改
    		}
    	}
    }
    if (!found) {
        va_end(args);
        return 1; // 未找到匹配项直接返回
    }
    /* 找到 修改数据点数据 */
	S->data_points[location].timestamp = time;	//时间戳
	Set_PointValue(&(S->data_points[location]), args);

	va_end(args);
	return 0;
}


/*	@brief	销毁一个设备对象和添加到其中的所有数据点
 */
void Sensor_Data_free(Sensor* SenIndex) {
	if (SenIndex) {
        // 释放所有字符串数据点
    	for (int i = 0; i < SenIndex->count; i++) {
    		//如果是字符串数据点且有值,需要释放旧的字符串内存,因为此时值是指针
    		if ((SenIndex->data_points[i].type == DATA_string) && (SenIndex->data_points[i].value.string_value)){
    			free((void*)SenIndex->data_points[i].value.string_value);
    		}
    	}
    	free(SenIndex->data_points);
    	free(SenIndex);
	}
}

/**
 * @brief 	将一个Sensor对象序列化为符合OneNet物模型格式的JSON字符串
 * @param sensor 		指向要序列化的Sensor对象的指针
 * @param buffer 		用于存放生成的JSON字符串的缓冲区
 * @param buffer_size 	缓冲区的最大大小
 * @return 		成功则返回生成的JSON字符串长度，失败则返回-1
 */
int Sensor_Data_to_json_string(const Sensor* sensor, char* buffer, size_t buffer_size){
#if 1
	 if (!sensor || !buffer) return -1;
	    int offset = 0;
	    int written;
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

	    printf("write begin\r\n");

	    // 写入JSON头部
	    written = snprintf(buffer + offset, buffer_size - offset,
	                       "{\"id\":\"%s\",\"version\":\"%s\",\"params\":{",
	                       sensor->Message_ID, sensor->version);
	    if (written < 0 || written >= buffer_size - offset) return -1;
	    offset += written;

	    printf("write head\r\n");

	    // 循环写入所有数据点
	    for (int i = 0; i < sensor->count; ++i) {
	        const DataPoint* p = &sensor->data_points[i];// 写入数据点名称
	        written = snprintf(buffer + offset, buffer_size - offset,
	                           "\"%s\":{\"value\":", p->name);
	        if (written < 0 || written >= buffer_size - offset) return -1;
	        offset += written;

		    printf("write P_count %d\r\n",i);

	        // 根据类型写入数据点的值
	        switch (p->type) {
	            case DATA_int:
	                written = snprintf(buffer + offset, buffer_size - offset, "%d", p->value.int_value);
	                break;
	            case DATA_float:
	                // 注意: snprintf 对浮点数的支持可能需要链接器配置
	                written = snprintf(buffer + offset, buffer_size - offset, "%.2f", p->value.float_value);
	                break;
	            case DATA_double:
	                written = snprintf(buffer + offset, buffer_size - offset, "%f", p->value.double_value);
	                break;
	            case DATA_string:
	                written = snprintf(buffer + offset, buffer_size - offset, "\"%s\"", p->value.string_value);
	                break;
	        }
	        if (written < 0 || written >= buffer_size - offset) return -1;

	        offset += written;
	        // 写入时间戳和结尾
	        written = snprintf(buffer + offset, buffer_size - offset, ",\"time\":%lu000}",p->timestamp);
	        if (written < 0 || written >= buffer_size - offset) return -1;

		    printf("write time\r\n");

	        offset += written;
	        // 如果不是最后一个数据点，则添加逗号
	        if (i < sensor->count - 1) {
	            written = snprintf(buffer + offset, buffer_size - offset, ",");
	            if (written < 0 || written >= buffer_size - offset) return -1;
	            offset += written;
	        }
	    }
	    // 写入JSON尾部
	    written = snprintf(buffer + offset, buffer_size - offset, "}}");
	    if (written < 0 || written >= buffer_size - offset) return -1;

	    printf("write tail\r\n");

	    offset += written;

	    return offset; // 返回总长度
#endif
}


