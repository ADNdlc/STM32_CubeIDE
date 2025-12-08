#include "elog_init.h"

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
  MX_LTDC_Init();
  MX_USART1_UART_Init();
  MX_FMC_Init();
  MX_SDMMC1_SD_Init();
  MX_RTC_Init();
  MX_TIM1_Init();
  MX_DMA2D_Init();
  /* USER CODE BEGIN 2 */

  // 初始化BSP
  bsp_init();
  
  // 初始化并配置EasyLogger
  if (elog_init_and_config() != ELOG_NO_ERR) {
    // 如果日志初始化失败，可以选择在此处理错误
    // 在此示例中我们仅继续执行
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}