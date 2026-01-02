/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define touch_RST_Pin GPIO_PIN_8
#define touch_RST_GPIO_Port GPIOI
#define btn2_Pin GPIO_PIN_13
#define btn2_GPIO_Port GPIOC
#define btn1_Pin GPIO_PIN_2
#define btn1_GPIO_Port GPIOH
#define btn0_Pin GPIO_PIN_3
#define btn0_GPIO_Port GPIOH
#define ESP_RST_Pin GPIO_PIN_5
#define ESP_RST_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOB
#define LED0_Pin GPIO_PIN_1
#define LED0_GPIO_Port GPIOB
#define dht11_D_Pin GPIO_PIN_11
#define dht11_D_GPIO_Port GPIOB
#define touch_SCL_Pin GPIO_PIN_6
#define touch_SCL_GPIO_Port GPIOH
#define touch_INT_Pin GPIO_PIN_7
#define touch_INT_GPIO_Port GPIOH
#define touch_SDA_Pin GPIO_PIN_3
#define touch_SDA_GPIO_Port GPIOI

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
