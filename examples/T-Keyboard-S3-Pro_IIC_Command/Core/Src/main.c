/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "stm32g0xx_it.h"
#include "WS2812B_PWM_DMA.h"
#include "T-Keyboard-S3-Pro_Drive.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_LEDS 14 // Led总个数
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t Temp_Buf[100];

struct T_Keyboard_S3_Pro My_Device;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void Flash_OB_Handle(void)
{
    FLASH_OBProgramInitTypeDef optionsbytesstruct;
    uint8_t UPDATE = 0;

    HAL_FLASHEx_OBGetConfig(&optionsbytesstruct);
    uint32_t userconfig = optionsbytesstruct.USERConfig;

    if ((userconfig & FLASH_OPTR_nBOOT_SEL_Msk) != OB_BOOT0_FROM_PIN)
    {
        userconfig &= ~FLASH_OPTR_nBOOT_SEL_Msk;
        userconfig |= OB_BOOT0_FROM_PIN;
        UPDATE = 1;
    }

    if (UPDATE)
    {
        optionsbytesstruct.USERConfig = userconfig;
        HAL_FLASH_Unlock();
        HAL_FLASH_OB_Unlock();
        HAL_FLASHEx_OBProgram(&optionsbytesstruct);
        HAL_FLASH_OB_Launch();
        HAL_FLASH_OB_Lock();
        HAL_FLASH_Lock();
    }
}

uint8_t KEY_WS2812B_Loop(void)
{
    if (My_Device.KEY_Trigger_Flag == 1)
    {
        My_Device.KEY_Trigger_Flag = 0;
        WS2812B_ClearAll();

        if ((My_Device.KEY_Trigger_Number & B00010000) != 0)
        {
            WS2812B_SetHSV(360, 0, 10, 0);
            WS2812B_SetHSV(360, 0, 10, 1);
        }
        if ((My_Device.KEY_Trigger_Number & B00001000) != 0)
        {
            WS2812B_SetHSV(360, 0, 10, 3);
            WS2812B_SetHSV(360, 0, 10, 4);
        }
        if ((My_Device.KEY_Trigger_Number & B00000100) != 0)
        {
            WS2812B_SetHSV(360, 0, 10, 6);
            WS2812B_SetHSV(360, 0, 10, 7);
        }
        if ((My_Device.KEY_Trigger_Number & B00000010) != 0)
        {
            WS2812B_SetHSV(360, 0, 10, 9);
            WS2812B_SetHSV(360, 0, 10, 10);
        }
        if ((My_Device.KEY_Trigger_Number & B00000001) != 0)
        {
            WS2812B_SetHSV(360, 0, 10, 12);
            WS2812B_SetHSV(360, 0, 10, 13);
        }

        WS2812B_Show();
        return 1;
    }
    return 0;
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
    MX_TIM3_Init();
    MX_I2C1_Init();
    /* USER CODE BEGIN 2 */
    Flash_OB_Handle();

    HAL_I2C_EnableListen_IT(&hi2c1); // 使能I2C1的侦听中断

    WS2812B_ClearAll();
    WS2812B_Show();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        // WS2812B_Test1();
        // WS2812B_Test2(Red, Green, Blue, 60);
        // WS2812B_Test3();
        // WS2812B_Test4();
        // WS2812B_Test5();
        // WS2812B_Test6();
        // WS2812B_Test7(15);
        // WS2812B_Test8();
        // WS2812B_Test9();

        KEY_WS2812B_Loop();

        if (IIC_Slave_Receive_Struct.Number != 0)
        {
            switch (IIC_Slave_Receive_Struct.Command[0])
            {
            case 0x01:
                switch (IIC_Slave_Receive_Struct.Data[0])
                {
                case B00010000:
                    WS2812B_SetHSV(360, 0, 10, 1);
                    WS2812B_Show();
                    HAL_Delay(100);

                    WS2812B_ClearAll();
                    WS2812B_Show();
                    break;
                case B00001000:
                    WS2812B_SetHSV(360, 0, 10, 4);
                    WS2812B_Show();
                    HAL_Delay(100);

                    WS2812B_ClearAll();
                    WS2812B_Show();
                    break;
                case B00000100:
                    WS2812B_SetHSV(360, 0, 10, 7);
                    WS2812B_Show();
                    HAL_Delay(100);

                    WS2812B_ClearAll();
                    WS2812B_Show();
                    break;
                case B00000010:
                    WS2812B_SetHSV(360, 0, 10, 10);
                    WS2812B_Show();
                    HAL_Delay(100);

                    WS2812B_ClearAll();
                    WS2812B_Show();
                    break;
                case B00000001:
                    WS2812B_SetHSV(360, 0, 10, 13);
                    WS2812B_Show();
                    HAL_Delay(100);

                    WS2812B_ClearAll();
                    WS2812B_Show();
                    break;

                default:
                    break;
                }

                break;

            case 0x02: // 读取按键命令
                IIC_Slave_Send_Struct.Data = My_Device.KEY_Trigger_Number;
                break;

            case 0x03:
                switch (IIC_Slave_Receive_Struct.Data[0])
                {
                case B00000001:
                    WS2812B_Test1();
                    break;

                default:
                    break;
                }
                break;

            default:
                break;
            }

            IIC_Slave_Receive_Struct.Number--;
            memmove(&IIC_Slave_Receive_Struct.Command[0],
                    &IIC_Slave_Receive_Struct.Command[1], IIC_Slave_Receive_Struct.Number);
            memmove(&IIC_Slave_Receive_Struct.Data[0],
                    &IIC_Slave_Receive_Struct.Data[1], IIC_Slave_Receive_Struct.Number);
        }

        // HAL_Delay(100);
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

    /** Configure the main internal regulator output voltage
     */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 8;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

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
