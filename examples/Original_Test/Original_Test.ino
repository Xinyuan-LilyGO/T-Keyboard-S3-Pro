/*
 * @Description: 
            Complete test example
        Testing individual connections for each device after completing the main device test, 
    then test the slave device. Click on the 1st button to retest the current test item, click on 
    the 5th button to test the next project, and click on the 3rd button to skip all tests.
 * @Author: LILYGO_L
 * @Date: 2023-12-26 16:20:20
 * @LastEditTime: 2024-09-05 14:01:23
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "Arduino_DriveBus_Library.h"
#include "T-Keyboard-S3-Pro_Drive.h"
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "Material_16Bit.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define IIC_MAIN_DEVICE_ADDRESS 0x01 // 主设备的IIC地址，默认为0x01
#define IIC_LCD_CS_DEVICE_DELAY 20   // 选定屏幕等待时间

#define WIFI_SSID "xinyuandianzi"
#define WIFI_PASSWORD "AA15994823428"
// #define WIFI_SSID "LilyGo-AABB"
// #define WIFI_PASSWORD "xinyuandianzi"

#define WIFI_CONNECT_WAIT_MAX 30000

enum KNOB_State
{
    KNOB_NULL,
    KNOB_INCREMENT,
    KNOB_DECREMENT,
};

// 文件下载链接
// const char *fileDownloadUrl = "https://code.visualstudio.com/docs/?dv=win64user";//vscode
// const char *fileDownloadUrl = "https://dldir1.qq.com/qqfile/qq/PCQQ9.7.17/QQ9.7.17.29225.exe"; // 腾讯CDN加速
// const char *fileDownloadUrl = "https://cd001.www.duba.net/duba/install/packages/ever/kinsthomeui_150_15.exe"; // 金山毒霸
const char *fileDownloadUrl = "http://music.163.com/song/media/outer/url?id=26122999.mp3";
// const char *fileDownloadUrl = "https://github.com/espressif/arduino-esp32/releases/download/3.0.1/esp32-3.0.1.zip";
// const char *fileDownloadUrl = "http://vipspeedtest8.wuhan.net.cn:8080/download?size=1073741824";

std::vector<unsigned char> IIC_Device_ID_Scan;
std::vector<unsigned char> IIC_Device_ID_Registry;

bool IIC_Device_ID_State = false;

uint8_t IIC_Master_Receive_Data = 0;

bool Wifi_Connection_Flag = true;

int32_t KNOB_Data = 0;
bool KNOB_Trigger_Flag = false;
uint8_t KNOB_State_Flag = KNOB_State::KNOB_NULL;

//  0B000000[KNOB_DATA_A][KNOB_DATA_B]
uint8_t KNOB_Previous_Logical = 0B00000000;

size_t CycleTime = 0;
size_t IIC_Bus_CycleTime = 0;
size_t KNOB_CycleTime = 0;

Arduino_DataBus *bus = new Arduino_HWSPI(
    LCD_DC /* DC */, -1 /* CS */, LCD_SCLK /* SCK */, LCD_MOSI /* MOSI */, -1 /* MISO */);

Arduino_GFX *gfx = new Arduino_GC9107(
    bus, -1 /* RST */, 0 /* rotation */, true /* IPS */,
    LCD_WIDTH /* width */, LCD_HEIGHT /* height */,
    2 /* col offset 1 */, 1 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::vector<T_Keyboard_S3_Pro_Device_KEY> KEY_Trigger;

void Wifi_STA_Test(void)
{
    String text;
    int wifi_num = 0;

    gfx->fillScreen(WHITE);
    gfx->setCursor(0, 0);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);

    Serial.println("\nScanning wifi");
    gfx->printf("Scanning wifi\n");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    delay(100);

    wifi_num = WiFi.scanNetworks();
    if (wifi_num == 0)
    {
        text = "\nWiFi scan complete !\nNo wifi discovered.\n";
    }
    else
    {
        text = "\nWiFi scan complete !\n";
        text += wifi_num;
        text += " wifi discovered.\n\n";

        for (int i = 0; i < wifi_num; i++)
        {
            text += (i + 1);
            text += ": ";
            text += WiFi.SSID(i);
            text += " (";
            text += WiFi.RSSI(i);
            text += ")";
            text += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " \n" : "*\n";
            delay(10);
        }
    }

    Serial.println(text);
    gfx->println(text);

    delay(3000);
    gfx->fillScreen(WHITE);
    gfx->setCursor(0, 10);

    Serial.print("Connecting to ");
    gfx->printf("Connecting to\n");

    Serial.print(WIFI_SSID);
    gfx->printf("%s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t last_tick = millis();

    Wifi_Connection_Flag = true;
    while (WiFi.status() != WL_CONNECTED)
    {
        static uint8_t temp_count = 0;
        Serial.print(".");
        gfx->printf(".");
        delay(100);

        temp_count++;
        if ((temp_count % 100) == 0)
        {
            temp_count = 0;
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
        if (millis() - last_tick > WIFI_CONNECT_WAIT_MAX)
        {
            Wifi_Connection_Flag = false;
            break;
        }
    }

    if (Wifi_Connection_Flag == true)
    {
        Serial.print("\nThe connection was successful ! \nTakes ");
        gfx->printf("\nThe connection was successful ! \nTakes ");
        Serial.print(millis() - last_tick);
        gfx->print(millis() - last_tick);
        Serial.println(" ms\n");
        gfx->printf(" ms\n");

        gfx->setTextColor(DARKGREEN);
        gfx->printf("\nWifi test passed!");
    }
    else
    {
        gfx->setTextColor(RED);
        gfx->printf("\nWifi test error!\n");
    }
}

void IIC_KEY_Read_Loop(void)
{
    if (IIC_Device_ID_Scan.empty() == false)
    {
        if (KEY_Trigger.empty() == true) // 等上一次数据读取完成时候再触发
        {
            // 扫描到的所有设备都接收数据
            for (int i = 0; i < IIC_Device_ID_Scan.size(); i++)
            {
                IIC_Bus->IIC_ReadC8_Delay_Data(IIC_Device_ID_Scan[i], T_KEYBOARD_S3_PRO_RD_KEY_TRIGGER, 20,
                                               &IIC_Master_Receive_Data, 1);

                T_Keyboard_S3_Pro_Device_KEY key_trigger_temp;
                key_trigger_temp.ID = IIC_Device_ID_Scan[i];
                key_trigger_temp.Trigger_Data = IIC_Master_Receive_Data;

                KEY_Trigger.push_back(key_trigger_temp);
            }
        }
    }
}

void IIC_KEY_Trigger_Loop(void)
{
    if (KEY_Trigger.empty() == false)
    {
        Serial.printf("\nKEY Trigger [ID: %d  Trigger_Data: %d ]\n",
                      KEY_Trigger[0].ID, KEY_Trigger[0].Trigger_Data);
        Serial.printf("KEY Trigger Num: %d\n", KEY_Trigger.size());

        if (KEY_Trigger[0].Trigger_Data != 0)
        {
            IIC_Bus->IIC_WriteC8D8(KEY_Trigger[0].ID,
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, KEY_Trigger[0].Trigger_Data); // 选定屏幕
            delay(IIC_LCD_CS_DEVICE_DELAY);

            gfx->fillRect(0, 60, 128, 68, WHITE);

            gfx->setTextSize(1);
            gfx->setTextColor(RED);

            gfx->setCursor(25, 60);
            gfx->printf("[KEY Trigger]");

            gfx->setCursor(25, 75);
            gfx->printf("[IIC ID]: 0x");
            if (KEY_Trigger[0].ID < 16)
            {
                gfx->print("0");
                gfx->print(KEY_Trigger[0].ID, HEX);
            }
            else
            {
                gfx->print(KEY_Trigger[0].ID, HEX);
            }

            gfx->setCursor(25, 90);
            gfx->printf("[IIC Data]: %d", KEY_Trigger[0].Trigger_Data);
            gfx->setCursor(25, 105);

            switch (KEY_Trigger[0].Trigger_Data)
            {
            case 0B00010000:
                gfx->printf("[KEY]: KEY1\n");
                break;
            case 0B00001000:
                gfx->printf("[KEY]: KEY2\n");
                break;
            case 0B00000100:
                gfx->printf("[KEY]: KEY3\n");
                break;
            case 0B00000010:
                gfx->printf("[KEY]: KEY4\n");
                break;
            case 0B00000001:
                gfx->printf("[KEY]: KEY5\n");
                break;

            default:
                break;
            }

            IIC_Bus->IIC_WriteC8D8(KEY_Trigger[0].ID,
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }

        KEY_Trigger.erase(KEY_Trigger.begin());
    }
}

void KNOB_Logical_Scan_Loop(void)
{
    uint8_t KNOB_Logical_Scan = 0B00000000;

    if (digitalRead(KNOB_DATA_A) == 1)
    {
        KNOB_Logical_Scan |= 0B00000010;
    }
    else
    {
        KNOB_Logical_Scan &= 0B11111101;
    }

    if (digitalRead(KNOB_DATA_B) == 1)
    {
        KNOB_Logical_Scan |= 0B00000001;
    }
    else
    {
        KNOB_Logical_Scan &= 0B11111110;
    }

    if (KNOB_Previous_Logical != KNOB_Logical_Scan)
    {
        if (KNOB_Logical_Scan == 0B00000000 || KNOB_Logical_Scan == 0B00000011)
        {
            KNOB_Previous_Logical = KNOB_Logical_Scan;
            KNOB_Trigger_Flag = true;
        }
        else
        {
            if (KNOB_Logical_Scan == 0B00000010)
            {
                switch (KNOB_Previous_Logical)
                {
                case 0B00000000:
                    KNOB_State_Flag = KNOB_State::KNOB_INCREMENT;
                    break;
                case 0B00000011:
                    KNOB_State_Flag = KNOB_State::KNOB_DECREMENT;
                    break;

                default:
                    break;
                }
            }
            if (KNOB_Logical_Scan == 0B00000001)
            {
                switch (KNOB_Previous_Logical)
                {
                case 0B00000000:
                    KNOB_State_Flag = KNOB_State::KNOB_DECREMENT;
                    break;
                case 0B00000011:
                    KNOB_State_Flag = KNOB_State::KNOB_INCREMENT;
                    break;

                default:
                    break;
                }
            }
        }
        // delay(IIC_LCD_CS_DEVICE_DELAY);
    }
}

void KNOB_Trigger_Loop(std::vector<unsigned char> device_id)
{
    if (KNOB_Trigger_Flag == true)
    {
        KNOB_Trigger_Flag = false;

        switch (KNOB_State_Flag)
        {
        case KNOB_State::KNOB_INCREMENT:
            for (int i = 0; i < device_id.size(); i++)
            {
                IIC_Bus->IIC_WriteC8D8(device_id[i],
                                       T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
                delay(IIC_LCD_CS_DEVICE_DELAY);
            }
            KNOB_Data++;
            Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);

            gfx->fillRect(0, 60, 128, 68, WHITE);

            gfx->setTextSize(1);
            gfx->setTextColor(MAROON);

            gfx->setCursor(25, 60);
            gfx->printf("[KNOB Trigger]");

            gfx->setCursor(25, 75);
            gfx->printf("[KNOB Data]: %d", KNOB_Data);

            for (int i = 0; i < device_id.size(); i++)
            {
                IIC_Bus->IIC_WriteC8D8(device_id[i],
                                       T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定全部屏幕
                delay(IIC_LCD_CS_DEVICE_DELAY);
            }
            break;
        case KNOB_State::KNOB_DECREMENT:

            for (int i = 0; i < device_id.size(); i++)
            {
                IIC_Bus->IIC_WriteC8D8(device_id[i],
                                       T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
                delay(IIC_LCD_CS_DEVICE_DELAY);
            }
            KNOB_Data--;
            Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);

            gfx->fillRect(0, 60, 128, 68, WHITE);

            gfx->setTextSize(1);
            gfx->setTextColor(BLUE);

            gfx->setCursor(25, 60);
            gfx->printf("[KNOB Trigger]");

            gfx->setCursor(25, 75);
            gfx->printf("[KNOB Data]: %d", KNOB_Data);

            for (int i = 0; i < device_id.size(); i++)
            {
                IIC_Bus->IIC_WriteC8D8(device_id[i],
                                       T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定全部屏幕
                delay(IIC_LCD_CS_DEVICE_DELAY);
            }
            break;

        default:
            break;
        }
    }
}

void LCD_Initialization(std::vector<unsigned char> device_id)
{
    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }

    // 这一步很重要
    // 必须选定全部LCD_CS引脚初始化一遍全部的屏幕才能使用
    // 未初始化屏幕，屏幕将不受控制
    // 外挂多个设备SPI速率只能在4.5MHz以下否则屏幕数据会显示不全
    // 在添加其他设备屏幕的时候也需要重新初始化一遍
    gfx->begin(4500000);
    gfx->fillScreen(BLACK);
    delay(500);

    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
}

void Original_Test_1(uint8_t device_id)
{
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);

    gfx->fillScreen(WHITE);
    gfx->setCursor(40, 30);
    gfx->setTextSize(2);
    gfx->setTextColor(PINK);
    gfx->printf("TEST");

    gfx->setCursor(0, 60);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);
    gfx->printf("1.LCD Backlight Test");

    gfx->setCursor(55, 90);
    gfx->setTextSize(3);
    gfx->setTextColor(RED);
    gfx->printf("3");
    delay(1000);
    gfx->fillRect(55, 90, 30, 30, WHITE);
    gfx->setCursor(55, 90);
    gfx->printf("2");
    delay(1000);
    gfx->fillRect(55, 90, 30, 30, WHITE);
    gfx->setCursor(55, 90);
    gfx->printf("1");
    delay(1000);

    gfx->fillScreen(WHITE);
    gfx->setCursor(20, 55);
    gfx->setTextSize(3);
    gfx->setTextColor(RED);
    gfx->printf("START");

    for (int i = 0; i <= 255; i++)
    {
        ledcWrite(1, i);
        delay(2);
    }
    delay(3000);
    for (int i = 255; i >= 0; i--)
    {
        ledcWrite(1, i);
        delay(5);
    }
    delay(1000);

    gfx->fillScreen(WHITE);
    gfx->setCursor(10, 40);
    gfx->setTextSize(3);
    gfx->setTextColor(ORANGE);
    gfx->printf("FINISH");

    gfx->setCursor(15, 80);
    gfx->setTextSize(1);
    gfx->setTextColor(MAROON);
    gfx->printf("KEY[1]->Try Again");
    gfx->setCursor(15, 90);
    gfx->printf("KEY[5]->Next Test");

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);
}

void Original_Test_2(uint8_t device_id)
{
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);

    gfx->fillScreen(WHITE);
    gfx->setCursor(40, 30);
    gfx->setTextSize(2);
    gfx->setTextColor(PINK);
    gfx->printf("TEST");

    gfx->setCursor(0, 60);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);
    gfx->printf("2.LCD Edge Detection Test");

    gfx->setCursor(55, 90);
    gfx->setTextSize(3);
    gfx->setTextColor(RED);
    gfx->printf("3");
    delay(1000);
    gfx->fillRect(55, 90, 30, 30, WHITE);
    gfx->setCursor(55, 90);
    gfx->printf("2");
    delay(1000);
    gfx->fillRect(55, 90, 30, 30, WHITE);
    gfx->setCursor(55, 90);
    gfx->printf("1");
    delay(1000);

    gfx->fillScreen(WHITE);
    gfx->drawRect(0, 0, 128, 128, RED);

    gfx->setCursor(10, 40);
    gfx->setTextSize(3);
    gfx->setTextColor(ORANGE);
    gfx->printf("FINISH");

    gfx->setCursor(15, 80);
    gfx->setTextSize(1);
    gfx->setTextColor(MAROON);
    gfx->printf("KEY[1]->Try Again");
    gfx->setCursor(15, 90);
    gfx->printf("KEY[5]->Next Test");

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);
}

void Original_Test_3(uint8_t device_id)
{
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);

    gfx->fillScreen(WHITE);
    gfx->setCursor(40, 30);
    gfx->setTextSize(2);
    gfx->setTextColor(PINK);
    gfx->printf("TEST");

    gfx->setCursor(0, 60);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);
    gfx->printf("3.LCD Color Test");

    gfx->setCursor(55, 90);
    gfx->setTextSize(3);
    gfx->setTextColor(RED);
    gfx->printf("3");
    delay(1000);
    gfx->fillRect(55, 90, 30, 30, WHITE);
    gfx->setCursor(55, 90);
    gfx->printf("2");
    delay(1000);
    gfx->fillRect(55, 90, 30, 30, WHITE);
    gfx->setCursor(55, 90);
    gfx->printf("1");
    delay(1000);

    gfx->fillScreen(RED);
    delay(3000);
    gfx->fillScreen(GREEN);
    delay(3000);
    gfx->fillScreen(BLUE);
    delay(3000);
    gfx->fillScreen(WHITE);

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00010000); // 选定屏幕1
    delay(IIC_LCD_CS_DEVICE_DELAY);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_1, 128, 128);
    delay(3000);

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00001000); // 选定屏幕2
    delay(IIC_LCD_CS_DEVICE_DELAY);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_2, 128, 128);
    delay(3000);

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000100); // 选定屏幕3
    delay(IIC_LCD_CS_DEVICE_DELAY);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_3, 128, 128);
    delay(3000);

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000010); // 选定屏幕4
    delay(IIC_LCD_CS_DEVICE_DELAY);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_4, 128, 128);
    delay(3000);

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000001); // 选定屏幕5
    delay(IIC_LCD_CS_DEVICE_DELAY);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_5, 128, 128);
    delay(3000);

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);
    gfx->setCursor(10, 40);
    gfx->setTextSize(3);
    gfx->setTextColor(ORANGE);
    gfx->printf("FINISH");

    gfx->setCursor(15, 80);
    gfx->setTextSize(1);
    gfx->setTextColor(MAROON);
    gfx->printf("KEY[1]->Try Again");
    gfx->setCursor(15, 90);
    gfx->printf("KEY[5]->Next Test");

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);
}

void Original_Test_4(uint8_t device_id)
{
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);

    gfx->fillScreen(WHITE);
    gfx->setCursor(40, 30);
    gfx->setTextSize(2);
    gfx->setTextColor(PINK);
    gfx->printf("TEST");

    gfx->setCursor(0, 60);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);
    gfx->printf("4.LED Color Test");

    gfx->setCursor(55, 90);
    gfx->setTextSize(3);
    gfx->setTextColor(RED);
    gfx->printf("3");
    delay(1000);
    gfx->fillRect(55, 90, 30, 30, WHITE);
    gfx->setCursor(55, 90);
    gfx->printf("2");
    delay(1000);
    gfx->fillRect(55, 90, 30, 30, WHITE);
    gfx->setCursor(55, 90);
    gfx->printf("1");
    delay(1000);

    gfx->fillScreen(WHITE);
    gfx->setCursor(20, 55);
    gfx->setTextSize(3);
    gfx->setTextColor(RED);
    gfx->printf("START");

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 100); // 设置LED亮度
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000011); // 选择LED测试模式1
    delay(3000);

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000010); // 选择LED模式为自由模式
    delay(1000);
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_H, (uint8_t)((uint16_t)360 >> 8)); // 设置LED的颜色Hue值
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_L, (uint8_t)360); // 设置LED的颜色Hue值
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_COLOR_STATURATION, 0); // 设置LED的颜色Staturation值
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 0); // 设置LED亮度
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B10000000); // 清除选择LED
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111); // 选择LED
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111); // 选择LED
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000); // 显示LED

    for (int i = 0; i <= 100; i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id,
                               T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, i); // 设置LED亮度
        IIC_Bus->IIC_WriteC8D8(device_id,
                               T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111); // 选择LED
        IIC_Bus->IIC_WriteC8D8(device_id,
                               T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111); // 选择LED
        IIC_Bus->IIC_WriteC8D8(device_id,
                               T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000); // 显示LED
        delay(2);
    }
    delay(3000);
    for (int i = 100; i >= 0; i--)
    {
        IIC_Bus->IIC_WriteC8D8(device_id,
                               T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, i); // 设置LED亮度
        IIC_Bus->IIC_WriteC8D8(device_id,
                               T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111); // 选择LED
        IIC_Bus->IIC_WriteC8D8(device_id,
                               T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111); // 选择LED
        IIC_Bus->IIC_WriteC8D8(device_id,
                               T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000); // 显示LED
        delay(5);
    }
    delay(2000);

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 100); // 设置LED亮度
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100); // 选择LED测试模式2
    delay(5000);

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 10); // 设置LED亮度
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001); // 选择LED模式为普通模式

    gfx->fillScreen(WHITE);
    gfx->setCursor(10, 40);
    gfx->setTextSize(3);
    gfx->setTextColor(ORANGE);
    gfx->printf("FINISH");

    gfx->setCursor(15, 80);
    gfx->setTextSize(1);
    gfx->setTextColor(MAROON);
    gfx->printf("KEY[1]->Try Again");
    gfx->setCursor(15, 90);
    gfx->printf("KEY[5]->Next Test");

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);
}

void WIFI_HTTP_Download_File(void)
{
    gfx->fillScreen(WHITE);
    gfx->setCursor(0, 10);
    gfx->setTextColor(BLACK);

    // 初始化HTTP客户端
    HTTPClient http;
    http.begin(fileDownloadUrl);
    // 获取重定向的URL
    const char *headerKeys[] = {"Location"};
    http.collectHeaders(headerKeys, 1);

    // 记录下载开始时间
    size_t startTime = millis();
    // 无用时间
    size_t uselessTime = 0;

    // 发起GET请求
    int httpCode = http.GET();

    while (httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND)
    {
        String newUrl = http.header("Location");
        Serial.printf("Redirecting to: %s\n", newUrl.c_str());
        // gfx->printf("Redirecting to: %s\n", newUrl.c_str());
        http.end(); // 关闭旧的HTTP连接

        // 使用新的URL重新发起GET请求
        http.begin(newUrl);
        httpCode = http.GET();
    }

    if (httpCode == HTTP_CODE_OK)
    {
        // 获取文件大小
        size_t fileSize = http.getSize();
        Serial.printf("Starting file download...\n");
        Serial.printf("file size: %f MB\n", fileSize / 1024.0 / 1024.0);
        gfx->printf("Starting file download...\n");
        gfx->printf("file size: %f MB\n", fileSize / 1024.0 / 1024.0);

        // 读取HTTP响应
        WiFiClient *stream = http.getStreamPtr();

        size_t temp_count_s = 0;
        size_t temp_fileSize = fileSize;
        uint8_t buf_1[4096] = {0};
        CycleTime = millis() + 1000; // 开始计时
        bool temp_count_flag = true;
        while (http.connected() && (temp_fileSize > 0 || temp_fileSize == -1))
        {
            // 获取可用数据的大小
            size_t availableSize = stream->available();
            if (availableSize)
            {
                temp_fileSize -= stream->read(buf_1, min(availableSize, (size_t)4096));

                if (millis() > CycleTime)
                {
                    size_t temp_time_1 = millis();
                    temp_count_s++;
                    Serial.printf("WIFI RSSI: (%d)\n", WiFi.RSSI());
                    Serial.printf("Download speed: %f KB/s\n", ((fileSize - temp_fileSize) / 1024.0) / temp_count_s);
                    Serial.printf("Remaining file size: %f MB\n\n", temp_fileSize / 1024.0 / 1024.0);
                    Serial.printf("System running time: %d\n\n", (uint32_t)millis() / 1000);

                    gfx->fillRect(0, 60, 128, 68, WHITE);
                    gfx->setCursor(0, 60);
                    gfx->printf("WIFI RSSI: (%d)\n", WiFi.RSSI());
                    gfx->printf("Speed: %f KB/s\n", ((fileSize - temp_fileSize) / 1024.0) / temp_count_s);
                    gfx->printf("Size: %f MB\n\n", temp_fileSize / 1024.0 / 1024.0);

                    CycleTime = millis() + 1000;
                    size_t temp_time_2 = millis();

                    uselessTime = uselessTime + (temp_time_2 - temp_time_1);
                }
            }

            if (temp_count_s > 30)
            {
                temp_count_flag = false;
                break;
            }
        }

        stream->stop();
        // 关闭HTTP客户端
        http.end();

        // 记录下载结束时间并计算总花费时间
        size_t endTime = millis();

        gfx->fillRect(0, 60, 128, 68, WHITE);
        gfx->setCursor(0, 60);

        if (temp_count_flag == true)
        {
            gfx->setTextColor(DARKGREEN);
            Serial.printf("Download completed!\n");
            Serial.printf("Total download time: %f s\n", (endTime - startTime - uselessTime) / 1000.0);
            Serial.printf("Average download speed: %f KB/s\n", (fileSize / 1024.0) / ((endTime - startTime - uselessTime) / 1000.0));

            gfx->printf("Completed!\n");
            gfx->printf("Time: %f s\n", (endTime - startTime - uselessTime) / 1000.0);
            gfx->printf("Speed: %f KB/s\n", (fileSize / 1024.0) / ((endTime - startTime - uselessTime) / 1000.0));
        }
        else
        {
            gfx->setTextColor(ORANGE);
            Serial.printf("Incomplete!\n");
            Serial.printf("Download time: %f s\n", (endTime - startTime - uselessTime) / 1000.0);
            Serial.printf("Average download speed: %f KB/s\n", ((fileSize - temp_fileSize) / 1024.0) / ((endTime - startTime - uselessTime) / 1000.0));

            gfx->printf("Incomplete!\n");
            gfx->printf("Time: %f s\n", (endTime - startTime - uselessTime) / 1000.0);
            gfx->printf("Speed: %f KB/s\n", ((fileSize - temp_fileSize) / 1024.0) / ((endTime - startTime - uselessTime) / 1000.0));
        }
    }
    else
    {
        gfx->setTextColor(RED);
        Serial.printf("Failed to download\n");
        Serial.printf("Error httpCode: %d \n", httpCode);

        gfx->printf("Failed to download\n");
        gfx->printf("Error httpCode: %d \n\n", httpCode);
    }
}

void Original_Test_5(uint8_t device_id)
{
    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);

    gfx->fillScreen(WHITE);
    gfx->setCursor(40, 30);
    gfx->setTextSize(2);
    gfx->setTextColor(PINK);
    gfx->printf("TEST");

    gfx->setCursor(0, 60);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);
    gfx->printf("5.WIFI STA Test");

    gfx->setCursor(55, 90);
    gfx->setTextSize(3);
    gfx->setTextColor(RED);
    gfx->printf("3");
    delay(1000);
    gfx->fillRect(55, 90, 30, 30, WHITE);
    gfx->setCursor(55, 90);
    gfx->printf("2");
    delay(1000);
    gfx->fillRect(55, 90, 30, 30, WHITE);
    gfx->setCursor(55, 90);
    gfx->printf("1");
    delay(1000);

    Wifi_STA_Test();

    delay(3000);

    if (Wifi_Connection_Flag == true)
    {
        WIFI_HTTP_Download_File();
    }
    else
    {
        gfx->setCursor(20, 100);
        gfx->setTextColor(RED);
        gfx->print("Not connected to the network");
    }
    delay(3000);

    gfx->fillScreen(WHITE);
    gfx->setCursor(10, 40);
    gfx->setTextSize(3);
    gfx->setTextColor(ORANGE);
    gfx->printf("FINISH");

    gfx->setCursor(15, 80);
    gfx->setTextSize(1);
    gfx->setTextColor(MAROON);
    gfx->printf("KEY[1]->Try Again");
    gfx->setCursor(15, 90);
    gfx->printf("KEY[5]->Next Test");

    IIC_Bus->IIC_WriteC8D8(device_id,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);
}

bool IIC_New_Device_Test_Loop(uint8_t device_id)
{
    bool tatchdog_timer_timeout_flag = false;
    size_t watchdog_cycleTime = 0;
    uint8_t watchdog_timer_s = 0;
    const uint8_t tatchdog_timer_timeout_count = 3;//3s后超时

    Original_Test_1(device_id);
    while (1)
    {
        bool temp = false;

        IIC_KEY_Read_Loop();

        if (KEY_Trigger.empty() == false)
        {
            Serial.printf("\nKEY Trigger [ID: %d  Trigger_Data: %d ]\n",
                          KEY_Trigger[0].ID, KEY_Trigger[0].Trigger_Data);
            Serial.printf("KEY Trigger Num: %d\n", KEY_Trigger.size());

            if (KEY_Trigger[0].ID == device_id)
            {
                watchdog_timer_s = 0;

                switch (KEY_Trigger[0].Trigger_Data)
                {
                case 0B00010000:
                    Serial.printf("Trigger_KEY: KEY1\n");
                    Original_Test_1(device_id);
                    break;
                case 0B00001000:
                    Serial.printf("Trigger_KEY: KEY2\n");
                    break;
                case 0B00000100:
                    Serial.printf("Trigger_KEY: KEY3\n");
                    temp = true;
                    tatchdog_timer_timeout_flag = true;
                    break;
                case 0B00000010:
                    Serial.printf("Trigger_KEY: KEY4\n");
                    break;
                case 0B00000001:
                    Serial.printf("Trigger_KEY: KEY5\n");
                    temp = true;
                    break;

                default:
                    break;
                }
            }

            KEY_Trigger.erase(KEY_Trigger.begin());
        }

        if (millis() > watchdog_cycleTime)
        {
            watchdog_timer_s++;
            watchdog_cycleTime = millis() + 1000; // 1000ms
        }

        if (watchdog_timer_s > tatchdog_timer_timeout_count)
        {
            tatchdog_timer_timeout_flag = true;
            break;
        }

        if (temp == true)
        {
            break;
        }
    }

    if (tatchdog_timer_timeout_flag == true)
    {
        return false;
    }

    Original_Test_2(device_id);
    while (1)
    {
        bool temp = false;

        IIC_KEY_Read_Loop();

        if (KEY_Trigger.empty() == false)
        {
            Serial.printf("\nKEY Trigger [ID: %d  Trigger_Data: %d ]\n",
                          KEY_Trigger[0].ID, KEY_Trigger[0].Trigger_Data);
            Serial.printf("KEY Trigger Num: %d\n", KEY_Trigger.size());

            if (KEY_Trigger[0].ID == device_id)
            {
                watchdog_timer_s = 0;

                switch (KEY_Trigger[0].Trigger_Data)
                {
                case 0B00010000:
                    Serial.printf("Trigger_KEY: KEY1\n");
                    Original_Test_2(device_id);
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
                    temp = true;
                    break;

                default:
                    break;
                }
            }
            KEY_Trigger.erase(KEY_Trigger.begin());
        }

        if (millis() > watchdog_cycleTime)
        {
            watchdog_timer_s++;
            watchdog_cycleTime = millis() + 1000; // 1000ms
        }

        if (watchdog_timer_s > tatchdog_timer_timeout_count)
        {
            tatchdog_timer_timeout_flag = true;
            break;
        }

        if (temp == true)
        {
            break;
        }
    }

    if (tatchdog_timer_timeout_flag == true)
    {
        return false;
    }

    Original_Test_3(device_id);
    while (1)
    {
        bool temp = false;

        IIC_KEY_Read_Loop();

        if (KEY_Trigger.empty() == false)
        {
            Serial.printf("\nKEY Trigger [ID: %d  Trigger_Data: %d ]\n",
                          KEY_Trigger[0].ID, KEY_Trigger[0].Trigger_Data);
            Serial.printf("KEY Trigger Num: %d\n", KEY_Trigger.size());

            if (KEY_Trigger[0].ID == device_id)
            {
                watchdog_timer_s = 0;

                switch (KEY_Trigger[0].Trigger_Data)
                {
                case 0B00010000:
                    Serial.printf("Trigger_KEY: KEY1\n");
                    Original_Test_3(device_id);
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
                    temp = true;
                    break;

                default:
                    break;
                }
            }
            KEY_Trigger.erase(KEY_Trigger.begin());
        }

        if (millis() > watchdog_cycleTime)
        {
            watchdog_timer_s++;
            watchdog_cycleTime = millis() + 1000; // 1000ms
        }

        if (watchdog_timer_s > tatchdog_timer_timeout_count)
        {
            tatchdog_timer_timeout_flag = true;
            break;
        }

        if (temp == true)
        {
            break;
        }
    }

    if (tatchdog_timer_timeout_flag == true)
    {
        return false;
    }

    Original_Test_4(device_id);
    while (1)
    {
        bool temp = false;

        IIC_KEY_Read_Loop();

        if (KEY_Trigger.empty() == false)
        {
            Serial.printf("\nKEY Trigger [ID: %d  Trigger_Data: %d ]\n",
                          KEY_Trigger[0].ID, KEY_Trigger[0].Trigger_Data);
            Serial.printf("KEY Trigger Num: %d\n", KEY_Trigger.size());

            if (KEY_Trigger[0].ID == device_id)
            {
                watchdog_timer_s = 0;

                switch (KEY_Trigger[0].Trigger_Data)
                {
                case 0B00010000:
                    Serial.printf("Trigger_KEY: KEY1\n");
                    Original_Test_4(device_id);
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
                    temp = true;
                    break;

                default:
                    break;
                }
            }
            KEY_Trigger.erase(KEY_Trigger.begin());
        }

        if (millis() > watchdog_cycleTime)
        {
            watchdog_timer_s++;
            watchdog_cycleTime = millis() + 1000; // 1000ms
        }

        if (watchdog_timer_s > tatchdog_timer_timeout_count)
        {
            tatchdog_timer_timeout_flag = true;
            IIC_Bus->IIC_WriteC8D8(device_id,
                                   T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001); // 选择LED普通模式
            delay(IIC_LCD_CS_DEVICE_DELAY);
            break;
        }

        if (temp == true)
        {
            IIC_Bus->IIC_WriteC8D8(device_id,
                                   T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001); // 选择LED普通模式
            delay(IIC_LCD_CS_DEVICE_DELAY);
            break;
        }
    }

    if (tatchdog_timer_timeout_flag == true)
    {
        return false;
    }

    Original_Test_5(device_id);
    while (1)
    {
        bool temp = false;

        IIC_KEY_Read_Loop();

        if (KEY_Trigger.empty() == false)
        {
            Serial.printf("\nKEY Trigger [ID: %d  Trigger_Data: %d ]\n",
                          KEY_Trigger[0].ID, KEY_Trigger[0].Trigger_Data);
            Serial.printf("KEY Trigger Num: %d\n", KEY_Trigger.size());

            if (KEY_Trigger[0].ID == device_id)
            {
                watchdog_timer_s = 0;

                switch (KEY_Trigger[0].Trigger_Data)
                {
                case 0B00010000:
                    Serial.printf("Trigger_KEY: KEY1\n");
                    Original_Test_5(device_id);
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
                    temp = true;
                    break;

                default:
                    break;
                }
            }
            KEY_Trigger.erase(KEY_Trigger.begin());
        }

        if (millis() > watchdog_cycleTime)
        {
            watchdog_timer_s++;
            watchdog_cycleTime = millis() + 1000; // 1000ms
        }

        if (watchdog_timer_s > tatchdog_timer_timeout_count)
        {
            tatchdog_timer_timeout_flag = true;
            break;
        }

        if (temp == true)
        {
            break;
        }
    }

    if (tatchdog_timer_timeout_flag == true)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void Print_IIC_New_Device_Added(std::vector<unsigned char> device_id_refresh,
                                uint8_t device_id_find, bool state)
{
    for (int i = 0; i < device_id_refresh.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id_refresh[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }

    gfx->fillScreen(ORANGE);
    gfx->setCursor(15, 30);
    gfx->setTextSize(2);
    gfx->setTextColor(GREEN);
    gfx->printf("FIND IIC");

    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);

    gfx->setCursor(25, 60);
    gfx->printf("[IIC ID]: 0x");
    if (device_id_find < 16)
    {
        gfx->print("0");
        gfx->print(device_id_find, HEX);
    }
    else
    {
        gfx->print(device_id_find, HEX);
    }

    gfx->setTextSize(2);

    if (state == true)
    {
        gfx->setTextColor(GREEN);
        gfx->setCursor(25, 90);
        gfx->printf("SUCCESS");
    }
    else
    {
        gfx->setTextColor(RED);
        gfx->setCursor(40, 90);
        gfx->printf("FAIL");
    }

    for (int i = 0; i < device_id_refresh.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id_refresh[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
}

void Print_IIC_Info(std::vector<unsigned char> device_id)
{
    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }

    gfx->fillScreen(WHITE);
    gfx->setCursor(15, 30);
    gfx->setTextSize(2);
    gfx->setTextColor(PURPLE);
    gfx->printf("IIC Info");

    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
}

void Print_IIC_Info_Refresh(std::vector<unsigned char> device_id)
{
    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }

    gfx->fillRect(0, 60, 128, 68, WHITE);

    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
}

void Print_IIC_Device_Registry_Refresh(std::vector<unsigned char> device_id_refresh,
                                       uint8_t device_id_find)
{
    for (int i = 0; i < device_id_refresh.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id_refresh[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }

    gfx->fillScreen(ORANGE);
    gfx->setCursor(15, 30);
    gfx->setTextSize(2);
    gfx->setTextColor(GREEN);
    gfx->printf("FIND IIC");

    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);

    gfx->setCursor(25, 60);
    gfx->printf("[IIC ID]: 0x");
    if (device_id_find < 16)
    {
        gfx->print("0");
        gfx->print(device_id_find, HEX);
    }
    else
    {
        gfx->print(device_id_find, HEX);
    }

    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(35, 90);
    gfx->printf("START");

    for (int i = 0; i < device_id_refresh.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id_refresh[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
}

void Task1(void *pvParameters)
{
    // 在这里可以添加一些代码，这样的话这个任务执行时会先执行一次这里的内容
    // 当然后面进入while循环之后不会再执行这部分了
    while (1)
    {
        if (millis() > IIC_Bus_CycleTime)
        {
            if (IIC_Bus->IIC_Device_7Bit_Scan(&IIC_Device_ID_Scan) == true)
            {
                if (IIC_Device_ID_Scan.size() != IIC_Device_ID_Registry.size())
                {
                    IIC_Device_ID_State = true;
                }
            }

            // 修改获取设备状态的灵敏度
            IIC_Bus_CycleTime = millis() + 10; // 10ms
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(KNOB_DATA_A, INPUT_PULLUP);
    pinMode(KNOB_DATA_B, INPUT_PULLUP);

    pinMode(LCD_RST, OUTPUT);
    digitalWrite(LCD_RST, HIGH);
    delay(100);
    digitalWrite(LCD_RST, LOW);
    delay(100);
    digitalWrite(LCD_RST, HIGH);
    delay(100);

    ledcAttachPin(LCD_BL, 1);
    ledcSetup(1, 2000, 8);
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

        // if (IIC_Device_ID_State == true)
        // {
        //     delay(100);
        //     IIC_Device_ID_Registry = IIC_Device_ID_Scan;

        //     Serial.printf("Find IIC ID: %#X\n", IIC_Device_ID_Registry[0]);

        //     if (IIC_Device_ID_Registry[0] == IIC_MAIN_DEVICE_ADDRESS) // 只初始化主设备，其他设备稍后测试
        //     {
        //         std::vector<unsigned char> vector_temp;
        //         vector_temp.push_back(IIC_MAIN_DEVICE_ADDRESS);

        //         LCD_Initialization(vector_temp);

        //         temp = true;
        //     }

        //     IIC_Device_ID_State = false;
        // }
        // else
        // {
        //     temp = false;
        // }

        if (IIC_Device_ID_State == true)
        {
            delay(100);

            Serial.printf("Find IIC ID: %#X\n", IIC_Device_ID_Scan[0]);

            if (IIC_Device_ID_Scan[0] == IIC_MAIN_DEVICE_ADDRESS) // 只初始化主设备，其他设备稍后测试
            {
                IIC_Device_ID_Registry.push_back(IIC_Device_ID_Scan[0]);

                std::vector<unsigned char> vector_temp;
                vector_temp.push_back(IIC_MAIN_DEVICE_ADDRESS);

                LCD_Initialization(vector_temp);

                temp = true;
            }

            // IIC_Device_ID_State = false;
        }
        else
        {
            temp = false;
        }

        if (temp == true)
        {
            Serial.println("IIC_Bus select LCD_CS successful");
            break;
        }
        else
        {
            Serial.println("IIC ID not found");
            delay(1000);
        }
    }

    if (IIC_Bus->IIC_ReadC8_Delay_Data(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_RD_DRIVE_FIRMWARE_VERSION, 20,
                                       &IIC_Master_Receive_Data, 1) == true)
    {
        Serial.printf("STM32 dirve firmware version: %#X \n", IIC_Master_Receive_Data);
    }

    IIC_New_Device_Test_Loop(IIC_MAIN_DEVICE_ADDRESS);

    std::vector<unsigned char> vector_temp;
    vector_temp.push_back(IIC_MAIN_DEVICE_ADDRESS);

    Print_IIC_Info(vector_temp);
    delay(100);
}

void loop()
{
    if (millis() > CycleTime)
    {
        Print_IIC_Info_Refresh(IIC_Device_ID_Registry);
        CycleTime = millis() + 3000; // 3000ms
    }

    IIC_KEY_Read_Loop();

    IIC_KEY_Trigger_Loop();

    if (millis() > KNOB_CycleTime)
    {
        KNOB_Logical_Scan_Loop();
        KNOB_CycleTime = millis() + 10; // 10ms
    }

    KNOB_Trigger_Loop(IIC_Device_ID_Registry);

    if (IIC_Device_ID_State == true)
    {
        if (IIC_Device_ID_Registry.size() > IIC_Device_ID_Scan.size()) // 设备被删除
        {
            delay(500); // 等待连接稳定
        }
        if (IIC_Device_ID_Registry.size() < IIC_Device_ID_Scan.size()) // 添加设备
        {
            std::vector<unsigned char> temp_ID; // 去除原来的ID，保留添加的ID
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

            // LCD_Initialization(temp_ID);

            // 扫描到的所有被添加的设备
            for (int i = 0; i < temp_ID.size(); i++)
            {
                if (temp_ID[i] != IIC_MAIN_DEVICE_ADDRESS) // 主设备一直连接所以这里排除主设备
                {
                    std::vector<unsigned char> vector_temp;
                    vector_temp.push_back(temp_ID[i]);

                    delay(1000); // 等待连接稳定

                    Print_IIC_Device_Registry_Refresh(IIC_Device_ID_Registry, temp_ID[i]);

                    LCD_Initialization(vector_temp);

                    if (IIC_New_Device_Test_Loop(temp_ID[i]) == true)
                    {
                        Print_IIC_New_Device_Added(IIC_Device_ID_Registry, temp_ID[i], true);
                    }
                    else
                    {
                        Print_IIC_New_Device_Added(IIC_Device_ID_Registry, temp_ID[i], false);
                    }
                    delay(1000);

                    Print_IIC_Info(vector_temp);
                    delay(1000);
                }
            }
        }

        IIC_Device_ID_Registry = IIC_Device_ID_Scan;

        Print_IIC_Info(IIC_Device_ID_Registry);
        delay(1000);

        IIC_Device_ID_State = false;
    }
}