/*
 * @Description: None
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-12-26 16:20:20
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-03-25 08:52:49
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

#define WIFI_CONNECT_WAIT_MAX 5000

#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define GMT_OFFSET_SEC 8 * 3600 // Time zone setting function, written as 8 * 3600 in East Eighth Zone (UTC/GMT+8:00)
#define DAY_LIGHT_OFFSET_SEC 0  // Fill in 3600 for daylight saving time, otherwise fill in 0

enum KNOB_State
{
    KNOB_NULL,
    KNOB_INCREMENT,
    KNOB_DECREMENT,
};

std::vector<unsigned char> IIC_Device_ID_Scan;
std::vector<unsigned char> IIC_Device_ID_Registry;

bool IIC_Device_ID_State = false;

uint8_t IIC_Master_Receive_Data = 0;

bool Wifi_Connection_Failure_Flag = false;

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

    gfx->fillScreen(BLACK);
    gfx->setCursor(0, 0);
    gfx->setTextSize(1);
    gfx->setTextColor(GREEN);

    Serial.println("\nScanning wifi");
    gfx->printf("Scanning wifi\n");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
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
    text.clear();
    gfx->fillScreen(BLACK);
    gfx->setCursor(0, 10);

    text = "Connecting to ";
    Serial.print("Connecting to ");
    gfx->printf("Connecting to\n");
    text += WIFI_SSID;
    text += "\n";

    Serial.print(WIFI_SSID);
    gfx->printf("%s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    size_t last_tick = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        gfx->printf(".");
        text += ".";
        delay(100);

        if (millis() - last_tick > WIFI_CONNECT_WAIT_MAX)
        {
            Wifi_Connection_Failure_Flag = true;
            break;
        }
        else
        {
            Wifi_Connection_Failure_Flag = false;
        }
    }

    if (!Wifi_Connection_Failure_Flag)
    {
        text += "\nThe connection was successful ! \nTakes ";
        Serial.print("\nThe connection was successful ! \nTakes ");
        gfx->printf("\nThe connection was successful ! \nTakes ");
        text += millis() - last_tick;
        Serial.print(millis() - last_tick);
        gfx->print(millis() - last_tick);
        text += " ms\n";
        Serial.println(" ms\n");
        gfx->printf(" ms\n");

        gfx->setTextColor(GREEN);
        gfx->printf("\nWifi test passed!");
    }
    else
    {
        gfx->setTextColor(RED);
        gfx->printf("\nWifi test error!\n");
    }
}

void PrintLocalTime(void)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 10000))
    {
        Serial.println("Failed to obtain time");
        gfx->setCursor(20, 90);
        gfx->setTextColor(RED);
        gfx->print("Failed to obtain time!");
        return;
    }
    Serial.println("Get time success");
    Serial.println(&timeinfo, "%A,%B %d %Y %H:%M:%S"); // Format Output
    gfx->setCursor(30, 90);
    gfx->setTextColor(ORANGE);
    gfx->print(&timeinfo, " %Y");
    gfx->setCursor(30, 100);
    gfx->print(&timeinfo, "%B %d");
    gfx->setCursor(30, 110);
    gfx->print(&timeinfo, "%H:%M:%S");
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

void KNOB_Trigger_Loop(std::vector<unsigned char> device_ID)
{
    if (KNOB_Trigger_Flag == true)
    {
        KNOB_Trigger_Flag = false;

        switch (KNOB_State_Flag)
        {
        case KNOB_State::KNOB_INCREMENT:
            for (int i = 0; i < device_ID.size(); i++)
            {
                IIC_Bus->IIC_WriteC8D8(device_ID[i],
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

            for (int i = 0; i < device_ID.size(); i++)
            {
                IIC_Bus->IIC_WriteC8D8(device_ID[i],
                                       T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定全部屏幕
                delay(IIC_LCD_CS_DEVICE_DELAY);
            }
            break;
        case KNOB_State::KNOB_DECREMENT:

            for (int i = 0; i < device_ID.size(); i++)
            {
                IIC_Bus->IIC_WriteC8D8(device_ID[i],
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

            for (int i = 0; i < device_ID.size(); i++)
            {
                IIC_Bus->IIC_WriteC8D8(device_ID[i],
                                       T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定全部屏幕
                delay(IIC_LCD_CS_DEVICE_DELAY);
            }
            break;

        default:
            break;
        }
    }
}

void LCD_Initialization(std::vector<unsigned char> device_ID)
{
    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
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

    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
}

void Original_Test_4(uint8_t device_ID)
{
    IIC_Bus->IIC_WriteC8D8(device_ID,
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000010); // 选择LED测试模式
    delay(IIC_LCD_CS_DEVICE_DELAY);
}

void Original_Test_5(uint8_t device_ID)
{
    IIC_Bus->IIC_WriteC8D8(device_ID,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);

    Wifi_STA_Test();

    delay(2000);

    while (Wifi_Connection_Failure_Flag)
    {
        gfx->setCursor(20, 100);
        gfx->setTextColor(RED);
        gfx->print("Not connected to the network");

        Wifi_STA_Test();
    }

    // Obtain and set the time from the network time server
    // After successful acquisition, the chip will use the RTC clock to update the holding time
    configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);

    delay(3000);

    PrintLocalTime();

    delay(5000);

    IIC_Bus->IIC_WriteC8D8(device_ID,
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
    delay(IIC_LCD_CS_DEVICE_DELAY);
}

bool IIC_New_Device_Test_Loop(uint8_t device_ID)
{
    Original_Test_4(device_ID);

    Original_Test_5(device_ID);

    return true;
}

void Print_IIC_New_Device_Added(std::vector<unsigned char> device_ID_refresh,
                                uint8_t device_ID_find, bool state)
{
    for (int i = 0; i < device_ID_refresh.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID_refresh[i],
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
    if (device_ID_find < 16)
    {
        gfx->print("0");
        gfx->print(device_ID_find, HEX);
    }
    else
    {
        gfx->print(device_ID_find, HEX);
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

    for (int i = 0; i < device_ID_refresh.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID_refresh[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
}

void Print_IIC_Info(std::vector<unsigned char> device_ID)
{
    struct tm timeinfo;

    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }

    gfx->fillScreen(WHITE);
    gfx->setTextSize(1);

    gfx->setCursor(0, 50);
    gfx->setTextColor(ORANGE);

    if (!getLocalTime(&timeinfo, 3000))
    {
        Serial.println("Failed to obtain time");
        gfx->setCursor(30, 10);
        gfx->setTextColor(RED);
        gfx->print("Failed to obtain time!");
    }
    else
    {
        Serial.println("Get time success");
        Serial.println(&timeinfo, "%A,%B %d %Y %H:%M:%S"); // Format Output
        gfx->setCursor(30, 10);
        gfx->setTextColor(ORANGE);
        gfx->print(&timeinfo, " %Y");
        gfx->setCursor(30, 20);
        gfx->print(&timeinfo, "%B %d");
        gfx->setCursor(30, 30);
        gfx->print(&timeinfo, "%H:%M:%S");
    }

    gfx->setCursor(0, 50);
    gfx->setTextColor(PURPLE);
    gfx->printf("Run Time:%d", (int)millis() / 1000);

    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
}

void Print_IIC_Info_Refresh(std::vector<unsigned char> device_ID)
{
    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111); // 选定全部屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }

    gfx->fillRect(0, 60, 128, 68, WHITE);

    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(IIC_LCD_CS_DEVICE_DELAY);
    }
}

void Print_IIC_Device_Registry_Refresh(std::vector<unsigned char> device_ID_refresh,
                                       uint8_t device_ID_find)
{
    for (int i = 0; i < device_ID_refresh.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID_refresh[i],
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
    if (device_ID_find < 16)
    {
        gfx->print("0");
        gfx->print(device_ID_find, HEX);
    }
    else
    {
        gfx->print(device_ID_find, HEX);
    }

    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(35, 90);
    gfx->printf("START");

    for (int i = 0; i < device_ID_refresh.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID_refresh[i],
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
    // Serial.begin(115200);
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

        if (IIC_Device_ID_State == true)
        {
            IIC_Device_ID_Registry = IIC_Device_ID_Scan;

            if (IIC_Device_ID_Registry[0] == IIC_MAIN_DEVICE_ADDRESS) // 只初始化主设备，其他设备稍后测试
            {
                std::vector<unsigned char> vector_temp;
                vector_temp.push_back(IIC_MAIN_DEVICE_ADDRESS);

                LCD_Initialization(vector_temp);

                temp = true;
            }

            IIC_Device_ID_State = false;
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
        Print_IIC_Info(IIC_Device_ID_Registry);
        Print_IIC_Info_Refresh(IIC_Device_ID_Registry);

        for (int i = 0; i < IIC_Device_ID_Registry.size(); i++)
        {
            Original_Test_4(IIC_Device_ID_Registry[i]);
        }

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

                    Original_Test_4(temp_ID[i]);

                    // if (IIC_New_Device_Test_Loop(temp_ID[i]) == true)
                    // {
                    //     Print_IIC_New_Device_Added(IIC_Device_ID_Registry, temp_ID[i], true);
                    // }
                    // else
                    // {
                    //     Print_IIC_New_Device_Added(IIC_Device_ID_Registry, temp_ID[i], false);
                    // }
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