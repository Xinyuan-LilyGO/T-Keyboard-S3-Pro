# 1 "C:\\Users\\XK\\AppData\\Local\\Temp\\tmpqfok2yu4"
#include <Arduino.h>
# 1 "D:/Information/VSCode/GitHub/T-Keyboard-S3-Pro/arduino-esp32-libs_V2.0.14/examples/Lvgl_UI/Lvgl_UI.ino"
# 25 "D:/Information/VSCode/GitHub/T-Keyboard-S3-Pro/arduino-esp32-libs_V2.0.14/examples/Lvgl_UI/Lvgl_UI.ino"
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


uint8_t KNOB_Previous_Logical = 0B00000000;

size_t KNOB_CycleTime = 0;
# 65 "D:/Information/VSCode/GitHub/T-Keyboard-S3-Pro/arduino-esp32-libs_V2.0.14/examples/Lvgl_UI/Lvgl_UI.ino"
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

static uint32_t screenWidth = LCD_WIDTH;
static uint32_t screenHeight = LCD_HEIGHT;
# 97 "D:/Information/VSCode/GitHub/T-Keyboard-S3-Pro/arduino-esp32-libs_V2.0.14/examples/Lvgl_UI/Lvgl_UI.ino"
TFT_eSPI *tft = new TFT_eSPI(screenWidth, screenHeight);

lv_ui guider_ui;
My_Lvgl_UI My_UI;

USBHIDKeyboard Keyboard;
USBHIDConsumerControl ConsumerControl;

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::vector<T_Keyboard_S3_Pro_Device_KEY> KEY_Trigger;
void KNOB_Logical_Scan_Loop(void);
void T_Keyboard_S3_Pro_KEY_Read_Loop(void);
uint8_t T_Keyboard_S3_Pro_Key_Trigger_Loop();
void Lvgl_KEY_Trigger_Loop(uint8_t device_ID);
void Lvgl_Rotary_Encoder_Trigger_Loop();
void Lvgl_KEY_Trigger_Loop_2();
uint8_t Lvgl_Google_Dinosaur_KEY_Trigger_Loop(void);
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
static bool keypad_get_key(void);
void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
void Lvgl_Initialization(void);
void Task1(void *pvParameters);
void setup();
void loop();
#line 110 "D:/Information/VSCode/GitHub/T-Keyboard-S3-Pro/arduino-esp32-libs_V2.0.14/examples/Lvgl_UI/Lvgl_UI.ino"
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

    }
}

void T_Keyboard_S3_Pro_KEY_Read_Loop(void)
{
    if (IIC_Device_ID_Scan.empty() == false)
    {
        if (KEY_Trigger.empty() == true)
        {

            for (int i = 0; i < IIC_Device_ID_Scan.size(); i++)
            {


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
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111);
        delay(10);
    }
# 309 "D:/Information/VSCode/GitHub/T-Keyboard-S3-Pro/arduino-esp32-libs_V2.0.14/examples/Lvgl_UI/Lvgl_UI.ino"
    tft->begin();
    tft->fillScreen(TFT_BLACK);
    delay(100);

    for (int i = 0; i < device_ID.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_ID[i],
                               T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000);
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


                break;
            case KNOB_State::KNOB_DECREMENT:
                ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
                ConsumerControl.release();


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


                break;
            case KNOB_State::KNOB_DECREMENT:
                ConsumerControl.press(CONSUMER_CONTROL_BRIGHTNESS_DECREMENT);
                ConsumerControl.release();


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


                break;
            case KNOB_State::KNOB_DECREMENT:
                My_UI.LCD_Screen_Brightness = My_UI.LCD_Screen_Brightness + 10;
                if (My_UI.LCD_Screen_Brightness > 255)
                {
                    My_UI.LCD_Screen_Brightness = 255;
                }
                ledcWrite(1, My_UI.LCD_Screen_Brightness);


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


void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);


#if (LV_COLOR_16_SWAP != 0)

#else



    tft->startWrite();
    tft->setAddrWindow(area->x1, area->y1, w, h);
    tft->pushColors((uint16_t *)&color_p->full, w * h, true);
    tft->endWrite();
#endif







    lv_disp_flush_ready(disp);
}
# 1496 "D:/Information/VSCode/GitHub/T-Keyboard-S3-Pro/arduino-esp32-libs_V2.0.14/examples/Lvgl_UI/Lvgl_UI.ino"
static bool keypad_get_key(void)
{
    delay(50);

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







        pinMode(LCD_RST, OUTPUT);
        digitalWrite(LCD_RST, HIGH);
        delay(100);
        digitalWrite(LCD_RST, LOW);
        delay(100);
        digitalWrite(LCD_RST, HIGH);
        delay(100);

        LCD_Initialization(IIC_Device_ID_Registry);

        My_UI.Window_Load_Anim_Delay_Lock_Flag = false;
        My_UI.Window_Load_Anim_Delay = 4294967295LL;
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


            data->point.x = touchX;
            data->point.y = touchY;
# 1644 "D:/Information/VSCode/GitHub/T-Keyboard-S3-Pro/arduino-esp32-libs_V2.0.14/examples/Lvgl_UI/Lvgl_UI.ino"
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


    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);


    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = keypad_read;
    lv_indev_drv_register(&indev_drv);
}

void Task1(void *pvParameters)
{


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


            IIC_Bus_CycleTime = millis() + 1;
        }
    }
}

void setup()
{

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
    ledcWrite(1, My_UI.LCD_Screen_Brightness);

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
                           T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);

    if (My_UI.Device_Number == 2)
    {
        IIC_Bus->IIC_WriteC8D8(IIC_SLAVE_DEVICE_ADDRESS,
                               T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);
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

    for (int i = 0; i <= 255; i++)
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
    lv_timer_handler();


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
                                   T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100);

            if (My_UI.Device_Number == 2)
            {
                IIC_Bus->IIC_WriteC8D8(IIC_SLAVE_DEVICE_ADDRESS,
                                       T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000100);
            }
        }
    }
    else
    {
        if (Sleep_Lock_Flag == false)
        {
            Sleep_Lock_Flag = true;
            IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS,
                                   T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);

            if (My_UI.Device_Number == 2)
            {
                IIC_Bus->IIC_WriteC8D8(IIC_SLAVE_DEVICE_ADDRESS,
                                       T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000001);
            }
        }
    }

    if (IIC_Device_ID_State == true)
    {
        if (IIC_Device_ID_Registry.size() > IIC_Device_ID_Scan.size())
        {
            delay(1000);
            if (IIC_Device_ID_Scan.size() == 1)
            {
                if ((IIC_Device_ID_Scan[0] == IIC_MAIN_DEVICE_ADDRESS))
                {
                    My_UI.Device_Number = 1;

                    My_UI.Window_Load_Anim_Delay_Lock_Flag = false;
                    My_UI.Window_Load_Anim_Delay = 4294967295LL;
                    My_UI.Window_Unlock_Flag = false;
                    My_UI.Window_Initialization_Number = 1;
                }
            }
        }
        if (IIC_Device_ID_Registry.size() < IIC_Device_ID_Scan.size())
        {
            delay(1000);
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
                    My_UI.Window_Load_Anim_Delay = 4294967295LL;
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


    if (My_UI.Window_Current_State == My_UI.Window_Current_Home)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            if (My_UI.Device_Number == 2)
            {
                My_UI.Window_Load_Anim_Delay = millis() + 500;
            }
            else
            {
                My_UI.Window_Load_Anim_Delay = millis() + 200;
            }
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if (millis() > My_UI.Window_Load_Anim_Delay)
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Home_Loop();
                Window_CycleTime1 = millis() + 10;
            }
        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Home_2)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            My_UI.Window_Load_Anim_Delay = millis() + 200;
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if (millis() > My_UI.Window_Load_Anim_Delay)
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Home_2_Loop();
                Window_CycleTime1 = millis() + 10;
            }
        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Clock)
    {
        My_UI.Window_Clock_Refresh_Second = millis() / 1000 % 60;
        My_UI.Window_Clock_Refresh_Minute = millis() / 1000 / 60 % 60;
        My_UI.Window_Clock_Refresh_Hour = millis() / 1000 / 60 / 24 % 24;

        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            if (My_UI.Device_Number == 2)
            {
                My_UI.Window_Load_Anim_Delay = millis() + 500;
            }
            else
            {
                My_UI.Window_Load_Anim_Delay = millis() + 300;
            }
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if (millis() > My_UI.Window_Load_Anim_Delay)
        {
            My_UI.Window_Load_Anim_Delay = 0;

            if (millis() > Window_CycleTime1)
            {
                Window_Clock_Loop();
                Window_CycleTime1 = millis() + 10;
            }
        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Keyboard)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            if (My_UI.Device_Number == 2)
            {
                My_UI.Window_Load_Anim_Delay = millis() + 500;
            }
            else
            {
                My_UI.Window_Load_Anim_Delay = millis() + 400;
            }
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if (millis() > My_UI.Window_Load_Anim_Delay)
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Key_Mode_Keyboard_Loop();
                Window_CycleTime1 = millis() + 10;
            }
        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_CtrlCV)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            My_UI.Window_Load_Anim_Delay = millis() + 200;
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if ((millis() > My_UI.Window_Load_Anim_Delay) && (My_UI.Window_Unlock_Flag == false))
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Keyboard_CtrlCV_Initialization();
                Window_CycleTime1 = millis() + 10;
            }
        }
        else
        {

        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_OSU)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            My_UI.Window_Load_Anim_Delay = millis() + 200;
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if (millis() > My_UI.Window_Load_Anim_Delay)
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Keyboard_OSU_Initialization();
                Window_CycleTime1 = millis() + 10;
            }
        }
        else
        {

        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Volume_Windows)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            My_UI.Window_Load_Anim_Delay = millis() + 400;
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if ((millis() > My_UI.Window_Load_Anim_Delay) && (My_UI.Window_Unlock_Flag == false))
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Keyboard_Volume_Initialization();
                Window_CycleTime1 = millis() + 10;
            }
        }
        else
        {

        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Volume)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            if (My_UI.Device_Number == 2)
            {
                My_UI.Window_Load_Anim_Delay = millis() + 500;
            }
            else
            {
                My_UI.Window_Load_Anim_Delay = millis() + 400;
            }
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if (millis() > My_UI.Window_Load_Anim_Delay)
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Key_Mode_Volume_Loop();
                Window_CycleTime1 = millis() + 20;
            }
        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Brightness)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            if (My_UI.Device_Number == 2)
            {
                My_UI.Window_Load_Anim_Delay = millis() + 500;
            }
            else
            {
                My_UI.Window_Load_Anim_Delay = millis() + 400;
            }
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if (millis() > My_UI.Window_Load_Anim_Delay)
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Key_Mode_Brightness_Loop();
                Window_CycleTime1 = millis() + 20;
            }
        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Key_Mode_Game)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            My_UI.Window_Load_Anim_Delay = millis() + 400;
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if (millis() > My_UI.Window_Load_Anim_Delay)
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Key_Mode_Game_Loop();
                Window_CycleTime1 = millis() + 20;
            }
        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_Windows)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            My_UI.Window_Load_Anim_Delay = millis() + 400;
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if ((millis() > My_UI.Window_Load_Anim_Delay) && (My_UI.Window_Unlock_Flag == false))
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Keyboard_Brightness_Windows_Initialization();
                Window_CycleTime1 = millis() + 10;
            }
        }
        else
        {

        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Brightness_MCU)
    {
        if (My_UI.Window_Load_Anim_Delay_Lock_Flag == false)
        {
            My_UI.Window_Load_Anim_Delay = millis() + 400;
            My_UI.Window_Load_Anim_Delay_Lock_Flag = true;
        }

        if ((millis() > My_UI.Window_Load_Anim_Delay) && (My_UI.Window_Unlock_Flag == false))
        {
            if (millis() > Window_CycleTime1)
            {
                Window_Keyboard_Brightness_MCU_Initialization();
                Window_CycleTime1 = millis() + 10;
            }
        }
        else
        {

        }
    }


    if (My_UI.Window_Current_State == My_UI.Window_Current_Keyboard_Game_Google_Dinosaur)
    {
        if ((millis() > My_UI.Window_Load_Anim_Delay) && (My_UI.Window_Unlock_Flag == false))
        {
            if (millis() > Window_CycleTime1)
            {
                Google_Dinosaur_Initialization();
                Window_CycleTime1 = millis() + 10;
            }
        }
        else
        {
            Google_Dinosaur_Loop();
        }
    }
}