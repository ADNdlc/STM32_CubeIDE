/*
 * Sensor_Data.c
 *
 *  Created on: May 31, 2025
 *      Author: 12114
 */

#include "Sensor_Data.h"


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


/*	@brief			单独设置数据点的 值
 *	@param point 	数据点
 *	@param ... 		可变参数传入(只能传DataType枚举出的类型)
 *
 *
 */
void Sensor_Set_PointValue(DataPoint* point, ...) {
    va_list args;
    va_start(args, point);

    switch(point->type) {
        case DATA_int:
            point->value.int_value = (int)va_arg(args, int);
            break;
        case DATA_float:
            point->value.float_value = (float)va_arg(args, double);
            break;
        case DATA_double:
            point->value.double_value = (double)va_arg(args, double);
            break;
        case DATA_string:
            // 如果是字符串类型且之前已有字符串，先释放
            if (point->type == DATA_string && point->value.string_value) {
                free((void*)point->value.string_value);
            }
            // 复制新字符串（假设需要独立副本）
            const char* new_str = va_arg(args, const char*);
            point->value.string_value = strdup(new_str);
            break;
    }
    va_end(args);
}

/*===============================================设备函数====================================================*/

/*	@brief				构造并返回一个 设备对象
 *	@param device_id 	设备ID
 *	@param version 		版本
 *	@return				构造的 设备对象
 *
 */
Sensor* Sensor_Init(const char* device_id, const char* version) {
	void * temp=NULL;
	temp = malloc(sizeof(Sensor));
	if(temp!=NULL){
		Sensor* sensor = temp;

		sensor->device_id = device_id;
		sensor->version   = version;
		/*之后添加数据点*/
		sensor->data_points = NULL;
		sensor->count = 0;
		return sensor;
	}
	return temp;
}


/*	@brief				添加数据点到设备
 *	@param set 			设备
 *	@param point 		数据点
 *
 *	@return
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
#if(reallocInfo == 1)
		printf("\r\n\r\nAdd_Point:reallocFail");
#endif
		return false;//如果失败原来的内存块仍然保持不变（并没有释放，相当于没有做本次添加的操作）
	}
}

/*	@brief			更新设备的一个数据点的值(需要指定原数据类型)和时间戳
 *
 *	@param S 		设备对象
 *	@param name 	数据点"标识符"
 *	@param time		毫秒时间戳
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
    			//错误，数据点为设备属性，创建后即不可修改
    			return 2;
    		}
    	}
    }
    if (!found) {
        va_end(args);
        return 1; // 未找到匹配项直接返回
    }

    /*修改数据点数据*/
	S->data_points[location].timestamp = time;	//时间戳
	switch(S->data_points[location].type) {		//值
	     case DATA_int:
	    	S->data_points[location].value.int_value = (int)va_arg(args, int);
	     	break;
	     case DATA_float:
	     	S->data_points[location].value.float_value = (float)va_arg(args, double);
	     	break;
	     case DATA_double:
	     	S->data_points[location].value.double_value = (double)va_arg(args, double);
	     	break;
	     case DATA_string:
	    	 //如果是字符串类型且之前已有字符串，先释放
	    	 if (S->data_points[location].type == DATA_string && S->data_points[location].value.string_value) {
	    		 free((void*)S->data_points[location].value.string_value);
	    	 }
	    	 //复制新字符串
	    	 const char* new_str = va_arg(args, const char*);
	    	 S->data_points[location].value.string_value = strdup(new_str);
	    	 break;
	    }
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
    		if (SenIndex->data_points[i].type == DATA_string && SenIndex->data_points[i].value.string_value){
    			free((void*)SenIndex->data_points[i].value.string_value);
    		}
    	}
    	free(SenIndex->data_points);
    	free(SenIndex);
	}
}

