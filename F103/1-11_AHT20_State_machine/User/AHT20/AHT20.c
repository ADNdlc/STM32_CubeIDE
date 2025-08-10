/*
 * AHT20.c
 *
 *  Created on: May 4, 2025
 *      Author: 12114
 */

#include "AHT20.h"

#define AHT20_ADDRESS 0x70		//地址只需要按左对齐定义，读写位默认0就可以，HAL自动处理
uint8_t readbuffer[6] = {0};	//6字节测量数据

void AHT20_Init(void){
	uint8_t readbuffer;	//状态信息
	HAL_Delay(40);
	HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDRESS, &readbuffer, 1, 1000);

	if((readbuffer & 0x08) == 0x00){			//判断 状态字的校准使能位Bit[3]是否为1
		uint8_t sendbuffer[3] = {0xBE,0x80,0x00};	//初始化指令

		HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDRESS, sendbuffer, 3, 1000);//发送指令
	}

}

void AHT20_Read(float *Temperature,float *Humidity){	//读取温湿度
	uint8_t sendbuffer[3] = {0xAC,0x80,0x00};	//触发测量指令
	uint8_t readbuffer[6] = {0};				//6字节测量数据
	HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDRESS, sendbuffer, 3, 1000);	//发送指令，开始测量

	HAL_Delay(75);

	HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDRESS, readbuffer, 6, 1000);		//读取数据
	if((readbuffer[0] & 0x80) == 0x00){	//判断忙状态
		uint32_t datatemp = 0;
		datatemp =((uint32_t)readbuffer[3]>>4) + ((uint32_t)readbuffer[2]<<4 ) + ((uint32_t)readbuffer[1]<<12 );//湿度数据拼接
		*Humidity = datatemp*100.0f / (1<<20);

		datatemp =(( (uint32_t)readbuffer[3]&0x0F) << 16) + ((uint32_t)readbuffer[4]<<8) + (uint32_t)readbuffer[5];
		*Temperature = datatemp*200.0f / (1<<20) -50;
	}
}



void AHT20_Start(){
	uint8_t sendbuffer[3] = {0xAC,0x80,0x00};	//触发测量指令
	HAL_I2C_Master_Transmit_DMA(&hi2c1, AHT20_ADDRESS, sendbuffer, 3);	//发送指令，开始测量
}

void AHT20_Get(){
	HAL_I2C_Master_Receive_DMA(&hi2c1, AHT20_ADDRESS, readbuffer, 6);	//读取数据
}

void AHT20_Analysis(float *Temperature,float *Humidity){				//解析数据
	if((readbuffer[0] & 0x80) == 0x00){	//判断忙状态
		uint32_t datatemp = 0;
		datatemp =((uint32_t)readbuffer[3]>>4) + ((uint32_t)readbuffer[2]<<4 ) + ((uint32_t)readbuffer[1]<<12 );//湿度数据拼接
		*Humidity = datatemp*100.0f / (1<<20);

		datatemp =(( (uint32_t)readbuffer[3]&0x0F) << 16) + ((uint32_t)readbuffer[4]<<8) + (uint32_t)readbuffer[5];
		*Temperature = datatemp*200.0f / (1<<20) -50;
	}
}

