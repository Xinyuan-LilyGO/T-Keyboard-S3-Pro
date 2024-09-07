/*
 * @Description: Screen Test
 * @Author: LILYGO_L
 * @Date: 2023-12-26 16:20:20
 * @LastEditTime: 2024-09-05 11:36:45
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "Arduino_DriveBus_Library.h"
#include "T-Keyboard-S3-Pro_Drive.h"
#include "Arduino_GFX_Library.h"
#include "pin_config.h"

std::vector<unsigned char> IIC_Device_ID_Scan;
std::vector<unsigned char> IIC_Device_ID_Registry;

bool IIC_Device_ID_State = false;

uint8_t IIC_Master_Send_Data[2];
uint8_t IIC_Master_Receive_Data;

size_t CycleTime = 0;
size_t CycleTime2 = 0;
size_t CycleTime3 = 0;

Arduino_DataBus *bus = new Arduino_HWSPI(
    LCD_DC /* DC */, -1 /* CS */, LCD_SCLK /* SCK */, LCD_MOSI /* MOSI */, -1 /* MISO */);

Arduino_GFX *gfx = new Arduino_GC9107(
    bus, -1 /* RST */, 0 /* rotation */, true /* IPS */,
    LCD_WIDTH /* width */, LCD_HEIGHT /* height */,
    2 /* col offset 1 */, 1 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::vector<T_Keyboard_S3_Pro_Device_KEY> KEY_Trigger;

void Task1(void *pvParameters)
{
    // 在这里可以添加一些代码，这样的话这个任务执行时会先执行一次这里的内容
    // 当然后面进入while循环之后不会再执行这部分了
    while (1)
    {
        if (IIC_Bus->IIC_Device_7Bit_Scan(&IIC_Device_ID_Scan) == true)
        {
            if (IIC_Device_ID_Scan != IIC_Device_ID_Registry)
            {
                IIC_Device_ID_State = true;
                delay(1); // 修改获取设备状态的灵敏度
            }
        }
    }
}

void LCD_Initialization(std::vector<unsigned char> device_ID)
{
    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
        delay(10);
    }

    // 这一步很重要
    // 必须选定全部LCD_CS引脚初始化一遍全部的屏幕才能使用
    // 未初始化屏幕，屏幕将不受控制
    // 外挂多个设备SPI速率只能在4.5MHz以下否则屏幕数据会显示不全
    // 在添加其他设备屏幕的时候也需要重新初始化一遍
    gfx->begin(4500000);
    gfx->fillScreen(BLACK);
    delay(100);

    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(10);
    }
}

void LCD_Show_Test(std::vector<unsigned char> device_ID)
{
    // 扫描到的所有设备都发送数据
    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00010000); // 选定第1个屏幕
        delay(10);

        gfx->setCursor(5, 50);
        gfx->fillScreen(WHITE);
        gfx->setTextColor(RED);
        gfx->printf("IIC ID: 0x");
        if (device_ID[i] < 16)
        {
            gfx->print("0");
            gfx->print(device_ID[i], HEX);
        }
        else
        {
            gfx->print(device_ID[i], HEX);
        }
        gfx->setCursor(5, 60);
        gfx->setTextColor(MAGENTA);
        gfx->println("Ciallo1~(L *##*L)^**");

        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(10);
    }

    // 扫描到的所有设备都发送数据
    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00001000); // 选定第2个屏幕
        delay(10);

        gfx->setCursor(5, 50);
        gfx->fillScreen(WHITE);
        gfx->setTextColor(RED);
        gfx->printf("IIC ID: 0x");
        if (device_ID[i] < 16)
        {
            gfx->print("0");
            gfx->print(device_ID[i], HEX);
        }
        else
        {
            gfx->print(device_ID[i], HEX);
        }
        gfx->setCursor(5, 60);
        gfx->setTextColor(MAGENTA);
        gfx->println("Ciallo2~(L *##*L)^**");

        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(10);
    }

    // 扫描到的所有设备都发送数据
    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000100); // 选定第3个屏幕
        delay(10);

        gfx->setCursor(5, 50);
        gfx->fillScreen(WHITE);
        gfx->setTextColor(RED);
        gfx->printf("IIC ID: 0x");
        if (device_ID[i] < 16)
        {
            gfx->print("0");
            gfx->print(device_ID[i], HEX);
        }
        else
        {
            gfx->print(device_ID[i], HEX);
        }
        gfx->setCursor(5, 60);
        gfx->setTextColor(MAGENTA);
        gfx->println("Ciallo3~(L *##*L)^**");

        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(10);
    }

    // 扫描到的所有设备都发送数据
    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000010); // 选定第4个屏幕
        delay(10);

        gfx->setCursor(5, 50);
        gfx->fillScreen(WHITE);
        gfx->setTextColor(RED);
        gfx->printf("IIC ID: 0x");
        if (device_ID[i] < 16)
        {
            gfx->print("0");
            gfx->print(device_ID[i], HEX);
        }
        else
        {
            gfx->print(device_ID[i], HEX);
        }
        gfx->setCursor(5, 60);
        gfx->setTextColor(MAGENTA);
        gfx->println("Ciallo4~(L *##*L)^**");

        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(10);
    }

    // 扫描到的所有设备都发送数据
    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000001); // 选定第5个屏幕
        delay(10);

        gfx->setCursor(5, 50);
        gfx->fillScreen(WHITE);
        gfx->setTextColor(RED);
        gfx->printf("IIC ID: 0x");
        if (device_ID[i] < 16)
        {
            gfx->print("0");
            gfx->print(device_ID[i], HEX);
        }
        else
        {
            gfx->print(device_ID[i], HEX);
        }
        gfx->setCursor(5, 60);
        gfx->setTextColor(MAGENTA);
        gfx->println("Ciallo5~(L *##*L)^**");

        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(10);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(LCD_RST, OUTPUT);
    digitalWrite(LCD_RST, HIGH);

    ledcAttachPin(LCD_BL, 1);
    ledcSetup(1, 20000, 8);
    ledcWrite(1, 0); // brightness 0 - 255

    while (IIC_Bus->begin(1000000UL) == false)
    {
        Serial.println("IIC_Bus initialization fail");
        delay(2000);
    }
    Serial.println("IIC_Bus initialization successfully");

    xTaskCreatePinnedToCore(Task1, "Task1", 10000, NULL, 1, NULL, 1);

    delay(1000);

    while (1)
    {
        bool temp = false;

        if (IIC_Device_ID_State == true)
        {
            IIC_Device_ID_Registry = IIC_Device_ID_Scan;

            LCD_Initialization(IIC_Device_ID_Registry);
            IIC_Device_ID_State = false;

            temp = true;
        }
        else
        {
            temp = false;
        }

        if (temp == true)
        {
            Serial.println("IIC_Bus select LCD_ CS successful");
            break;
        }
        else
        {
            Serial.println("IIC_Bus select LCD_ CS fail");
            delay(1000);
        }
    }

    LCD_Show_Test(IIC_Device_ID_Registry);
}

void loop()
{
    // if (millis() > CycleTime)
    // {
    //     if (IIC_Device_ID_Scan.size() > 0)
    //     {
    //         IIC_Master_Send_Data[0] = T_KEYBOARD_S3_PRO_WR_LCD_CS;
    //         IIC_Master_Send_Data[1] >>= 1;
    //         if (IIC_Master_Send_Data[1] == 0)
    //         {
    //             IIC_Master_Send_Data[1] = 0B00010000;
    //         }

    //         // 扫描到的所有设备都发送数据
    //         for (int i = 0; i < IIC_Device_ID_Scan.size(); i++)
    //         {
    //             // 1000ms发送一次LCD_CS使能信号
    //             // 按顺序触发T-Keyboard-S3-Pro上的灯
    //             IIC_Bus->IIC_Write_Data(IIC_Device_ID_Scan[i], IIC_Master_Send_Data, 2);
    //         }
    //     }

    //     CycleTime = millis() + 1000; // 1000ms
    // }

    if (millis() > CycleTime2)
    {
        if (IIC_Device_ID_Scan.size() > 0)
        {
            // 扫描到的所有设备都接收数据
            for (int i = 0; i < IIC_Device_ID_Scan.size(); i++)
            {
                IIC_Bus->IIC_ReadC8_Data(IIC_Device_ID_Scan[i], T_KEYBOARD_S3_PRO_RD_KEY_TRIGGER,
                                         &IIC_Master_Receive_Data, 1);

                if (IIC_Master_Receive_Data != 0)
                {
                    T_Keyboard_S3_Pro_Device_KEY key_trigger_temp;
                    key_trigger_temp.ID = IIC_Device_ID_Scan[i];
                    key_trigger_temp.Trigger_Data = IIC_Master_Receive_Data;

                    KEY_Trigger.push_back(key_trigger_temp);
                }
            }
        }

        // 修改获取设备按键状态的灵敏度
        CycleTime2 = millis() + 10; // 10ms
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

    if (IIC_Device_ID_State == true)
    {
        if (IIC_Device_ID_Registry.size() > IIC_Device_ID_Scan.size()) // 设备被删除
        {
        }
        if (IIC_Device_ID_Registry.size() < IIC_Device_ID_Scan.size()) // 添加设备
        {
            std::vector<unsigned char> temp_ID; // 去除原来的ID保留添加的ID
            temp_ID = IIC_Device_ID_Scan;

            for (auto i = IIC_Device_ID_Registry.begin(); i != IIC_Device_ID_Registry.end(); ++i)
            {
                temp_ID.erase(std::remove(temp_ID.begin(), temp_ID.end(), *i), temp_ID.end());
            }

            // Serial.printf("%d\n", temp_ID.size());
            // for (int i = 0; i < temp_ID.size(); i++)
            // {
            //     Serial.printf("temp_ID:%d\n", temp_ID[i]);
            // }

            LCD_Initialization(temp_ID);

            LCD_Show_Test(temp_ID);
        }

        IIC_Device_ID_Registry = IIC_Device_ID_Scan;
        IIC_Device_ID_State = false;
    }

    // if (millis() > CycleTime)
    // {
    //     Serial.printf("IIC_Device_ID_Registry Size: %d\n", IIC_Device_ID_Registry.size());
    //     Serial.printf("IIC_Device_ID_Scan Size: %d\n", IIC_Device_ID_Scan.size());
    //     Serial.println();

    //     CycleTime = millis() + 1000; // 1000ms
    // }
}