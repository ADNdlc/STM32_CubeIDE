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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include"../inc/retarget.h"		//printf函数重映射
#include "../../User/OLED/OLED.h"
#include "../../User/Key/Key.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*实例化两个按钮*/
Button btn1;
Button btn2;

/*点击事件*/
void btn1_single_click(Button* btn) {
    // 单击处理
	printf("btn1_single_click\r\n");
}

void btn1_double_click(Button* btn) {
    // 双击处理
	printf("btn1_double_click\r\n");
}

void btn1_triple_click(Button* btn) {
    // 三击处理
	printf("btn1_triple_click\r\n");
}
void btn1_long_click(Button* btn) {
    // 长按处理
	printf("btn1_long_click\r\n");
}


void btn2_single_click(Button* btn) {
    // 单击处理
	printf("btn2_single_click\r\n");
}

void btn2_double_click(Button* btn) {
    // 双击处理
	printf("btn2_double_click\r\n");
}

void btn2_triple_click(Button* btn) {
    // 三击处理
	printf("btn2_triple_click\r\n");
}
void btn2_long_click(Button* btn) {
    // 长按处理
	printf("btn2_long_click\r\n");
}


uint16_t fps = 0, fps_max = 0;

//重定义'定时器周期回调'函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){	//1S周期回调
	if(htim == &htim2){
		if(fps_max - fps < -1 || fps_max - fps > 1)
		{
			fps_max = fps;
		}
		fps = 0;
	}
}

extern DMA_HandleTypeDef hdma_usart1_rx;//声明外部句柄
char receivData[50] = {0};//存放接收内容(记得初始化)
uint8_t dataReady;//发送标志位


//重定义'串口事件回调'函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	if(huart == &huart1){
		dataReady = 1;//在主函数里处理发送

		//处理数据...

		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)receivData, 50);
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx,DMA_IT_HT);//关闭DMA接收过半中断
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
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */


  RetargetInit(&huart1);//将printf()函数映射到UART1串口上
  OLED_Init();

  //初始化按键
  Button_Init(&btn1,btn1_GPIO_Port,btn1_Pin,GPIO_PIN_RESET);
  btn1.SinglePressHandler = btn1_single_click;
  btn1.DoublePressHandler = btn1_double_click;
  btn1.TriplePressHandler = btn1_triple_click;
  btn1.LongPressHandler = btn1_long_click;

  Button_Init(&btn2,btn2_GPIO_Port,btn2_Pin,GPIO_PIN_RESET);
  btn2.SinglePressHandler = btn2_single_click;
  btn2.DoublePressHandler = btn2_double_click;
  btn2.TriplePressHandler = btn2_triple_click;
  btn2.LongPressHandler = btn2_long_click;



  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)receivData, 50);//开启接收，末参数为最大长度
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx,DMA_IT_HT);//关闭DMA接收过半中断

  HAL_TIM_Base_Start_IT(&htim2);//开启定时
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (dataReady) {
		  // 处理activeBuffer中的数据
		  HAL_UART_Transmit_DMA(&huart1, (uint8_t*)receivData, strlen(receivData));
		  dataReady = 0;
	  }


	  Button_Update(&btn1);
	  Button_Update(&btn2);
	//dosomething...

	//显示帧数


	fps++;//每刷新一帧++

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
