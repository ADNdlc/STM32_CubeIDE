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
  *
  * 此工程调通: FMC(sdram), ltdc显示屏, dma2d, 触摸屏, sdmmc(SD卡), printf(uart1), 事件按钮, fatFS文件系统, lvgl图形界面
  *
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "dma2d.h"
#include "fatfs.h"
#include "ltdc.h"
#include "rtc.h"
#include "sdmmc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#define NDEBUG
#include "string.h"
#include "sys.h"
#include "delay.h"
#include "usart/retarget.h"
//BSP
#include "sdram/W9825G6KH.h"
#include "MALLOC/malloc.h"
#include "rgbLCD/LCD.h"
#include "touch_GT9147/touch_gt9xxx.h"
#include "key/Button_event.h"
//Middlewares
#include "lvgl.h"                // 它为整个LVGL提供了更完整的头文件引用
#include "lv_port_disp.h"        // LVGL的显示支持
#include "lv_port_indev.h"       // LVGL的触屏支持
#include "lv_conf.h"
//APP
#include "SDRAM_test/sdram_test.h"		//SDRAM基本读写测试
#include "RAMspeed_test/speed_test.h"	//melloc测试不同ram区速度
#include "SDCARD_test/SDCARD_test.h"	//SD卡基本读写测试
#include "FATFS_test/FATFS_test.h"		//fatfs文件系统测试
#include "LVGL_test/LVGL_test.h"		//LVGL文件系统测试
//lvgl_app
#include "benchmark/lv_demo_benchmark.h"
#include "ui/Act_Manager.h"		//ui界面


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
char receivData[50] = {0};	//存放串口接收内容(记得初始化)
uint8_t dataReady = 0;		//发送标志位

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */


//重定义'串口事件回调'函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	if(huart == &huart1){
		dataReady = 1;//在主函数里处理发送
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)receivData, 50);//重新开启接收
	}
}

//重定义'定时时间到'
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	static uint16_t Times6 = 0;
	if (htim->Instance == TIM6){
		lv_tick_inc(1);	//给lvgl提供时基
		Times6++;
		if(Times6 >= 500){
			HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);//led闪烁指示
			Times6 = 0;
		}
	}

}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define usart1_echo			1
#define sdram_base_test		0
#define memory_speed_test	0	//malloc
#define fatfs_base_test		1	//直接操作fatfs
#define lvgl_base_test		0	//lvgl基本显示测试
#define benchmark			0	//跑个分
#define lvgl_ui_test		1	//ui测试

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
  delay_init(480);								//自定义延时初始化
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_FMC_Init();
  MX_LTDC_Init();
  MX_DMA2D_Init();
  MX_TIM6_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  MX_RTC_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  Button_Init();								//按钮初始化(这个还没有加入到lvgl组)
  RetargetInit(&huart1);						//绑定printf使用的串口
#if usart1_echo
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)receivData, 50);//开启接收
#endif
#ifndef NDEBUG
  printf("printf init success!!\r\n");
#endif
  SDRAM_InitSequence();							//外部SDRAM初始化

#if  sdram_base_test
  fsmc_sdram_test();			//sdram基本读写测试
#endif

  HAL_Delay(100);								//内存池初始化
  my_mem_init(SRAMIN);							//D1域
  my_mem_init(SRAMEX);							//外部SDRAM
  my_mem_init(SRAM12);							//D2域
  my_mem_init(SRAM4);							//D3域
  my_mem_init(SRAMDTCM);						//DTCM
  my_mem_init(SRAMITCM);						//ITCM
#ifndef NDEBUG
  printf("mallco init success!!\r\n");
#endif
#if  sdram_base_test
  memory_speed_test_all();		//内存速度测试
#endif
/* ================================ fatFS ===================================== */
#if  fatfs_base_test
    fatfs_test();				//文件系统测试
#endif


/* ================================= LVGL初始化 ===================================== */
    lv_init();                             		// LVGL 初始化
    lv_port_disp_init();                   		// 注册LVGL的显示任务
    lv_port_indev_init();                  		// 注册LVGL的触屏检测任务
    HAL_TIM_Base_Start_IT(&htim6);				//开启lvgl时基
/* ================================================================================= */

/* ==================================== ui ========================================= */
#if LV_USE_DEMO_BENCHMARK && benchmark
    lv_demo_benchmark();
#endif

#if lvgl_base_test
    lvgl_basic_test();			//lvgl基本显示测试
#endif
#if lvgl_ui_test
    act_manager_init();			//lvgl_ui
#endif
/* ================================================================================= */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
#if usart1_echo
	if (dataReady) {HAL_UART_Transmit_DMA(&huart1, (uint8_t *) receivData, strlen(receivData));dataReady = 0;}//回显
#endif

	lv_timer_handler();
	HAL_Delay(2);

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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
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

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* DMA2D_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2D_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA2D_IRQn);
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
  while (1)
  {
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
