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
static uint8_t KEY_WS2812B_Loop_Mode = 0;
static uint8_t KEY_WS2812B_Brightness = 10;
static uint16_t KEY_WS2812B_Hue = 360;
static uint8_t KEY_WS2812B_Saturation = 0;

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
            WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 0);
            WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 1);
        }
        if ((My_Device.KEY_Trigger_Number & B00001000) != 0)
        {
            WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 3);
            WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 4);
        }
        if ((My_Device.KEY_Trigger_Number & B00000100) != 0)
        {
            WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 6);
            WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 7);
        }
        if ((My_Device.KEY_Trigger_Number & B00000010) != 0)
        {
            WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 9);
            WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 10);
        }
        if ((My_Device.KEY_Trigger_Number & B00000001) != 0)
        {
            WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 12);
            WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 13);
        }

        WS2812B_Show();
        return 1;
    }
    return 0;
}

void KEY_WS2812B_Mode_1(void)
{
    static uint8_t key_ws2812b_set_color = 0;

    switch (key_ws2812b_set_color)
    {
    case 0:
        WS2812B_SetSequentialHSV(0, 100, KEY_WS2812B_Brightness, NUM_LEDS);
        break;
    case 1:
        WS2812B_SetSequentialHSV(120, 100, KEY_WS2812B_Brightness, NUM_LEDS);
        break;
    case 2:
        WS2812B_SetSequentialHSV(240, 100, KEY_WS2812B_Brightness, NUM_LEDS);
        break;

    default:
        break;
    }

    key_ws2812b_set_color++;
    if (key_ws2812b_set_color > 2)
    {
        key_ws2812b_set_color = 0;
    }
}

void KEY_WS2812B_Mode_2(void)
{
    // WS2812B_Shift1(Shift_Left, 1);
    WS2812B_Shift2(Shift_Right, 1);
    WS2812B_Show();
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
    MX_TIM14_Init();
    /* USER CODE BEGIN 2 */
    Flash_OB_Handle();

    HAL_TIM_Base_Start_IT(&htim14);  // 初始化定时器中断
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

        if (TIM14_Timed_10ms_Flag == 1)
        {
            TIM14_Timed_10ms_Flag = 0;
            if (KEY_WS2812B_Loop_Mode == 0)
            {
                KEY_WS2812B_Loop();
            }
        }

        switch (KEY_WS2812B_Loop_Mode)
        {
        case 2:
            if (TIM14_Timed_1000ms_Flag == 1)
            {
                TIM14_Timed_1000ms_Flag = 0;
                KEY_WS2812B_Mode_1();
            }
            break;
        case 3:
            if (TIM14_Timed_50ms_Flag == 1)
            {
                TIM14_Timed_50ms_Flag = 0;
                KEY_WS2812B_Mode_2();
            }
            break;

        default:

            break;
        }

        if (IIC_Slave_Receive_Struct.Number != 0)
        {
            switch (IIC_Slave_Receive_Struct.Command[0])
            {
            case 0x01:
                if ((IIC_Slave_Receive_Struct.Data[0] & B00010000) != 0) // LCD_CS1
                {
                    HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET);
                }
                else
                {
                    HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_SET);
                }

                if ((IIC_Slave_Receive_Struct.Data[0] & B00001000) != 0) // LCD_CS2
                {
                    HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET);
                }
                else
                {
                    HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_SET);
                }

                if ((IIC_Slave_Receive_Struct.Data[0] & B00000100) != 0) // LCD_CS3
                {
                    HAL_GPIO_WritePin(LCD_CS3_GPIO_Port, LCD_CS3_Pin, GPIO_PIN_RESET);
                }
                else
                {
                    HAL_GPIO_WritePin(LCD_CS3_GPIO_Port, LCD_CS3_Pin, GPIO_PIN_SET);
                }

                if ((IIC_Slave_Receive_Struct.Data[0] & B00000010) != 0) // LCD_CS4
                {
                    HAL_GPIO_WritePin(LCD_CS4_GPIO_Port, LCD_CS4_Pin, GPIO_PIN_RESET);
                }
                else
                {
                    HAL_GPIO_WritePin(LCD_CS4_GPIO_Port, LCD_CS4_Pin, GPIO_PIN_SET);
                }

                if ((IIC_Slave_Receive_Struct.Data[0] & B00000001) != 0) // LCD_CS5
                {
                    HAL_GPIO_WritePin(LCD_CS5_GPIO_Port, LCD_CS5_Pin, GPIO_PIN_RESET);
                }
                else
                {
                    HAL_GPIO_WritePin(LCD_CS5_GPIO_Port, LCD_CS5_Pin, GPIO_PIN_SET);
                }
                break;
            case 0x03:
                switch (IIC_Slave_Receive_Struct.Data[0])
                {
                case B00000001:
                    WS2812B_ClearAll();
                    KEY_WS2812B_Loop_Mode = 0;
                    break;
                case B00000010:
                    WS2812B_ClearAll();
                    KEY_WS2812B_Loop_Mode = 1;
                    break;
                case B00000011:
                    WS2812B_ClearAll();
                    KEY_WS2812B_Loop_Mode = 2;
                    break;
                case B00000100:
                    WS2812B_ClearAll();
                    for (uint8_t i = 0; i < NUM_LEDS; i++)
                    {
                        WS2812B_SetHSV(i * (360.0 / NUM_LEDS), 100, KEY_WS2812B_Brightness, i);
                        WS2812B_Show();
                    }
                    KEY_WS2812B_Loop_Mode = 3;
                    break;
                default:
                    break;
                }
                break;
            case 0x04:
                if (IIC_Slave_Receive_Struct.Data[0] <= 100)
                {
                    KEY_WS2812B_Brightness = IIC_Slave_Receive_Struct.Data[0];
                }
                break;
            case 0x05:
                if (IIC_Slave_Receive_Struct.Data[0] <= 1)
                {
                    KEY_WS2812B_Hue &= 0xFEFF;
                    KEY_WS2812B_Hue = (((uint16_t)(IIC_Slave_Receive_Struct.Data[0] & B00000001)) << 8) | KEY_WS2812B_Hue;
                }
                break;
            case 0x06:
                    KEY_WS2812B_Hue &= 0xFF00;
                    KEY_WS2812B_Hue = ((uint16_t)IIC_Slave_Receive_Struct.Data[0]) | KEY_WS2812B_Hue;
                break;
            case 0x07:9
                if (IIC_Slave_Receive_Struct.Data[0] <= 100)
                {
                    KEY_WS2812B_Saturation = IIC_Slave_Receive_Struct.Data[0];
                }
                break;
            case 0x08:
                if ((IIC_Slave_Receive_Struct.Data[0] & B10000000) != 0)
                {
                    WS2812B_ClearAll();
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B01000000) != 0)
                {
                    WS2812B_Show();
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00100000) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 0);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00010000) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 1);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00001000) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 2);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00000100) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 3);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00000010) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 4);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00000001) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 5);
                }
                break;
            case 0x09:
                if ((IIC_Slave_Receive_Struct.Data[0] & B10000000) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 6);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B01000000) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 7);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00100000) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 8);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00010000) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 9);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00001000) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 10);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00000100) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 11);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00000010) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 12);
                }
                if ((IIC_Slave_Receive_Struct.Data[0] & B00000001) != 0)
                {
                    WS2812B_SetHSV(KEY_WS2812B_Hue, KEY_WS2812B_Saturation, KEY_WS2812B_Brightness, 13);
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
    }
    // HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
