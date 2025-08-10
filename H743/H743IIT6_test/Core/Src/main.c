/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "dma2d.h"
#include "ltdc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include"../inc/retarget.h"		//printf函数重映射
#include "../../User/Key/Button_event.h"
#include "../../User/W9825G6KH/W9825G6KH.h"
#include "../../User/RGBLCD/LCD.h"
#include "../../User/RGBLCD/IMG.h"
#include "../../User/TOUCH_GT9147/touch_gt9xxx.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t fps = 0, fps_max = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//重定义'定时器周期回调'函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {	//1S周期回调
	if (htim == &htim2) {
		if (fps_max - fps < -1 || fps_max - fps > 1) {
			fps_max = fps;
		}

		printf("H7");
		fps++;
	}
}

char receivData[50] = { 0 };	//存放接收内容(记得初始化)
uint8_t dataReady;	//发送标志位
//重定义'串口事件回调'函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	if (huart == &huart1) {
		dataReady = 1;	//在主函数里处理发送

		//处理数据...

		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*) receivData, 50);
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);	//关闭DMA接收过半中断
	}
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_FMC_Init();
  MX_LTDC_Init();
  MX_DMA2D_Init();
  /* USER CODE BEGIN 2 */

	RetargetInit(&huart1);	//将printf()函数映射到UART1串口上
	Button_Init();	//初始化按键
	HAL_TIM_Base_Start_IT(&htim2);	//开启定时

	SDRAM_InitSequence();	//W9825G6KH初始化
	gt9xxx_init();


	uint32_t RED=0xF800;  // 定义红色
	uint32_t WHITE=0xFFFF;// 定义白色
	uint32_t 黄色=0xffc0;  // 定义黄色
	uint32_t 蓝色=0x1155;  // 定义蓝色

	/* 防止SDRAM有残留数据 */
	FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer0);
	FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer1);

	// 填充背景色
	FillRect_horizontal(0,   0,   320, 240,  0xFFFF,(uint32_t) LCD_Buffer1);
	// 绘制数据条
	FillRect_horizontal(80,  80,  20,  120,  0x001f,(uint32_t) LCD_Buffer1);
	FillRect_horizontal(120, 100, 20,  100,  0x001f,(uint32_t) LCD_Buffer1);
	FillRect_horizontal(160, 40,  20,  160,  0x001f,(uint32_t) LCD_Buffer1);
	FillRect_horizontal(200, 60,  20,  140,  0x001f,(uint32_t) LCD_Buffer1);
	// 绘制X轴
	FillRect_horizontal(40,  200, 240, 1,    0x0000,(uint32_t) LCD_Buffer1);
	HAL_Delay(1000);

	FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer0);
	FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer1);

	// 填充背景色
	FillRect_vertical(0,   0,   320, 240,  0xFFFF,(uint32_t) LCD_Buffer1);
	// 绘制数据条(竖屏)
	FillRect_vertical(80,  80,  20,  120,  0x001f,(uint32_t) LCD_Buffer1);
	FillRect_vertical(120, 100, 20,  100,  0x001f,(uint32_t) LCD_Buffer1);
	FillRect_vertical(160, 40,  20,  160,  0x001f,(uint32_t) LCD_Buffer1);
	FillRect_vertical(200, 60,  20,  140,  0x001f,(uint32_t) LCD_Buffer1);
	// 绘制X轴
	FillRect_vertical(40,  200, 240, 1,    0x0000,(uint32_t) LCD_Buffer1);
	HAL_Delay(1000);
	FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer0);
	FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer1);

	/* 图片 */
	LTDC_Color_Fill(0,0,368,128,(uint16_t*)gImage_IMG,(uint32_t) LCD_Buffer1);

	HAL_Delay(1500);
	FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer0);
	FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer1);

	/* 方块混合图 */
	//横屏
	FillRect_horizontal(50,50,100,100,RED,(uint32_t) LCD_Buffer0);
	FillRect_horizontal(100,100,100,100,WHITE,(uint32_t) LCD_Buffer1);

	FillRect_horizontal(200,200,100,100,黄色,(uint32_t) LCD_Buffer0);
	FillRect_horizontal(250,250,100,100,蓝色,(uint32_t) LCD_Buffer1);

	//竖屏
	FillRect_vertical(0,0,100,100,WHITE,(uint32_t) LCD_Buffer1);
	FillRect_vertical(50,50,100,100,RED,(uint32_t) LCD_Buffer0);
	FillRect_vertical(0,800-100,100,100,黄色,(uint32_t) LCD_Buffer0);
	FillRect_vertical(50,800-150,100,100,蓝色,(uint32_t) LCD_Buffer1);

	HAL_Delay(1500);
	FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer0);
	FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer1);


	int temp = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
		if (dataReady) {
			// 处理activeBuffer中的数据
			HAL_UART_Transmit_DMA(&huart1, (uint8_t*) receivData,strlen(receivData));
			dataReady = 0;
		}


		for(int i=0;i<3840;i+=10){
			gt9xxx_scan();
			Button_UPDATE();

			/* 横向移动 */
			FillRect_horizontal((50+i)%800,50,100,100,RED,(uint32_t) LCD_Buffer0);
			FillRect_horizontal((200+i)%800,200,100,100,黄色,(uint32_t) LCD_Buffer0);
			temp=(100-i)%800;
			if(temp<0){
				temp=800+temp;//超出范围回到最右边(最大处)
			}
			FillRect_horizontal(temp,100,100,100,WHITE,(uint32_t) LCD_Buffer1);
			temp=(250-i)%800;
			if(temp<0){
				temp=800+temp;////超出范围回到最右边(最大处)
			}
			FillRect_horizontal(temp,250,100,100,蓝色,(uint32_t) LCD_Buffer1);



			/* 竖向移动 */
			FillRect_vertical((0+i)%480,0,100,100,WHITE,(uint32_t) LCD_Buffer1);
			FillRect_vertical((50+i)%480,(800-150),100,100,蓝色,(uint32_t) LCD_Buffer1);

			temp=(50-i)%480;
			if(temp<0){
				temp=480+temp;////超出范围回到最右边(最大处)
			}
			FillRect_vertical(temp,50,100,100,RED,(uint32_t) LCD_Buffer0);
			temp=(0-i)%480;
			if(temp<0){
				temp=480+temp;////超出范围回到最右边(最大处)
			}
			FillRect_vertical(temp,(800-100),100,100,黄色,(uint32_t) LCD_Buffer0);

			HAL_Delay(30);

			FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer0);
			FillRect_horizontal(0,0,800,480,0x0000,(uint32_t) LCD_Buffer1);
		}


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
