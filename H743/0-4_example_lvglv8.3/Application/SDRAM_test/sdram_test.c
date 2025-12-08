/*
 * sdram.c
 *
 *  Created on: Jul 13, 2025
 *      Author: 12114
 */

#include "sdram_test.h"

//SDRAM内存测试
void fsmc_sdram_test() {
	__IO uint32_t i = 0;
	__IO uint32_t temp = 0;
	__IO uint32_t sval = 0;	//在地址0读到的数据

	//每隔16K字节,写入一个数据,总共写入2048个数据,刚好是32M字节
	for (i = 0; i < 32 * 1024 * 1024; i += 16 * 1024) {
		*(__IO uint32_t*) (SDRAM_BANK_ADDR + i) = temp;
		temp++;
	}
	//依次读出之前写入的数据,进行校验
	for (i = 0; i < 32 * 1024 * 1024; i += 16 * 1024) {
		temp = *(__IO uint32_t*) (SDRAM_BANK_ADDR + i);
		if (i == 0)
			sval = temp;
		else if (temp <= sval)
			break;	//后面读出的数据一定要比第一次读到的数据大.
		printf("SDRAM Capacity:%dKB\r\n", (uint16_t) (temp - sval + 1) * 16);//打印SDRAM容量
	}
}

