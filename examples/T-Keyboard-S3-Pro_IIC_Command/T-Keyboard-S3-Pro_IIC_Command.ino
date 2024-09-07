/*
 * @Description: Testing the IIC slave device commands for the T-Keyboard-S3-Pro.
 * @Author: LILYGO_L
 * @Date: 2023-12-26 16:20:20
 * @LastEditTime: 2024-09-05 13:48:40
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "Arduino_DriveBus_Library.h"
#include "T-Keyboard-S3-Pro_Drive.h"
#include "pin_config.h"

#define IIC_DEVICE_DELAY 20 // 选定屏幕等待时间

#define IIC_DEVICE_ADDRESS 0x01 // 设置要操作的从机IIC地址，默认为主机0x01

std::vector<unsigned char> IIC_Device_ID_Registry_Scan;

uint8_t IIC_Master_Send_Data[2];
uint8_t IIC_Master_Receive_Data;

size_t CycleTime = 0;
size_t CycleTime2 = 0;

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::vector<T_Keyboard_S3_Pro_Device_KEY> KEY_Trigger;

void Task1(void *pvParameters)
{
    // 在这里可以添加一些代码，这样的话这个任务执行时会先执行一次这里的内容
    // 当然后面进入while循环之后不会再执行这部分了
    while (1)
    {
        IIC_Bus->IIC_Device_7Bit_Scan(&IIC_Device_ID_Registry_Scan);
        delay(1); // 修改获取设备状态的灵敏度
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    while (IIC_Bus->begin() == false)
    {
        Serial.println("IIC_Bus initialization fail");
        delay(2000);
    }
    Serial.println("IIC_Bus initialization successfully");

    xTaskCreatePinnedToCore(Task1, "Task1", 10000, NULL, 1, NULL, 1);

    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 100); // 设置LED亮度
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000011); // 选择LED测试模式1
    delay(3000);

    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000010); // 选择LED模式为自由模式
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_H, (uint8_t)((uint16_t)360 >> 8)); // 设置LED的颜色Hue值
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_L, (uint8_t)360); // 设置LED的颜色Hue值
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_COLOR_STATURATION, 0); // 设置LED的颜色Staturation值
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 0); // 设置LED亮度
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B10000000); // 清除选择LED
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111); // 选择LED
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111); // 选择LED
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000); // 显示LED

    for (uint8_t j = 0; j < 3; j++)
    {
        for (int i = 0; i <= 100; i++)
        {
            IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, i); // 设置LED亮度
            IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111); // 选择LED
            IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111); // 选择LED
            IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000); // 显示LED
            delay(2);
        }
        delay(3000);
        for (int i = 100; i >= 0; i--)
        {
            IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, i); // 设置LED亮度
            IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111); // 选择LED
            IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111); // 选择LED
            IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000); // 显示LED
            delay(5);
        }
        delay(2000);
    }

    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 100); // 设置LED亮度
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100); // 选择LED测试模式2
    delay(5000);

    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 10); // 设置LED亮度
    IIC_Bus->IIC_WriteC8D8(IIC_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001); // 选择LED模式为普通模式

    if (IIC_Bus->IIC_ReadC8_Delay_Data(IIC_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_RD_DRIVE_FIRMWARE_VERSION, 20,
                                       &IIC_Master_Receive_Data, 1) == true)
    {
        Serial.printf("STM32 dirve firmware version: %#X \n", IIC_Master_Receive_Data);
    }

    delay(1000);
}

void loop()
{
    // if (millis() > CycleTime)
    // {
    //     if (IIC_Device_ID_Registry_Scan.size() > 0)
    //     {
    //         IIC_Master_Send_Data[0] = T_KEYBOARD_S3_PRO_WR_LCD_CS;
    //         IIC_Master_Send_Data[1] >>= 1;
    //         if (IIC_Master_Send_Data[1] == 0)
    //         {
    //             IIC_Master_Send_Data[1] = 0B00010000;
    //         }

    //         // 扫描到的所有设备都发送数据
    //         for (int i = 0; i < IIC_Device_ID_Registry_Scan.size(); i++)
    //         {
    //             // 1000ms发送一次LCD_CS使能信号
    //             // 按顺序触发T-Keyboard-S3-Pro上的灯
    //             IIC_Bus->IIC_Write_Data(IIC_Device_ID_Registry_Scan[i], IIC_Master_Send_Data, 2);
    //             delay(10);
    //         }
    //     }

    //     CycleTime = millis() + 1000; // 1000ms
    // }

    if (millis() > CycleTime2) // 如果在按键扫描读取数据的时候发送数据的话
    {
        if (IIC_Device_ID_Registry_Scan.size() > 0)
        {
            // 扫描到的所有设备都接收数据
            for (int i = 0; i < IIC_Device_ID_Registry_Scan.size(); i++)
            {
                IIC_Bus->IIC_ReadC8_Delay_Data(IIC_Device_ID_Registry_Scan[i], T_KEYBOARD_S3_PRO_RD_KEY_TRIGGER, 20,
                                               &IIC_Master_Receive_Data, 1);

                T_Keyboard_S3_Pro_Device_KEY key_trigger_temp;
                key_trigger_temp.ID = IIC_Device_ID_Registry_Scan[i];
                key_trigger_temp.Trigger_Data = IIC_Master_Receive_Data;

                KEY_Trigger.push_back(key_trigger_temp);
            }
        }

        // 修改获取设备按键状态的灵敏度
        CycleTime2 = millis() + 100; // 100ms
    }

    if (KEY_Trigger.size() > 0)
    {
        Serial.printf("\nKEY Trigger [ID: %d  Trigger_Data: %d ]\n",
                      KEY_Trigger[0].ID, KEY_Trigger[0].Trigger_Data);
        Serial.printf("KEY Trigger Num: %d\n", KEY_Trigger.size());

        switch (KEY_Trigger[0].Trigger_Data)
        {
        case 0B00010000:
            Serial.printf("Trigger_KEY: KEY1\n");
            break;
        case 0B00001000:
            Serial.printf("Trigger_KEY: KEY2\n");
            break;
        case 0B00000100:
            Serial.printf("Trigger_KEY: KEY3\n");
            break;
        case 0B00000010:
            Serial.printf("Trigger_KEY: KEY4\n");
            break;
        case 0B00000001:
            Serial.printf("Trigger_KEY: KEY5\n");
            break;

        default:
            break;
        }

        KEY_Trigger.erase(KEY_Trigger.begin());
    }
}