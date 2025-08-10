/*
 * W9825G6KH.c
 *
 *  Created on: Jun 14, 2025
 *      Author: 12114
 */

#include "W9825G6KH.h"

static FMC_SDRAM_CommandTypeDef Command;   			//定义SDRAM命令结构体
#define sdramHandle				hsdram1  			//使用FMC的哪个Bank,这里SDRAM接在了FMC的SDRAM1上
#define SDRAM_TIMEOUT           ((uint32_t)0xFFFF)  //定义超时时间

/*
 ******************************************************************************************************
 * 函 数 名: SDRAM 初始化序列
 * 功能说明: 完成 SDRAM 序列初始化
 * 形 参: hsdram: SDRAM 句柄
 * Command: 命令结构体指针
 * 返 回 值: None
 ******************************************************************************************************
 *//**
  * @brief  对SDRAM芯片进行初始化配置
  * @param  None.
  * @retval None.
  */
void SDRAM_InitSequence(void)
{
	uint32_t tmpr = 0;

	/* Step 1 ----------------------------------------------------------------*/
	/* 配置命令：开启提供给SDRAM的时钟 */
	Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE; //时钟配置使能
	Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;     //目标SDRAM存储区域
	Command.AutoRefreshNumber = 1;
	Command.ModeRegisterDefinition = 0;
	/* 发送配置命令 */
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	/* Step 2: 延时100us */

	HAL_Delay(1);

	/* Step 3 ----------------------------------------------------------------*/
	/* 配置命令：对所有的bank预充电 */
	Command.CommandMode = FMC_SDRAM_CMD_PALL;    //预充电命令
	Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;    //目标SDRAM存储区域
	Command.AutoRefreshNumber = 1;
	Command.ModeRegisterDefinition = 0;
	/* 发送配置命令 */
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	/* Step 4 ----------------------------------------------------------------*/
	/* 配置命令：自动刷新 */
	Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;  //自动刷新命令
	Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber = 4;  //设置自刷新次数
	Command.ModeRegisterDefinition = 0;
	/* 发送配置命令 */
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	/* Step 5 ----------------------------------------------------------------*/
	/* 设置sdram寄存器配置 */
	tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |  //设置突发长度:1(可以是1/2/4/8)
				   SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |  //设置突发类型:连续(可以是连续/间隔)
				   SDRAM_MODEREG_CAS_LATENCY_2           |   //设置CAS值:3(可以是2/3)
				   SDRAM_MODEREG_OPERATING_MODE_STANDARD |   //设置操作模式:0,标准模式
				   SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;    //设置突发写模式:1,单点访问

	/* 配置命令：设置SDRAM寄存器 */
	Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;  //加载模式寄存器命令
	Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber = 1;
	Command.ModeRegisterDefinition = tmpr;
	/* 发送配置命令 */
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	/* Step 6 ----------------------------------------------------------------*/

	/* 设置自刷新速率 */

	//刷新频率计数器(以SDCLK频率计数),计算方法:
	//COUNT=SDRAM刷新周期/行数-20=SDRAM刷新周期(us)*SDCLK频率(Mhz)/行数
    //我们使用的SDRAM刷新周期为64ms,SDCLK=200/2=100Mhz,行数为8192(2^13).
	//所以,COUNT=64*1000*100/8192-20=761
	HAL_SDRAM_ProgramRefreshRate(&sdramHandle, 761);
}



/**
  * @brief  向sdram写入数据 ，在指定地址(WriteAddr+Bank5_SDRAM_ADDR)开始,连续写入n个字节
  * @param  pBuffer: 指向数据的指针
  * @param  WriteAddr: 要写入的SDRAM内部地址
  * @param  n:要写入的字节数
  * @retval None.
  */

void FMC_SDRAM_WriteBuffer(uint8_t *pBuffer,uint32_t WriteAddr,uint32_t n)
{

  /* 检查SDRAM标志，等待至SDRAM空闲 */
  while ( HAL_SDRAM_GetState(&hsdram1) != HAL_SDRAM_STATE_RESET){}
	for(;n!=0;n--)
	{
		*(__IO uint8_t*)(SDRAM_BANK_ADDR+WriteAddr)=*pBuffer;
		WriteAddr++;
		pBuffer++;
	}
}


/**
  * @brief  从sdram读数据 ，在指定地址((WriteAddr+Bank5_SDRAM_ADDR))开始,连续读出n个字节.
  * @param  pBuffer: 指向数据的指针
  * @param  ReadAddr: 要读出的起始地址
  * @param  n:要读出的字节数
  * @retval None.
  */
void FMC_SDRAM_ReadBuffer(uint8_t *pBuffer,uint32_t ReadAddr,uint32_t n)
{

  /* 检查SDRAM标志，等待至SDRAM空闲 */
  while ( HAL_SDRAM_GetState(&hsdram1) != HAL_SDRAM_STATE_RESET){}
	for(;n!=0;n--)
	{
		*pBuffer++=*(__IO uint8_t*)(SDRAM_BANK_ADDR+ReadAddr);
		ReadAddr++;
	}
}

