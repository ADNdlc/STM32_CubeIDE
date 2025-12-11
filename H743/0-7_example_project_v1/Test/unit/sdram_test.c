/*
 * sdram.c
 *
 *  Created on: Jul 13, 2025
 *      Author: 12114
 */

#include "all_tests_config.h"

#if _sdram_test_
#include "sys.h"
#include "elog.h"
#include "sdram_factory.h"
#include "sdram_driver.h"

#define SDRAM_BANK_ADDR 0xC0000000

// SDRAM内存测试
void sdram_test()
{
	log_i("Starting SDRAM test...");

	// 获取平台驱动实例
	sdram_driver_t *driver = sdram_driver_get(SDRAM_MAIN);
	if (driver == NULL)
	{
		log_e("Failed to get SDRAM driver instance");
		return;
	}

	// 使用通用宏函数来初始化
	int ret = SDRAM_INIT(driver);
	if (ret != 0)
	{
		log_e("Failed to initialize SDRAM device");
		return;
	}

	log_i("SDRAM initialization completed");

	// 运行已存在的测试代码
	__IO uint32_t i = 0;
	__IO uint32_t temp = 0;
	__IO uint32_t sval = 0; // 在地址0读到的数据

	// 每隔16K字节,写入一个数据,总共写入2048个数据,刚好是32M字节
	for (i = 0; i < 32 * 1024 * 1024; i += 16 * 1024)
	{
		*(__IO uint32_t *)(SDRAM_BANK_ADDR + i) = temp;
		temp++;
	}
	log_i("Write test completed");

	// 依次读出之前写入的数据,进行校验
	for (i = 0; i < 32 * 1024 * 1024; i += 16 * 1024)
	{
		temp = *(__IO uint32_t *)(SDRAM_BANK_ADDR + i);
		if (i == 0)
			sval = temp;
		else if (temp <= sval)
			break;																  // 后面读出的数据一定要比第一次读到的数据大.
		log_i("SDRAM Capacity: %dKB", (uint16_t)(temp - sval + 1) * 16); // 打印SDRAM容量
		sys_delay_ms(3);
	}
	log_i("Read test completed");
}

#endif
