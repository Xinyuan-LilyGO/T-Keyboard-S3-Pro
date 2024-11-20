/*
 * @Description:
    LVGL Configuration file:
    Copy your_arduino_path/libraries/lvgl/lv_conf_template.h
    to your_arduino_path/libraries/lv_conf.h

    In lv_conf.h around line 15, enable config file:
    #if 1 // Set it to "1" to enable content

    Then find and set:
    #define LV_COLOR_DEPTH     16
    #define LV_TICK_CUSTOM     1

    For SPI/parallel 8 display set color swap can be faster, parallel 16/RGB screen don't swap!
    #define LV_COLOR_16_SWAP   1 // for SPI and parallel 8
    #define LV_COLOR_16_SWAP   0 // for parallel 16 and RGB

 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2023-09-22 11:59:37
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2023-09-23 16:18:21
 * @License: GPL 3.0
 */
#include "lvgl.h"
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "gui_guider.h"
#include "events_init.h"
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include "custom.h"
#include <USB.h>
#include "google_dinosaur.h"

enum KNOB_State
{
    KNOB_NULL,
    KNOB_INCREMENT,
    KNOB_DECREMENT,
};

int32_t KNOB_Data = 0;
bool KNOB_Trigger_Flag = false;
uint8_t KNOB_State_Flag = KNOB_State::KNOB_NULL;

//  0B000000[KNOB_DATA_A][KNOB_DATA_B]
uint8_t KNOB_Previous_Logical = 0B00000000;

size_t KNOB_CycleTime = 0;

// 顺时针转
//  KNOB_DATA_A=0 KNOB_DATA_B=0
//  KNOB_DATA_A=1 KNOB_DATA_B=0  //正在旋转
//  KNOB_DATA_A=1 KNOB_DATA_B=1
//  KNOB_DATA_A=0 KNOB_DATA_B=1  //正在旋转

// 逆时针转
//  KNOB_DATA_A=0 KNOB_DATA_B=0
//  KNOB_DATA_A=0 KNOB_DATA_B=1  //正在旋转
//  KNOB_DATA_A=1 KNOB_DATA_B=1
//  KNOB_DATA_A=1 KNOB_DATA_B=0  //正在旋转

bool Sleep_Flag = false;
bool Sleep_Lock_Flag = false;
uint8_t Sleep_Watchdog_Count = 0;
size_t Sleep_CycleTime = 0;

std::vector<unsigned char> IIC_Device_ID_Scan;
std::vector<unsigned char> IIC_Device_ID_Registry;

bool IIC_Device_ID_State = false;

uint8_t IIC_Master_Receive_Data = 0;

size_t IIC_Bus_CycleTime = 0;

size_t Window_CycleTime1 = 0;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;

static uint32_t screenWidth = LCD_WIDTH;   // screenWidth
static uint32_t screenHeight = LCD_HEIGHT; // screenHeight

// Arduino_DataBus *bus = new Arduino_HWSPI(
//     LCD_DC /* DC */, -1 /* CS */, LCD_SCLK /* SCK */, LCD_MOSI /* MOSI */, -1 /* MISO */);

// Arduino_GFX *gfx = new Arduino_GC9107(
//     bus, -1 /* RST */, 0 /* rotation */, true /* IPS */,
//     LCD_WIDTH /* width */, LCD_HEIGHT /* height */,
//     2 /* col offset 1 */, 1 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

// TFT
TFT_eSPI *tft = new TFT_eSPI(screenWidth, screenHeight);

lv_ui guider_ui;
My_Lvgl_UI My_UI;

USBHIDKeyboard Keyboard;
USBHIDConsumerControl ConsumerControl;

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::vector<T_Keyboard_S3_Pro_Device_KEY> KEY_Trigger;

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

                    Sleep_Flag = false;
                    Sleep_Watchdog_Count = 0;
                    break;
                case 0B00000011:
                    KNOB_State_Flag = KNOB_State::KNOB_DECREMENT;

                    Sleep_Flag = false;
                    Sleep_Watchdog_Count = 0;
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

                    Sleep_Flag = false;
                    Sleep_Watchdog_Count = 0;
                    break;
                case 0B00000011:
                    KNOB_State_Flag = KNOB_State::KNOB_INCREMENT;

                    Sleep_Flag = false;
                    Sleep_Watchdog_Count = 0;
                    break;

                default:
                    break;
                }
            }
        }
        // delay(10);
    }
}

void T_Keyboard_S3_Pro_KEY_Read_Loop(void)
{
    if (IIC_Device_ID_Scan.empty() == false)
    {
        if (KEY_Trigger.empty() == true) // 等上一次数据读取完成时候再触发
        {
            // 扫描到的所有设备都接收数据
            for (int i = 0; i < IIC_Device_ID_Scan.size(); i++)
            {
                // IIC_Bus->IIC_ReadC8_Delay_Data(IIC_Device_ID_Scan[i], T_KEYBOARD_S3_PRO_RD_KEY_TRIGGER, 20,
                //                                &IIC_Master_Receive_Data, 1);
                IIC_Bus->IIC_ReadC8_Data(IIC_Device_ID_Scan[i], T_KEYBOARD_S3_PRO_RD_KEY_TRIGGER,
                                         &IIC_Master_Receive_Data, 1);

                T_Keyboard_S3_Pro_Device_KEY key_trigger_temp;
                key_trigger_temp.ID = IIC_Device_ID_Scan[i];
                key_trigger_temp.Trigger_Data = IIC_Master_Receive_Data;

                KEY_Trigger.push_back(key_trigger_temp);
            }
        }
    }

    if (millis() > Sleep_CycleTime)
    {
        Sleep_Watchdog_Count++;
        Sleep_CycleTime = millis() + 5000;
    }
}

uint8_t T_Keyboard_S3_Pro_Key_Trigger_Loop()
{
    uint8_t temp_buf = 0;

    T_Keyboard_S3_Pro_KEY_Read_Loop();

    if (KEY_Trigger.empty() == false)
    {
        // Serial.printf("\nKEY Trigger [ID0: %d  Trigger_Data: %d ]\n",
        //               KEY_Trigger[0].ID, KEY_Trigger[0].Trigger_Data);
        // Serial.printf("KEY Trigger Num: %d\n", KEY_Trigger.size());

        if (KEY_Trigger[0].ID == IIC_MAIN_DEVICE_ADDRESS)
        {
            if ((KEY_Trigger[0].Trigger_Data & 0B00011111) == 0B00011111)
            {
                temp_buf = 8;
            }
            else if (((KEY_Trigger[0].Trigger_Data & 0B00011000) >> 3) == 0B00000011)
            {
                temp_buf = 5;
            }
            else if (((KEY_Trigger[0].Trigger_Data & 0B00010100) >> 2) == 0B00000101)
            {
                temp_buf = 6;
            }
            else if ((KEY_Trigger[0].Trigger_Data & 0B00000001) == 0B00000001)
            {
                temp_buf = 7;
            }
            else if (((KEY_Trigger[0].Trigger_Data & 0B00000110) >> 1) == 0B00000011)
            {
                temp_buf = 9;
            }
            else if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
            {
                temp_buf = 1;
            }
            else if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
            {
                temp_buf = 2;
            }
            else if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
            {
                temp_buf = 3;
            }
            else if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
            {
                temp_buf = 4;
            }
        }

        if (My_UI.Device_Number == 2)
        {
            if (((KEY_Trigger[1].Trigger_Data & 0B00010000) >> 4) == 1)
            {
                temp_buf = 10;
            }
        }

        KEY_Trigger.erase(KEY_Trigger.begin(), KEY_Trigger.end());
    }
    if (temp_buf != 0)
    {
        Sleep_Flag = false;
        Sleep_Watchdog_Count = 0;
    }
    return temp_buf;
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
    // 外挂多个设备SPI速率只能在10MHz以下否则屏幕数据会显示不全
    // 在添加其他设备屏幕的时候也需要重新初始化一遍
    // tft
    // tft->begin(10000000);
    // tft->fillScreen(BLACK);

    // gfx->begin(6000000);
    // gfx->fillScreen(BLACK);

    tft->begin();
    tft->fillScreen(TFT_BLACK);
    delay(100);

    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000); // 取消选定屏幕
        delay(10);
    }
}

void Lvgl_KEY_Trigger_Loop(uint8_t device_ID)
{
    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_CtrlCV ||
        My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_OSU)
    {
        if (KEY_Trigger.empty() == false)
        {
            switch (device_ID)
            {
            case IIC_MAIN_DEVICE_ADDRESS:
                if (KEY_Trigger[0].ID == IIC_MAIN_DEVICE_ADDRESS)
                {
                    if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
                    {
                        Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[0]);
                    }
                    else
                    {
                        Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[0]);
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
                    {
                        Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[1]);
                    }
                    else
                    {
                        Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[1]);
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
                    {
                        Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[2]);
                    }
                    else
                    {
                        Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[2]);
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
                    {
                        Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[3]);
                    }
                    else
                    {
                        Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[3]);
                    }
                }
                break;

            default:
                break;
            }

            KEY_Trigger.erase(KEY_Trigger.begin(), KEY_Trigger.end());
        }
    }
    else if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Volume_Windows)
    {
        if (KEY_Trigger.empty() == false)
        {
            switch (device_ID)
            {
            case IIC_MAIN_DEVICE_ADDRESS:

                if (KEY_Trigger[0].ID == IIC_MAIN_DEVICE_ADDRESS)
                {
                    if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[2] == 0)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_MUTE);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[2] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[2] = 20;
                            ConsumerControl.press(CONSUMER_CONTROL_MUTE);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[2]++;
                        My_UI.Window_Keyboard_Key_Lock[2] = 1;
                    }
                    else
                    {
                        ConsumerControl.release();
                        My_UI.Window_Keyboard_Key_Press_Delay[2] = 0;
                        My_UI.Window_Keyboard_Key_Lock[2] = 0;
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[0] == 0)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[0] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[0] = 20;
                            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[0]++;
                        My_UI.Window_Keyboard_Key_Lock[0] = 1;
                    }
                    else
                    {
                        ConsumerControl.release();
                        My_UI.Window_Keyboard_Key_Press_Delay[0] = 0;
                        My_UI.Window_Keyboard_Key_Lock[0] = 0;
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[1] == 0)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[1] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[1] = 20;
                            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[1]++;
                        My_UI.Window_Keyboard_Key_Lock[1] = 1;
                    }
                    else
                    {
                        ConsumerControl.release();
                        My_UI.Window_Keyboard_Key_Press_Delay[1] = 0;
                        My_UI.Window_Keyboard_Key_Lock[1] = 0;
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
                    {
                        for (int i = 0; i < 100; i++)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_VOLUME);
                            ConsumerControl.release();
                        }
                    }
                    else
                    {
                    }
                }

                break;

            default:
                break;
            }

            KEY_Trigger.erase(KEY_Trigger.begin(), KEY_Trigger.end());
        }
    }
    else if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_Windows)
    {
        if (KEY_Trigger.empty() == false)
        {
            switch (device_ID)
            {
            case IIC_MAIN_DEVICE_ADDRESS:

                if (KEY_Trigger[0].ID == IIC_MAIN_DEVICE_ADDRESS)
                {
                    if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
                    {
                        for (int i = 0; i < 100; i++)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                            ConsumerControl.release();
                        }
                    }
                    else
                    {
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[0] == 0)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[0] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[0] = 20;
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[0]++;
                        My_UI.Window_Keyboard_Key_Lock[0] = 1;
                    }
                    else
                    {
                        ConsumerControl.release();

                        My_UI.Window_Keyboard_Key_Press_Delay[0] = 0;
                        My_UI.Window_Keyboard_Key_Lock[0] = 0;
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[1] == 0)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[1] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[1] = 20;
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[1]++;
                        My_UI.Window_Keyboard_Key_Lock[1] = 1;
                    }
                    else
                    {
                        ConsumerControl.release();
                        My_UI.Window_Keyboard_Key_Press_Delay[1] = 0;
                        My_UI.Window_Keyboard_Key_Lock[1] = 0;
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
                    {
                        for (int i = 0; i < 100; i++)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
                            ConsumerControl.release();
                        }
                    }
                    else
                    {
                    }
                }

                break;

            default:
                break;
            }

            KEY_Trigger.erase(KEY_Trigger.begin(), KEY_Trigger.end());
        }
    }
    else if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_MCU)
    {
        if (KEY_Trigger.empty() == false)
        {
            switch (device_ID)
            {
            case IIC_MAIN_DEVICE_ADDRESS:

                if (KEY_Trigger[0].ID == IIC_MAIN_DEVICE_ADDRESS)
                {
                    if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
                    {
                        My_UI.LCD_Screen_Brightness = 255;
                        ledcWrite(1, My_UI.LCD_Screen_Brightness);
                    }
                    else
                    {
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[0] == 0)
                        {
                            My_UI.LCD_Screen_Brightness++;
                            if (My_UI.LCD_Screen_Brightness > 255)
                            {
                                My_UI.LCD_Screen_Brightness = 255;
                            }
                            ledcWrite(1, My_UI.LCD_Screen_Brightness);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[0] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[0] = 20;
                            My_UI.LCD_Screen_Brightness++;
                            if (My_UI.LCD_Screen_Brightness > 255)
                            {
                                My_UI.LCD_Screen_Brightness = 255;
                            }
                            ledcWrite(1, My_UI.LCD_Screen_Brightness);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[0]++;
                        My_UI.Window_Keyboard_Key_Lock[0] = 1;
                    }
                    else
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[0] = 0;
                        My_UI.Window_Keyboard_Key_Lock[0] = 0;
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[1] == 0)
                        {
                            My_UI.LCD_Screen_Brightness--;
                            if (My_UI.LCD_Screen_Brightness < 0)
                            {
                                My_UI.LCD_Screen_Brightness = 0;
                            }
                            ledcWrite(1, My_UI.LCD_Screen_Brightness);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[1] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[1] = 20;
                            My_UI.LCD_Screen_Brightness--;
                            if (My_UI.LCD_Screen_Brightness < 0)
                            {
                                My_UI.LCD_Screen_Brightness = 0;
                            }
                            ledcWrite(1, My_UI.LCD_Screen_Brightness);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[1]++;
                        My_UI.Window_Keyboard_Key_Lock[1] = 1;
                    }
                    else
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[1] = 0;
                        My_UI.Window_Keyboard_Key_Lock[1] = 0;
                    }

                    if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
                    {
                        My_UI.LCD_Screen_Brightness = 0;
                        ledcWrite(1, My_UI.LCD_Screen_Brightness);
                    }
                    else
                    {
                    }
                }

                break;

            default:
                break;
            }

            KEY_Trigger.erase(KEY_Trigger.begin(), KEY_Trigger.end());
        }
    }
}

void Lvgl_Rotary_Encoder_Trigger_Loop()
{
    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Volume_Windows)
    {
        if (millis() > KNOB_CycleTime)
        {
            KNOB_Logical_Scan_Loop();
            KNOB_CycleTime = millis() + 5;
        }

        if (KNOB_Trigger_Flag == true)
        {
            KNOB_Trigger_Flag = false;

            switch (KNOB_State_Flag)
            {
            case KNOB_State::KNOB_INCREMENT:
                ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
                ConsumerControl.release();
                // KNOB_Data++;
                // Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
                break;
            case KNOB_State::KNOB_DECREMENT:
                ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
                ConsumerControl.release();
                // KNOB_Data--;
                // Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
                break;

            default:
                break;
            }
        }
    }
    else if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_Windows)
    {
        if (millis() > KNOB_CycleTime)
        {
            KNOB_Logical_Scan_Loop();
            KNOB_CycleTime = millis() + 5;
        }

        if (KNOB_Trigger_Flag == true)
        {
            KNOB_Trigger_Flag = false;

            switch (KNOB_State_Flag)
            {
            case KNOB_State::KNOB_INCREMENT:
                ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
                ConsumerControl.release();
                // KNOB_Data++;
                // Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
                break;
            case KNOB_State::KNOB_DECREMENT:
                ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                ConsumerControl.release();
                // KNOB_Data--;
                // Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
                break;

            default:
                break;
            }
        }
    }
    else if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_MCU)
    {
        if (millis() > KNOB_CycleTime)
        {
            KNOB_Logical_Scan_Loop();
            KNOB_CycleTime = millis() + 5;
        }

        if (KNOB_Trigger_Flag == true)
        {
            KNOB_Trigger_Flag = false;

            switch (KNOB_State_Flag)
            {
            case KNOB_State::KNOB_INCREMENT:
                My_UI.LCD_Screen_Brightness = My_UI.LCD_Screen_Brightness - 10;
                if (My_UI.LCD_Screen_Brightness < 0)
                {
                    My_UI.LCD_Screen_Brightness = 0;
                }
                ledcWrite(1, My_UI.LCD_Screen_Brightness);
                // KNOB_Data++;
                // Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
                break;
            case KNOB_State::KNOB_DECREMENT:
                My_UI.LCD_Screen_Brightness = My_UI.LCD_Screen_Brightness + 10;
                if (My_UI.LCD_Screen_Brightness > 255)
                {
                    My_UI.LCD_Screen_Brightness = 255;
                }
                ledcWrite(1, My_UI.LCD_Screen_Brightness);
                // KNOB_Data--;
                // Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
                break;

            default:
                break;
            }
        }
    }
}

void Lvgl_KEY_Trigger_Loop_2()
{
    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_CtrlCV ||
        My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_OSU)
    {
        if (KEY_Trigger.empty() == false)
        {
            if (KEY_Trigger[0].ID == IIC_MAIN_DEVICE_ADDRESS)
            {
                if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
                {
                    Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[0]);
                }
                else
                {
                    Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[0]);
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
                {
                    Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[1]);
                }
                else
                {
                    Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[1]);
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
                {
                    Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[2]);
                }
                else
                {
                    Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[2]);
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
                {
                    Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[3]);
                }
                else
                {
                    Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[3]);
                }
            }

            if (My_UI.Device_Number == 2)
            {
                if (KEY_Trigger[1].ID == IIC_SLAVE_DEVICE_ADDRESS)
                {
                    if (((KEY_Trigger[1].Trigger_Data & 0B00010000) >> 4) == 1)
                    {
                        Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[0]);
                    }
                    else
                    {
                        Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[0]);
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00001000) >> 3) == 1)
                    {
                        Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[1]);
                    }
                    else
                    {
                        Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[1]);
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00000100) >> 2) == 1)
                    {
                        Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[2]);
                    }
                    else
                    {
                        Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[2]);
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00000010) >> 1) == 1)
                    {
                        Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[3]);
                    }
                    else
                    {
                        Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[3]);
                    }
                    if ((KEY_Trigger[1].Trigger_Data & 0B00000001) == 1)
                    {
                        Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[4]);
                    }
                    else
                    {
                        Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[4]);
                    }
                }
            }

            KEY_Trigger.erase(KEY_Trigger.begin(), KEY_Trigger.end());
        }
    }
    else if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Volume_Windows)
    {
        if (KEY_Trigger.empty() == false)
        {

            if (KEY_Trigger[0].ID == IIC_MAIN_DEVICE_ADDRESS)
            {
                if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
                {
                    if (My_UI.Window_Keyboard_Key_Lock[2] == 0)
                    {
                        ConsumerControl.press(CONSUMER_CONTROL_MUTE);
                    }

                    if (My_UI.Window_Keyboard_Key_Press_Delay[2] >= 20)
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[2] = 20;
                        ConsumerControl.press(CONSUMER_CONTROL_MUTE);
                    }
                    My_UI.Window_Keyboard_Key_Press_Delay[2]++;
                    My_UI.Window_Keyboard_Key_Lock[2] = 1;
                }
                else
                {
                    ConsumerControl.release();
                    My_UI.Window_Keyboard_Key_Press_Delay[2] = 0;
                    My_UI.Window_Keyboard_Key_Lock[2] = 0;
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
                {
                    if (My_UI.Window_Keyboard_Key_Lock[0] == 0)
                    {
                        ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
                    }

                    if (My_UI.Window_Keyboard_Key_Press_Delay[0] >= 20)
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[0] = 20;
                        ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
                    }
                    My_UI.Window_Keyboard_Key_Press_Delay[0]++;
                    My_UI.Window_Keyboard_Key_Lock[0] = 1;
                }
                else
                {
                    ConsumerControl.release();
                    My_UI.Window_Keyboard_Key_Press_Delay[0] = 0;
                    My_UI.Window_Keyboard_Key_Lock[0] = 0;
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
                {
                    if (My_UI.Window_Keyboard_Key_Lock[1] == 0)
                    {
                        ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
                    }

                    if (My_UI.Window_Keyboard_Key_Press_Delay[1] >= 20)
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[1] = 20;
                        ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
                    }
                    My_UI.Window_Keyboard_Key_Press_Delay[1]++;
                    My_UI.Window_Keyboard_Key_Lock[1] = 1;
                }
                else
                {
                    ConsumerControl.release();
                    My_UI.Window_Keyboard_Key_Press_Delay[1] = 0;
                    My_UI.Window_Keyboard_Key_Lock[1] = 0;
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
                {
                    for (int i = 0; i < 100; i++)
                    {
                        ConsumerControl.press(CONSUMER_CONTROL_VOLUME);
                        ConsumerControl.release();
                    }
                }
                else
                {
                }
            }

            if (My_UI.Device_Number == 2)
            {
                if (KEY_Trigger[1].ID == IIC_SLAVE_DEVICE_ADDRESS)
                {
                    if (((KEY_Trigger[1].Trigger_Data & 0B00010000) >> 4) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[2] == 0)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_MUTE);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[2] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[2] = 20;
                            ConsumerControl.press(CONSUMER_CONTROL_MUTE);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[2]++;
                        My_UI.Window_Keyboard_Key_Lock[2] = 1;
                    }
                    else
                    {
                        ConsumerControl.release();
                        My_UI.Window_Keyboard_Key_Press_Delay[2] = 0;
                        My_UI.Window_Keyboard_Key_Lock[2] = 0;
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00001000) >> 3) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[0] == 0)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[0] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[0] = 20;
                            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[0]++;
                        My_UI.Window_Keyboard_Key_Lock[0] = 1;
                    }
                    else
                    {
                        ConsumerControl.release();
                        My_UI.Window_Keyboard_Key_Press_Delay[0] = 0;
                        My_UI.Window_Keyboard_Key_Lock[0] = 0;
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00000100) >> 2) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[1] == 0)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[1] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[1] = 20;
                            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[1]++;
                        My_UI.Window_Keyboard_Key_Lock[1] = 1;
                    }
                    else
                    {
                        ConsumerControl.release();
                        My_UI.Window_Keyboard_Key_Press_Delay[1] = 0;
                        My_UI.Window_Keyboard_Key_Lock[1] = 0;
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00000010) >> 1) == 1)
                    {
                        for (int i = 0; i < 100; i++)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_VOLUME);
                            ConsumerControl.release();
                        }
                    }
                    else
                    {
                    }

                    if ((KEY_Trigger[1].Trigger_Data & 0B00000001) == 1)
                    {
                        Keyboard.press(My_UI.Window_Keyboard_Key_Attribute[4]);
                    }
                    else
                    {
                        Keyboard.release(My_UI.Window_Keyboard_Key_Attribute[4]);
                    }
                }
            }

            KEY_Trigger.erase(KEY_Trigger.begin(), KEY_Trigger.end());
        }
    }
    else if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_Windows)
    {
        if (KEY_Trigger.empty() == false)
        {

            if (KEY_Trigger[0].ID == IIC_MAIN_DEVICE_ADDRESS)
            {
                if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
                {
                    for (int i = 0; i < 100; i++)
                    {
                        ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                        ConsumerControl.release();
                    }
                }
                else
                {
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
                {
                    if (My_UI.Window_Keyboard_Key_Lock[0] == 0)
                    {
                        ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                    }

                    if (My_UI.Window_Keyboard_Key_Press_Delay[0] >= 20)
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[0] = 20;
                        ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                    }
                    My_UI.Window_Keyboard_Key_Press_Delay[0]++;
                    My_UI.Window_Keyboard_Key_Lock[0] = 1;
                }
                else
                {
                    ConsumerControl.release();

                    My_UI.Window_Keyboard_Key_Press_Delay[0] = 0;
                    My_UI.Window_Keyboard_Key_Lock[0] = 0;
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
                {
                    if (My_UI.Window_Keyboard_Key_Lock[1] == 0)
                    {
                        ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
                    }

                    if (My_UI.Window_Keyboard_Key_Press_Delay[1] >= 20)
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[1] = 20;
                        ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
                    }
                    My_UI.Window_Keyboard_Key_Press_Delay[1]++;
                    My_UI.Window_Keyboard_Key_Lock[1] = 1;
                }
                else
                {
                    ConsumerControl.release();
                    My_UI.Window_Keyboard_Key_Press_Delay[1] = 0;
                    My_UI.Window_Keyboard_Key_Lock[1] = 0;
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
                {
                    for (int i = 0; i < 100; i++)
                    {
                        ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
                        ConsumerControl.release();
                    }
                }
                else
                {
                }
            }

            if (My_UI.Device_Number == 2)
            {
                if (KEY_Trigger[1].ID == IIC_SLAVE_DEVICE_ADDRESS)
                {
                    if (((KEY_Trigger[1].Trigger_Data & 0B00010000) >> 4) == 1)
                    {
                        for (int i = 0; i < 100; i++)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                            ConsumerControl.release();
                        }
                    }
                    else
                    {
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00001000) >> 3) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[0] == 0)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[0] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[0] = 20;
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[0]++;
                        My_UI.Window_Keyboard_Key_Lock[0] = 1;
                    }
                    else
                    {
                        ConsumerControl.release();

                        My_UI.Window_Keyboard_Key_Press_Delay[0] = 0;
                        My_UI.Window_Keyboard_Key_Lock[0] = 0;
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00000100) >> 2) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[1] == 0)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[1] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[1] = 20;
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[1]++;
                        My_UI.Window_Keyboard_Key_Lock[1] = 1;
                    }
                    else
                    {
                        ConsumerControl.release();
                        My_UI.Window_Keyboard_Key_Press_Delay[1] = 0;
                        My_UI.Window_Keyboard_Key_Lock[1] = 0;
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00000010) >> 1) == 1)
                    {
                        for (int i = 0; i < 100; i++)
                        {
                            ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_INCREMENT);
                            ConsumerControl.release();
                        }
                    }
                    else
                    {
                    }
                }
            }

            KEY_Trigger.erase(KEY_Trigger.begin(), KEY_Trigger.end());
        }
    }
    else if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_MCU)
    {
        if (KEY_Trigger.empty() == false)
        {
            if (KEY_Trigger[0].ID == IIC_MAIN_DEVICE_ADDRESS)
            {
                if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
                {
                    My_UI.LCD_Screen_Brightness = 255;
                    ledcWrite(1, My_UI.LCD_Screen_Brightness);
                }
                else
                {
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
                {
                    if (My_UI.Window_Keyboard_Key_Lock[0] == 0)
                    {
                        My_UI.LCD_Screen_Brightness++;
                        if (My_UI.LCD_Screen_Brightness > 255)
                        {
                            My_UI.LCD_Screen_Brightness = 255;
                        }
                        ledcWrite(1, My_UI.LCD_Screen_Brightness);
                    }

                    if (My_UI.Window_Keyboard_Key_Press_Delay[0] >= 20)
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[0] = 20;
                        My_UI.LCD_Screen_Brightness++;
                        if (My_UI.LCD_Screen_Brightness > 255)
                        {
                            My_UI.LCD_Screen_Brightness = 255;
                        }
                        ledcWrite(1, My_UI.LCD_Screen_Brightness);
                    }
                    My_UI.Window_Keyboard_Key_Press_Delay[0]++;
                    My_UI.Window_Keyboard_Key_Lock[0] = 1;
                }
                else
                {
                    My_UI.Window_Keyboard_Key_Press_Delay[0] = 0;
                    My_UI.Window_Keyboard_Key_Lock[0] = 0;
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
                {
                    if (My_UI.Window_Keyboard_Key_Lock[1] == 0)
                    {
                        My_UI.LCD_Screen_Brightness--;
                        if (My_UI.LCD_Screen_Brightness < 0)
                        {
                            My_UI.LCD_Screen_Brightness = 0;
                        }
                        ledcWrite(1, My_UI.LCD_Screen_Brightness);
                    }

                    if (My_UI.Window_Keyboard_Key_Press_Delay[1] >= 20)
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[1] = 20;
                        My_UI.LCD_Screen_Brightness--;
                        if (My_UI.LCD_Screen_Brightness < 0)
                        {
                            My_UI.LCD_Screen_Brightness = 0;
                        }
                        ledcWrite(1, My_UI.LCD_Screen_Brightness);
                    }
                    My_UI.Window_Keyboard_Key_Press_Delay[1]++;
                    My_UI.Window_Keyboard_Key_Lock[1] = 1;
                }
                else
                {
                    My_UI.Window_Keyboard_Key_Press_Delay[1] = 0;
                    My_UI.Window_Keyboard_Key_Lock[1] = 0;
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
                {
                    My_UI.LCD_Screen_Brightness = 0;
                    ledcWrite(1, My_UI.LCD_Screen_Brightness);
                }
                else
                {
                }
            }

            if (My_UI.Device_Number == 2)
            {
                if (KEY_Trigger[1].ID == IIC_SLAVE_DEVICE_ADDRESS)
                {
                    if (((KEY_Trigger[1].Trigger_Data & 0B00010000) >> 4) == 1)
                    {
                        My_UI.LCD_Screen_Brightness = 255;
                        ledcWrite(1, My_UI.LCD_Screen_Brightness);
                    }
                    else
                    {
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00001000) >> 3) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[0] == 0)
                        {
                            My_UI.LCD_Screen_Brightness++;
                            if (My_UI.LCD_Screen_Brightness > 255)
                            {
                                My_UI.LCD_Screen_Brightness = 255;
                            }
                            ledcWrite(1, My_UI.LCD_Screen_Brightness);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[0] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[0] = 20;
                            My_UI.LCD_Screen_Brightness++;
                            if (My_UI.LCD_Screen_Brightness > 255)
                            {
                                My_UI.LCD_Screen_Brightness = 255;
                            }
                            ledcWrite(1, My_UI.LCD_Screen_Brightness);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[0]++;
                        My_UI.Window_Keyboard_Key_Lock[0] = 1;
                    }
                    else
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[0] = 0;
                        My_UI.Window_Keyboard_Key_Lock[0] = 0;
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00000100) >> 2) == 1)
                    {
                        if (My_UI.Window_Keyboard_Key_Lock[1] == 0)
                        {
                            My_UI.LCD_Screen_Brightness--;
                            if (My_UI.LCD_Screen_Brightness < 0)
                            {
                                My_UI.LCD_Screen_Brightness = 0;
                            }
                            ledcWrite(1, My_UI.LCD_Screen_Brightness);
                        }

                        if (My_UI.Window_Keyboard_Key_Press_Delay[1] >= 20)
                        {
                            My_UI.Window_Keyboard_Key_Press_Delay[1] = 20;
                            My_UI.LCD_Screen_Brightness--;
                            if (My_UI.LCD_Screen_Brightness < 0)
                            {
                                My_UI.LCD_Screen_Brightness = 0;
                            }
                            ledcWrite(1, My_UI.LCD_Screen_Brightness);
                        }
                        My_UI.Window_Keyboard_Key_Press_Delay[1]++;
                        My_UI.Window_Keyboard_Key_Lock[1] = 1;
                    }
                    else
                    {
                        My_UI.Window_Keyboard_Key_Press_Delay[1] = 0;
                        My_UI.Window_Keyboard_Key_Lock[1] = 0;
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00000010) >> 1) == 1)
                    {
                        My_UI.LCD_Screen_Brightness = 0;
                        ledcWrite(1, My_UI.LCD_Screen_Brightness);
                    }
                    else
                    {
                    }
                }
            }

            KEY_Trigger.erase(KEY_Trigger.begin(), KEY_Trigger.end());
        }
    }
}

uint8_t Lvgl_Google_Dinosaur_KEY_Trigger_Loop(void)
{
    uint8_t temp = 0;
    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Game_Google_Dinosaur)
    {
        if (KEY_Trigger.empty() == false)
        {

            if (KEY_Trigger[0].ID == IIC_MAIN_DEVICE_ADDRESS)
            {
                if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
                {
                    temp = 1;
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
                {
                    temp = 2;
                }

                if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
                {
                    temp = 3;
                }
            }

            if (My_UI.Device_Number == 2)
            {
                if (KEY_Trigger[1].ID == IIC_SLAVE_DEVICE_ADDRESS)
                {
                    if (((KEY_Trigger[1].Trigger_Data & 0B00010000) >> 4) == 1)
                    {
                        temp = 1;
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00001000) >> 3) == 1)
                    {
                        temp = 2;
                    }

                    if (((KEY_Trigger[1].Trigger_Data & 0B00000100) >> 2) == 1)
                    {
                        temp = 3;
                    }
                }
            }
            KEY_Trigger.erase(KEY_Trigger.begin(), KEY_Trigger.end());
        }
    }
    return temp;
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    // GFX
#if (LV_COLOR_16_SWAP != 0)
    // gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
    // gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);

    // TFT
    tft->startWrite();
    tft->setAddrWindow(area->x1, area->y1, w, h);
    tft->pushColors((uint16_t *)&color_p->full, w * h, true);
    tft->endWrite();
#endif

    // TFT
    // tft->startWrite();
    // tft->setAddrWindow(area->x1, area->y1, w, h);
    // tft->pushColors((uint16_t *)&color_p->full, w * h, true);
    // tft->endWrite();

    lv_disp_flush_ready(disp);
}

// /*Read the touchpad*/
// void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
// {
//     if (touchpad.available())
//     {
//         if (touchpad.finger_num > 0)
//         {
//             data->state = LV_INDEV_STATE_PR;

//             /*Set the coordinates*/
//             data->point.x = touchpad.x;
//             data->point.y = touchpad.y;

//             // Serial.print("Data x ");
//             // Serial.println(touchpad.x);

//             // Serial.print("Data y ");
//             // Serial.println(touchpad.y);
//         }
//         else
//         {
//             data->state = LV_INDEV_STATE_REL;
//         }
//     }
// }

/*Get the currently being pressed key.  0 if no key is pressed*/
static bool keypad_get_key(void)
{
    delay(50); // 延时触发判断

    switch (T_Keyboard_S3_Pro_Key_Trigger_Loop())
    {
    case 0:
        break;
    case 1:

        if (My_UI.Window_Current_State == My_UI.Window_Current_Home ||
            My_UI.Window_Current_State == My_UI.Window_Current_Home_2 ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Keyboard ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Volume ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Brightness ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Game)
        {
            My_UI.Lvgl_Key_Trigger_State = My_UI.Lvgl_Key_Trigger_State_Key1;
            return true;
        }
        break;
    case 2:

        if (My_UI.Window_Current_State == My_UI.Window_Current_Home ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Keyboard ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Volume ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Brightness)
        {
            My_UI.Lvgl_Key_Trigger_State = My_UI.Lvgl_Key_Trigger_State_Key2;
            return true;
        }
        break;
    case 3:
        if (My_UI.Window_Current_State == My_UI.Window_Current_Home ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Keyboard ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Volume ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Brightness)
        {
            My_UI.Lvgl_Key_Trigger_State = My_UI.Lvgl_Key_Trigger_State_Key3;
            return true;
        }
        break;
    case 4:
        if (My_UI.Window_Current_State == My_UI.Window_Current_Home ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Keyboard ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Volume ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Brightness)
        {
            My_UI.Lvgl_Key_Trigger_State = My_UI.Lvgl_Key_Trigger_State_Key4;
            return true;
        }
        break;
    case 5:
        if (My_UI.Window_Current_State == My_UI.Window_Current_Home_2 ||
            My_UI.Window_Current_State == My_UI.Window_Current_Home)
        {
            My_UI.Lvgl_Key_Trigger_State = My_UI.Lvgl_Key_Trigger_State_Previous_Page;
            return true;
        }
        break;
    case 7:
        if (My_UI.Window_Current_State == My_UI.Window_Current_Clock ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Keyboard ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Volume ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Brightness ||
            My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Game ||
            My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_CtrlCV ||
            My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Volume_Windows ||
            My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_Windows ||
            My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_MCU ||
            My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Game_Google_Dinosaur ||
            My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_OSU)
        {
            My_UI.Lvgl_Key_Trigger_State = My_UI.Lvgl_Key_Trigger_State_Back;
            return true;
        }
        break;
    case 8:
        // if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_OSU)
        // {
        //     My_UI.Lvgl_Key_Trigger_State = My_UI.Lvgl_Key_Trigger_State_Back_Home;
        //     return true;
        // }

        // 屏幕复位操作
        pinMode(LCD_RST, OUTPUT);
        digitalWrite(LCD_RST, HIGH);
        delay(100);
        digitalWrite(LCD_RST, LOW);
        delay(100);
        digitalWrite(LCD_RST, HIGH);
        delay(100);

        LCD_Initialization(IIC_Device_ID_Registry);

        My_UI.Window_Load_Anim_Delay_Lock_Flag = false;
        My_UI.Window_Load_Anim_Delay = 4294967295LL; // 窗口等待动画加载延时，初始化设置最大
        My_UI.Window_Unlock_Flag = false;
        My_UI.Window_Initialization_Number = 1;
        break;
    case 9:
        if (My_UI.Window_Current_State == My_UI.Window_Current_Home ||
            My_UI.Window_Current_State == My_UI.Window_Current_Home_2)
        {
            My_UI.Lvgl_Key_Trigger_State = My_UI.Lvgl_Key_Trigger_State_Next_Page;
            return true;
        }
        break;
    case 10:
        if (My_UI.Window_Current_State == My_UI.Window_Current_Home)
        {
            My_UI.Lvgl_Key_Trigger_State = My_UI.Lvgl_Device_2_Trigger_State_Key1;
            return true;
        }
        break;

    default:
        break;
    }
    return false;
}

/*Read the touchpad*/
void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    uint16_t touchX = 1, touchY = 1;

    if (T_Keyboard_S3_Pro_Key_Trigger_Loop() == false)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        if (keypad_get_key() == true)
        {
            data->state = LV_INDEV_STATE_PR;

            /*Set the coordinates*/
            data->point.x = touchX;
            data->point.y = touchY;

            // Serial.print("Data x ");
            // Serial.println(touchX);

            // Serial.print("Data y ");
            // Serial.println(touchY);

            // data->key = act_key;
        }
    }
}

void Lvgl_Initialization(void)
{
    lv_init();

    disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * 40, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

    while (!disp_draw_buf)
    {
        Serial.println("LVGL disp_draw_buf allocate failed!");
        delay(1000);
    }

    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * 40);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = keypad_read;
    lv_indev_drv_register(&indev_drv);
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
                if (IIC_Device_ID_Scan != IIC_Device_ID_Registry)
                {
                    IIC_Device_ID_State = true;
                }
            }

            // 修改获取设备状态的灵敏度
            IIC_Bus_CycleTime = millis() + 1; // 1ms
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
    ledcSetup(1, 20000, 8);
    ledcWrite(1, My_UI.LCD_Screen_Brightness); // brightness 0 - 255

    while (IIC_Bus->begin(1000000UL) == false)
    {
        Serial.println("IIC_Bus initialization fail");
        delay(2000);
    }
    Serial.println("IIC_Bus initialization successfully");

    Keyboard.begin();
    ConsumerControl.begin();
    USB.begin();

    xTaskCreatePinnedToCore(Task1, "Task1", 10000, NULL, 1, NULL, 1);

    delay(1000);

    while (1)
    {
        bool temp = false;

        if (IIC_Device_ID_State == true)
        {
            IIC_Device_ID_Registry = IIC_Device_ID_Scan;

            // if (IIC_Device_ID_Registry[0] == IIC_MAIN_DEVICE_ADDRESS) // 只初始化主设备，其他设备稍后测试
            // {
            if (IIC_Device_ID_Registry.size() == 1)
            {
                if (IIC_Device_ID_Registry[0] == IIC_MAIN_DEVICE_ADDRESS)
                {
                    LCD_Initialization(IIC_Device_ID_Registry);

                    My_UI.Device_Number = 1;

                    temp = true;
                }
            }
            else if (IIC_Device_ID_Registry.size() == 2)
            {
                if ((IIC_Device_ID_Registry[0] == IIC_MAIN_DEVICE_ADDRESS) &&
                    (IIC_Device_ID_Registry[1] == IIC_SLAVE_DEVICE_ADDRESS))
                {
                    LCD_Initialization(IIC_Device_ID_Registry);

                    My_UI.Device_Number = 2;

                    temp = true;
                }
            }
            // }

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

    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS,
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001); // 选择LED模式为普通模式

    if (My_UI.Device_Number == 2)
    {
        IIC_Bus->IIC_WriteC8D8(IIC_SLAVE_DEVICE_ADDRESS,
                               T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001); // 选择LED模式为普通模式
    }

    Lvgl_Initialization();

    if (My_UI.Device_Number == 1)
    {
        My_UI.LCD_CS_Set(IIC_MAIN_DEVICE_ADDRESS, 0B00011111, IIC_LCD_CS_DEVICE_DELAY);
    }
    else if (My_UI.Device_Number == 2)
    {
        My_UI.LCD_CS_Set(IIC_MAIN_DEVICE_ADDRESS, 0B00011111, IIC_LCD_CS_DEVICE_DELAY);
        My_UI.LCD_CS_Set(IIC_SLAVE_DEVICE_ADDRESS, 0B00011111, IIC_LCD_CS_DEVICE_DELAY);
    }

    lv_obj_t *img = lv_gif_create(lv_scr_act());
    lv_gif_set_src(img, &_1_gif_128x67);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    for (uint32_t i = 0; i < 5; i++)
    {
        lv_timer_handler();
    }

    for (int i = 0; i <= 255; i++) // 打开屏幕
    {
        My_UI.LCD_Screen_Brightness--;
        ledcWrite(1, My_UI.LCD_Screen_Brightness);
        delay(2);
    }

    for (uint32_t i = 0; i < 200; i++)
    {
        lv_timer_handler();
    }
    lv_obj_del(img);

    if (My_UI.Device_Number == 1)
    {
        My_UI.LCD_CS_Set(IIC_MAIN_DEVICE_ADDRESS, 0B00000000, IIC_LCD_CS_DEVICE_DELAY);
    }
    else if (My_UI.Device_Number == 2)
    {
        My_UI.LCD_CS_Set(IIC_MAIN_DEVICE_ADDRESS, 0B00000000, IIC_LCD_CS_DEVICE_DELAY);
        My_UI.LCD_CS_Set(IIC_SLAVE_DEVICE_ADDRESS, 0B00000000, IIC_LCD_CS_DEVICE_DELAY);
    }

    setup_ui(&guider_ui);
    events_init(&guider_ui);
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    // delay(5);

    if (Sleep_Watchdog_Count == 4)
    {
        Sleep_Flag = true;
        Sleep_Lock_Flag = true;
        Sleep_Watchdog_Count = 5;
    }
    else if (Sleep_Watchdog_Count > 4)
    {
        Sleep_Watchdog_Count = 5;
    }

    if (Sleep_Flag == true)
    {
        if (Sleep_Lock_Flag == true)
        {
            Sleep_Lock_Flag = false;
            IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100); // 选择LED测试模式2

            if (My_UI.Device_Number == 2)
            {
                IIC_Bus->IIC_WriteC8D8(IIC_SLAVE_DEVICE_ADDRESS,
                                       T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100); // 选择LED测试模式2
            }
        }
    }
    else
    {
        if (Sleep_Lock_Flag == false)
        {
            Sleep_Lock_Flag = true;
            IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001); // 选择LED模式为普通模式

            if (My_UI.Device_Number == 2)
            {
                IIC_Bus->IIC_WriteC8D8(IIC_SLAVE_DEVICE_ADDRESS,
                                       T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001); // 选择LED模式为普通模式
            }
        }
    }

    if (IIC_Device_ID_State == true)
    {
        if (IIC_Device_ID_Registry.size() > IIC_Device_ID_Scan.size()) // 设备被删除
        {
            delay(1000); // 等待连接稳定
            if (IIC_Device_ID_Scan.size() == 1)
            {
                if ((IIC_Device_ID_Scan[0] == IIC_MAIN_DEVICE_ADDRESS))
                {
                    My_UI.Device_Number = 1;

                    My_UI.Window_Load_Anim_Delay_Lock_Flag = false;
                    My_UI.Window_Load_Anim_Delay = 4294967295LL; // 窗口等待动画加载延时，初始化设置最大
                    My_UI.Window_Unlock_Flag = false;
                    My_UI.Window_Initialization_Number = 1;
                }
            }
        }
        if (IIC_Device_ID_Registry.size() < IIC_Device_ID_Scan.size()) // 添加设备
        {
            delay(1000); // 等待连接稳定
            if (IIC_Device_ID_Scan.size() == 2)
            {
                if ((IIC_Device_ID_Scan[0] == IIC_MAIN_DEVICE_ADDRESS) &&
                    (IIC_Device_ID_Scan[1] == IIC_SLAVE_DEVICE_ADDRESS))
                {
                    My_UI.Device_Number = 2;

                    std::vector<unsigned char> temp_ID;
                    temp_ID.push_back(IIC_SLAVE_DEVICE_ADDRESS);

                    LCD_Initialization(temp_ID);

                    My_UI.Window_Load_Anim_Delay_Lock_Flag = false;
                    My_UI.Window_Load_Anim_Delay = 4294967295LL; // 窗口等待动画加载延时，初始化设置最大
                    My_UI.Window_Unlock_Flag = false;
                    My_UI.Window_Initialization_Number = 1;
                }
            }
        }

        IIC_Device_ID_Registry = IIC_Device_ID_Scan;

        IIC_Device_ID_State = false;
    }

    T_Keyboard_S3_Pro_KEY_Read_Loop();

    Lvgl_Rotary_Encoder_Trigger_Loop();

    if (My_UI.Device_Number == 2)
    {
        Lvgl_KEY_Trigger_Loop_2();
    }
    else
    {
        Lvgl_KEY_Trigger_Loop(IIC_MAIN_DEVICE_ADDRESS);
    }

    // Home
    if (My_UI.Window_Current_State == My_UI.Window_Current_Home)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            if (My_UI.Device_Number == 2)
            {
                My_UI.Window_Load_Anim_Delay = millis() + 500; // 设置等待动画时间200ms
            }
            else
            {
                My_UI.Window_Load_Anim_Delay = millis() + 200; // 设置等待动画时间200ms
            }
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if (millis() > My_UI.Window_Load_Anim_Delay) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Home_Loop();
                Window_CycleTime1 = millis() + 10; // 10ms
            }
        }
    }

    // Home 2
    if (My_UI.Window_Current_State == My_UI.Window_Current_Home_2)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            My_UI.Window_Load_Anim_Delay = millis() + 200; // 设置等待动画时间200ms
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if (millis() > My_UI.Window_Load_Anim_Delay) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Home_2_Loop();
                Window_CycleTime1 = millis() + 10; // 10ms
            }
        }
    }

    // Clock
    if (My_UI.Window_Current_State == My_UI.Window_Current_Clock)
    {
        My_UI.Window_Clock_Refresh_Second = millis() / 1000 % 60;         // 刷新秒时间
        My_UI.Window_Clock_Refresh_Minute = millis() / 1000 / 60 % 60;    // 刷新分时间
        My_UI.Window_Clock_Refresh_Hour = millis() / 1000 / 60 / 24 % 24; // 刷新时时间

        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            if (My_UI.Device_Number == 2)
            {
                My_UI.Window_Load_Anim_Delay = millis() + 500; // 设置等待动画时间200ms
            }
            else
            {
                My_UI.Window_Load_Anim_Delay = millis() + 300; // 设置等待动画时间200ms
            }
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if (millis() > My_UI.Window_Load_Anim_Delay) // 等待动画加载完成
        {
            My_UI.Window_Load_Anim_Delay = 0; // 任何时间都大于他

            if (millis() > Window_CycleTime1)
            {
                Window_Clock_Loop();
                Window_CycleTime1 = millis() + 10; // 10ms
            }
        }
    }

    // Key Mode Keyboard
    if (My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Keyboard)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            if (My_UI.Device_Number == 2)
            {
                My_UI.Window_Load_Anim_Delay = millis() + 500; // 设置等待动画时间200ms
            }
            else
            {
                My_UI.Window_Load_Anim_Delay = millis() + 400; // 设置等待动画时间200ms
            }
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if (millis() > My_UI.Window_Load_Anim_Delay) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Key_Mode_Keyboard_Loop();
                Window_CycleTime1 = millis() + 10; // 10ms
            }
        }
    }

    // Keyboard CtrlCV
    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_CtrlCV)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            My_UI.Window_Load_Anim_Delay = millis() + 200; // 设置等待动画时间200ms
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if ((millis() > My_UI.Window_Load_Anim_Delay) && (My_UI.Window_Unlock_Flag == false)) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Keyboard_CtrlCV_Initialization();
                Window_CycleTime1 = millis() + 10; // 10ms
            }
        }
        else
        {
            // Window_Keyboard_CtrlCV_Loop();
        }
    }

    // Keyboard OSU
    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_OSU)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            My_UI.Window_Load_Anim_Delay = millis() + 200; // 设置等待动画时间200ms
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if (millis() > My_UI.Window_Load_Anim_Delay) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Keyboard_OSU_Initialization();
                Window_CycleTime1 = millis() + 10; // 10ms
            }
        }
        else
        {
            // Window_Keyboard_OSU_Loop();
        }
    }

    // Keyboard Volume
    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Volume_Windows)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            My_UI.Window_Load_Anim_Delay = millis() + 400; // 设置等待动画时间400ms
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if ((millis() > My_UI.Window_Load_Anim_Delay) && (My_UI.Window_Unlock_Flag == false)) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Keyboard_Volume_Initialization();
                Window_CycleTime1 = millis() + 10; // 10ms
            }
        }
        else
        {
            // Window_Keyboard_Volume_Loop();
        }
    }

    // Key Mode Volume
    if (My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Volume)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            if (My_UI.Device_Number == 2)
            {
                My_UI.Window_Load_Anim_Delay = millis() + 500; // 设置等待动画时间200ms
            }
            else
            {
                My_UI.Window_Load_Anim_Delay = millis() + 400; // 设置等待动画时间200ms
            }
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if (millis() > My_UI.Window_Load_Anim_Delay) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Key_Mode_Volume_Loop();
                Window_CycleTime1 = millis() + 20; // 20ms
            }
        }
    }

    // Key Mode Brightness
    if (My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Brightness)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            if (My_UI.Device_Number == 2)
            {
                My_UI.Window_Load_Anim_Delay = millis() + 500; // 设置等待动画时间200ms
            }
            else
            {
                My_UI.Window_Load_Anim_Delay = millis() + 400; // 设置等待动画时间200ms
            }
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if (millis() > My_UI.Window_Load_Anim_Delay) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Key_Mode_Brightness_Loop();
                Window_CycleTime1 = millis() + 20; // 20ms
            }
        }
    }

    // Key Mode Game
    if (My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Game)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            My_UI.Window_Load_Anim_Delay = millis() + 400; // 设置等待动画时间400ms
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if (millis() > My_UI.Window_Load_Anim_Delay) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Key_Mode_Game_Loop();
                Window_CycleTime1 = millis() + 20; // 20ms
            }
        }
    }

    // Keyboard Brightness Windows
    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_Windows)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            My_UI.Window_Load_Anim_Delay = millis() + 400; // 设置等待动画时间400ms
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if ((millis() > My_UI.Window_Load_Anim_Delay) && (My_UI.Window_Unlock_Flag == false)) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Keyboard_Brightness_Windows_Initialization();
                Window_CycleTime1 = millis() + 10; // 10ms
            }
        }
        else
        {
            // Window_Keyboard_Brightness_Windows_Loop();
        }
    }

    // Keyboard Brightness MCU
    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_MCU)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false) // 如果动画等待设定没有锁的话
        {
            My_UI.Window_Load_Anim_Delay = millis() + 400; // 设置等待动画时间400ms
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true; // 将动画等待设定锁起来
        }

        if ((millis() > My_UI.Window_Load_Anim_Delay) && (My_UI.Window_Unlock_Flag == false)) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Keyboard_Brightness_MCU_Initialization();
                Window_CycleTime1 = millis() + 10; // 10ms
            }
        }
        else
        {
            // Window_Keyboard_Brightness_MCU_Loop();
        }
    }

    // Keyboard Game Google Dinosaur
    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Game_Google_Dinosaur)
    {
        if ((millis() > My_UI.Window_Load_Anim_Delay) && (My_UI.Window_Unlock_Flag == false)) // 等待动画加载完成
        {
            if (millis() > Window_CycleTime1)
            {
                Google_Dinosaur_Initialization();
                Window_CycleTime1 = millis() + 10; // 10ms
            }
        }
        else
        {
            Google_Dinosaur_Loop();
        }
    }
}