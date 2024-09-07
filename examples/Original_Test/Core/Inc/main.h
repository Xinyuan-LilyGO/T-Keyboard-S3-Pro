/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32g0xx_hal.h"

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
#define KEY3_Pin GPIO_PIN_15
#define KEY3_GPIO_Port GPIOC
#define KEY3_EXTI_IRQn EXTI4_15_IRQn
#define KEY2_Pin GPIO_PIN_0
#define KEY2_GPIO_Port GPIOA
#define KEY2_EXTI_IRQn EXTI0_1_IRQn
#define KEY1_Pin GPIO_PIN_1
#define KEY1_GPIO_Port GPIOA
#define KEY1_EXTI_IRQn EXTI0_1_IRQn
#define LCD_CS1_Pin GPIO_PIN_4
#define LCD_CS1_GPIO_Port GPIOA
#define LCD_CS5_Pin GPIO_PIN_5
#define LCD_CS5_GPIO_Port GPIOA
#define LCD_CS4_Pin GPIO_PIN_6
#define LCD_CS4_GPIO_Port GPIOA
#define WS2812B_Data_Pin GPIO_PIN_7
#define WS2812B_Data_GPIO_Port GPIOA
#define LCD_CS3_Pin GPIO_PIN_1
#define LCD_CS3_GPIO_Port GPIOB
#define LCD_CS2_Pin GPIO_PIN_11
#define LCD_CS2_GPIO_Port GPIOA
#define KEY5_Pin GPIO_PIN_12
#define KEY5_GPIO_Port GPIOA
#define KEY5_EXTI_IRQn EXTI4_15_IRQn
#define KEY4_Pin GPIO_PIN_4
#define KEY4_GPIO_Port GPIOB
#define KEY4_EXTI_IRQn EXTI4_15_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
