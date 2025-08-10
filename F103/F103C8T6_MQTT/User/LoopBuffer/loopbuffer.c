/*
 * loopbuffer.c
 *
 *  Created on: May 21, 2025
 *      Author: 12114
 */

#ifndef LOOPBUFFER_LOOPBUFFER_C_
#define LOOPBUFFER_LOOPBUFFER_C_

///*	@brief	增加读索引
// *	@param	length:	要增加的长度
// *
//*/
//void Add_ReadIndex(uint8_t length){
//	ESP32_AT.readIndex += length;
//	ESP32_AT.readIndex = buff_Size;
//}
//
//
///*	@brief	读取缓冲区第i位数据,超过缓存区长度自动循环
// *	@param:	i要读取的数据索引
// *
//*/
//char Read_buffer(uint8_t i){
//	uint8_t index = i % buff_Size;
//	return ESP32_AT.Reply_Data[index];
//}
//
//
///*	@brief		计算未处理的数据长度
// *	@return		未处理的数据长度
// *	@retval		0					缓冲区为空
// *	@retval		1~buff_Size-1		未处理的数据长度
// *	@retval		buff_Size(缓冲区大小)	缓冲区已满
// *
// */
//uint8_t Get_UNhandled(){
//	return (ESP32_AT.writeIndex + buff_Size - ESP32_AT.readIndex) % buff_Size;
//}
//
//
///*	@brief		算缓冲区剩余空间
// *	@return		剩余空间大小
// *	@retval		0					缓冲区已满
// *	@retval		1~buff_Size-1		剩余空间
// *	@retval		buff_Size(缓冲区大小)	缓冲区为空
// *
// */
//uint8_t Get_Empty(){
//	return buff_Size - Get_UNhandled();
//}
//
//
///*	@brief向缓冲区写入数据
// *	@param data 	要写入的数据指针
// *	@param length 	要写入的数据长度
// *	@return			写入的数据长度
// *
// */
//uint8_t Write_buffer(char *data,uint16_t length){
//	//缓冲区不足
//	if(Get_Empty() < length){
//		return 0;
//	}
//	//使用memcpy函数将数据写入缓冲区
//	if(ESP32_AT.writeIndex + length < buff_Size){
//		memcpy(ESP32_AT.Reply_Data + ESP32_AT.writeIndex + length, data, length);
//		ESP32_AT.writeIndex += length;
//	}
//	else{
//		uint8_t firstLength = buff_Size - ESP32_AT.writeIndex;
//		memcpy(ESP32_AT.Reply_Data + ESP32_AT.writeIndex, data, firstLength);
//		memcpy(ESP32_AT.Reply_Data, data + firstLength, length - firstLength);
//		ESP32_AT.writeIndex = length - firstLength;
//	}
//	return length;
//
//}


#endif /* LOOPBUFFER_LOOPBUFFER_C_ */
