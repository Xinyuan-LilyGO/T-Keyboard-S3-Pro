/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32g0xx_it.h
 * @brief   This file contains the headers of the interrupt handlers.
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
#ifndef __STM32G0xx_IT_H
#define __STM32G0xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

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
void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void EXTI0_1_IRQHandler(void);
void EXTI4_15_IRQHandler(void);
void DMA1_Channel1_IRQHandler(void);
void I2C1_IRQHandler(void);
/* USER CODE BEGIN EFP */

    struct IIC_Slave_Receive
    {
        uint8_t Command[101]; // IIC从机接收命令数据寄存器，可以最高存储100条命令块
        uint8_t Data[101];    // IIC从机接收命令数据寄存器，可以最高存储100条命令块
        uint8_t Number;       // IIC从机接收命令数据数量
    };

    struct IIC_Slave_Send
    {
        uint8_t Data;    // IIC从机发送命令数据寄存器
        uint8_t Trigger_Flag;       // IIC从机发送命令触发标志
    };

    extern struct IIC_Slave_Receive IIC_Slave_Receive_Struct;
    extern struct IIC_Slave_Send IIC_Slave_Send_Struct;

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32G0xx_IT_H */
