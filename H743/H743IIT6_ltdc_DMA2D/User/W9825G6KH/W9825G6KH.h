/*
 * W9825G6KH.h
 *
 *  Created on: Jun 14, 2025
 *      Author: 12114
 */

#ifndef W9825G6KH_W9825G6KH_H_
#define W9825G6KH_W9825G6KH_H_

#include "fmc.h"

/**
* @brief FMC SDRAM 模式配置的寄存器相关定义
*/
//突发长度设置
#define SDRAM_MODEREG_BURST_LENGTH_1 ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2 ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4 ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8 ((uint16_t)0x0004)
// 连续还是间隔模式
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED ((uint16_t)0x0008)
// CL等待几个时钟周期
#define SDRAM_MODEREG_CAS_LATENCY_2 ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3 ((uint16_t)0x0030)
//正常模式
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD ((uint16_t)0x0000)
//设置写操作的模式，可以选择突发模式或者单次写入模式*
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE ((uint16_t)0x0200)

/**
  * @brief  FMC SDRAM 数据基地址(SDRAM Bank1)
  */
#define SDRAM_BANK_ADDR     ((uint32_t)0xC0000000)


void SDRAM_InitSequence(void);
void FMC_SDRAM_WriteBuffer(uint8_t *pBuffer,uint32_t WriteAddr,uint32_t n);
void FMC_SDRAM_ReadBuffer(uint8_t *pBuffer,uint32_t ReadAddr,uint32_t n);

#endif /* W9825G6KH_W9825G6KH_H_ */
