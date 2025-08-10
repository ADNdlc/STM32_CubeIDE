/*
 * sd_card.c
 *
 *  Created on: Jul 14, 2025
 *      Author: 12114
 */

#include <SDCARD_test/SDCARD_test.h>
#include "usart/retarget.h"


void SDCard_ShowInfo(){
    HAL_SD_CardInfoTypeDef cardInfo;
    HAL_StatusTypeDef res = HAL_SD_GetCardInfo(&hsd1 ,&cardInfo);//获取信息，并存在结构体中
    if (res != HAL_OK){
    	printf("SD_GetCardInfo() error\r\n");
    	return;
    }

    printf("SD_GetCardInfo() Success\r\n");

    printf("Card Type=%lu\r\n", cardInfo.CardType);//类型(容量)

    printf("Card Version=%lu\r\n",cardInfo.CardVersion);//协议版本

    printf("Card Class=%lu\r\n", cardInfo.Class);//卡片类

    printf("Relative Card Address=%lu\r\n",cardInfo.RelCardAdd);//相对卡地址

    printf("Card Speed=%lu\r\n",cardInfo.CardSpeed);//速度

    printf("Block Count=%lu\r\n",cardInfo.BlockNbr);//块容量

    printf("Block Size(Bytes)=%lu\r\n",cardInfo.BlockSize);//块大小(bytes)

    printf("Logic Block Count=%lu\r\n",cardInfo.LogBlockNbr);//逻辑块容量

    printf("Logic Block Size(Bytes)=%lu\r\n",cardInfo.LogBlockSize);//逻辑块大小

    uint32_t capacity_mb = (cardInfo.BlockNbr / 2) / 1024;
    printf("SD Card Capacity(MB)=%lu\r\n", capacity_mb); //容量(转换为MB)
}


void SDCard_EraseBlocks(uint32_t BlockStartAdd,uint32_t BlockEndAdd){
	printf("SDCard_EraseBlocks\r\n");
	HAL_StatusTypeDef res = HAL_SD_Erase(&hsd1,BlockStartAdd,BlockEndAdd);//开始块 //结束块
	if (res == HAL_OK) {
		printf("EraseBlocks Success\r\n");
	}else{
		printf("EraseBlocks Err\r\n");
	}
	HAL_SD_CardStateTypeDef cardState = HAL_SD_GetCardState(&hsd1);	//获取状态
	printf("GetcardState()=%lu\r\n", cardState);						//打印状态(应该是7,编程态)

	while(cardState != HAL_SD_CARD_TRANSFER){	//等待发送状态(操作完成)
		HAL_Delay(1);
		cardState =HAL_SD_GetCardState(&hsd1);
	}
	printf("bocks 0-10 is erased\r\n");
	printf("GetcardState()=%lu\r\n", cardState);//打印状态(应该是4,传输态)
}



void SDCard_TestWrite(){
	printf("SDCard_TestWrite\r\n");
	uint8_t pData[512] = "Hello, welcome to UPC\0";
	uint32_t BlockAdd = 5;
	uint32_t NumberofBLocks = 1;
	uint32_t Timeout = 1000;
	HAL_StatusTypeDef res = HAL_SD_WriteBlocks(&hsd1,pData,BlockAdd,NumberofBLocks,Timeout);
	if (res == HAL_OK) {
		printf("Write to Block 5 is success\r\n");
		printf("String is %s\r\n", pData);
	}else{
		printf("Write to Block 5 is error\r\n");
		printf("HAL State is %d\r\n",res);
	}

}

void SDCard_TestRead(){
	printf("SDCard_TestRead\r\n");
	uint8_t pData[512];
	uint32_t BlockAdd = 5;
	uint32_t NumberofBlocks = 1;
	uint32_t Timeout = 1000;
	HAL_StatusTypeDef res = HAL_SD_ReadBlocks(&hsd1, pData,BlockAdd,NumberofBlocks, Timeout);
	if (res == HAL_OK) {
		printf("Read to Block 5 is success\r\n");
		printf("String is %s\r\n", pData);
	}else{
		printf("Read to Block 5 is error\r\n");
		printf("HAL State is %d\r\n",res);
	}
}

#if myCallback == 1
//全局存放
uint8_t pWrite_buffer[512];
uint8_t pRead_buffer[512] = {0};
uint8_t busy = 0;

void SDCard_TestWrite_DMA(){
	for(uint16_t i = 0; i<=512; i++){

		pWrite_buffer[i] = (i%256);
	}
	uint32_t BlockAdd = 5;
	uint32_t NumberofBLocks = 1;

	while(busy==1){};

	printf("SDCard_TestWrite_DMA\r\n");
	busy = 1;
	printf("writing\r\n");//** 注意别让printf被中断 **//
	HAL_SD_WriteBlocks_DMA(&hsd1,pWrite_buffer,BlockAdd,NumberofBLocks);
}


void SDCard_TestRead_DMA(){
	uint32_t BlockAdd = 5;
	uint32_t NumberofBlocks = 1;

	while(busy==1){};

	printf("SD_Card_TestRead_DMA\r\n");
	busy = 1;
	printf("reading\r\n");//** 注意别让printf被中断 **//
	HAL_SD_ReadBlocks_DMA(&hsd1, pRead_buffer,BlockAdd,NumberofBlocks);


}

uint8_t get_busy(){
	return busy;
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd){
	printf("SD_TxComplete!!!\r\n");

	busy = 0;
}


void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd){
	printf("SD_RxComplete!!!\r\n");

	busy = 0;

	printf("buf[1] = %d,buf[255] = %d,buf[256] = %d,buf[512] = %d\r\n",\
	pRead_buffer[1],pRead_buffer[255],pRead_buffer[256],pRead_buffer[511]);
}

#endif
