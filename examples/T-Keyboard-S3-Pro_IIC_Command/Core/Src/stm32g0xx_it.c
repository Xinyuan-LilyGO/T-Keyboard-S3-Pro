/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32g0xx_it.c
 * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g0xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "tim.h"
#include "T-Keyboard-S3-Pro_Drive.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_tim3_ch2;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    while (1)
    {
    }
    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
    /* USER CODE BEGIN HardFault_IRQn 0 */

    /* USER CODE END HardFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
    }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void)
{
    /* USER CODE BEGIN SVC_IRQn 0 */

    /* USER CODE END SVC_IRQn 0 */
    /* USER CODE BEGIN SVC_IRQn 1 */

    /* USER CODE END SVC_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void)
{
    /* USER CODE BEGIN PendSV_IRQn 0 */

    /* USER CODE END PendSV_IRQn 0 */
    /* USER CODE BEGIN PendSV_IRQn 1 */

    /* USER CODE END PendSV_IRQn 1 */
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
    /* USER CODE BEGIN SysTick_IRQn 0 */

    /* USER CODE END SysTick_IRQn 0 */
    HAL_IncTick();
    /* USER CODE BEGIN SysTick_IRQn 1 */

    /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles EXTI line 0 and line 1 interrupts.
 */
void EXTI0_1_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI0_1_IRQn 0 */

    /* USER CODE END EXTI0_1_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(KEY2_Pin);
    HAL_GPIO_EXTI_IRQHandler(KEY1_Pin);
    /* USER CODE BEGIN EXTI0_1_IRQn 1 */

    /* USER CODE END EXTI0_1_IRQn 1 */
}

/**
 * @brief This function handles EXTI line 4 to 15 interrupts.
 */
void EXTI4_15_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI4_15_IRQn 0 */

    /* USER CODE END EXTI4_15_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(KEY4_Pin);
    HAL_GPIO_EXTI_IRQHandler(KEY5_Pin);
    HAL_GPIO_EXTI_IRQHandler(KEY3_Pin);
    /* USER CODE BEGIN EXTI4_15_IRQn 1 */

    /* USER CODE END EXTI4_15_IRQn 1 */
}

/**
 * @brief This function handles DMA1 channel 1 interrupt.
 */
void DMA1_Channel1_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

    /* USER CODE END DMA1_Channel1_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_tim3_ch2);
    /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

    /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
 * @brief This function handles I2C1 event global interrupt / I2C1 wake-up interrupt through EXTI line 23.
 */
void I2C1_IRQHandler(void)
{
    /* USER CODE BEGIN I2C1_IRQn 0 */

    /* USER CODE END I2C1_IRQn 0 */
    if (hi2c1.Instance->ISR & (I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR))
    {
        HAL_I2C_ER_IRQHandler(&hi2c1);
    }
    else
    {
        HAL_I2C_EV_IRQHandler(&hi2c1);
    }
    /* USER CODE BEGIN I2C1_IRQn 1 */

    /* USER CODE END I2C1_IRQn 1 */
}

/* USER CODE BEGIN 1 */
/**
 * @brief DMA 传输完成暂停回调函数
 * @param *htim
 * @return
 * @Date 2022-10-17 22:27:42
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_2);
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
    case KEY1_Pin:
        My_Device.KEY_Trigger_Number &= B11101111;
        My_Device.KEY_Trigger_Flag = 1;
        break;
    case KEY2_Pin:
        My_Device.KEY_Trigger_Number &= B11110111;
        My_Device.KEY_Trigger_Flag = 1;
        break;
    case KEY3_Pin:
        My_Device.KEY_Trigger_Number &= B11111011;
        My_Device.KEY_Trigger_Flag = 1;
        break;
    case KEY4_Pin:
        My_Device.KEY_Trigger_Number &= B11111101;
        My_Device.KEY_Trigger_Flag = 1;
        break;
    case KEY5_Pin:
        My_Device.KEY_Trigger_Number &= B11111110;
        My_Device.KEY_Trigger_Flag = 1;
        break;

    default:
        break;
    }
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
    case KEY1_Pin:
        My_Device.KEY_Trigger_Number |= B00010000;
        My_Device.KEY_Trigger_Flag = 1;
        break;
    case KEY2_Pin:
        My_Device.KEY_Trigger_Number |= B00001000;
        My_Device.KEY_Trigger_Flag = 1;
        break;
    case KEY3_Pin:
        My_Device.KEY_Trigger_Number |= B00000100;
        My_Device.KEY_Trigger_Flag = 1;
        break;
    case KEY4_Pin:
        My_Device.KEY_Trigger_Number |= B00000010;
        My_Device.KEY_Trigger_Flag = 1;
        break;
    case KEY5_Pin:
        My_Device.KEY_Trigger_Number |= B00000001;
        My_Device.KEY_Trigger_Flag = 1;
        break;

    default:
        break;
    }
}

struct IIC_Slave_Receive IIC_Slave_Receive_Struct;
struct IIC_Slave_Send IIC_Slave_Send_Struct;

static uint8_t IIC_Slave_Receive_Buff[2]; // IIC从机接收数据缓冲器（中断里的参数必须为全局变量）

/**
 * @brief I2C设备地址回调函数（地址匹配上以后会进入该函数）
 * @param *hi2c
 * @param TransferDirection
 * @param AddrMatchCode
 * @return
 * @Date 2023-12-27 09:18:49
 */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    if (TransferDirection == I2C_DIRECTION_TRANSMIT) // 主机发送，从机接收
    {
        // 接收第一位命令
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, IIC_Slave_Receive_Buff, 2, I2C_FIRST_FRAME);

        // 如果接收到的数据不为0或者‘\0’
        if (IIC_Slave_Receive_Buff[0] != 0 || IIC_Slave_Receive_Buff[0] != '\0')
        {
            IIC_Slave_Send_Struct.Data = 0; // 收到命令时候先清除一遍发送寄存器避免发送错误数据

            switch (IIC_Slave_Receive_Buff[0]) // 判断是否为读命令
            {


            default:
                break;
            }
            
            if (IIC_Slave_Receive_Struct.Number < 100)
            {
                memmove(&IIC_Slave_Receive_Struct.Command[IIC_Slave_Receive_Struct.Number], &IIC_Slave_Receive_Buff[0], 1);
                memmove(&IIC_Slave_Receive_Struct.Data[IIC_Slave_Receive_Struct.Number], &IIC_Slave_Receive_Buff[1], 1);
                IIC_Slave_Receive_Struct.Number++;

                memset(IIC_Slave_Receive_Buff, 0, sizeof(IIC_Slave_Receive_Buff)); // 清空数组数据
            }
        }
    }
    else // 主机接收，从机发送
    {
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, &IIC_Slave_Send_Struct.Data, 1, I2C_LAST_FRAME);
    }
}

/**
 * @brief  I2C数据接收回调函数
 *          （在I2C完成一次接收时会关闭中断并调用该函数，因此在处理完成后需要手动重新打开中断）
 * @param *hi2c
 * @return
 * @Date 2023-12-27 09:21:12
 */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint8_t IIC_Slave_Receive_Temp[2] = {0, 0};
    // 打开中断这里开始接收清除其他非命令的杂物
    HAL_I2C_Slave_Seq_Receive_IT(hi2c, IIC_Slave_Receive_Temp, 2, I2C_LAST_FRAME);
}

/**
 * @brief I2C数据发送回调函数
 *          （在I2C完成一次发送后会关闭中断并调用该函数，因此在处理完成后需要手动重新打开中断）
 * @param *hi2c
 * @return
 * @Date 2023-12-27 09:21:56
 */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_Slave_Seq_Transmit_IT(hi2c, &IIC_Slave_Send_Struct.Data, 1, I2C_FIRST_FRAME);
}

/**
 * @brief 侦听完成回调函数（完成一次完整的i2c通信以后会进入该函数）
 * @param *hi2c
 * @return
 * @Date 2023-12-26 18:19:15
 */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_EnableListen_IT(hi2c); // slave is ready again
}

/* USER CODE END 1 */
