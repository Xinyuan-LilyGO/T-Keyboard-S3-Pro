/*
 * @Description: None
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2023-10-25 14:11:58
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-09-20 03:36:58
 * @License: GPL 3.0
 */
#pragma once

#include <Arduino.h>
#include "gui_guider.h"
#include "T-Keyboard-S3-Pro_Drive.h"
#include <USBHIDKeyboard.h>
#include <USBHIDConsumerControl.h>
#include "TFT_eSPI.h"
#include "Arduino_DriveBus_Library.h"

#define IIC_MAIN_DEVICE_ADDRESS 0x01  // 主设备的IIC地址，默认为0x01
#define IIC_SLAVE_DEVICE_ADDRESS 0x02 // 从设备的IIC地址
#define IIC_LCD_CS_DEVICE_DELAY 10    // 选定屏幕等待时间10ms

class My_Lvgl_UI
{
public:
    enum Window_Current_State
    {
        Window_Current_NULL = 0,
        Window_Current_Home,
        Window_Current_Home_2,
        Window_Current_Clock,

        Window_Current_Key_Mode_Keyboard,
        Window_Current_Key_Mode_Volume,
        Window_Current_Key_Mode_Brightness,
        Window_Current_Key_Mode_Game,

        Window_Current_Keyboard_CtrlCV,
        Window_Current_Keyboard_OSU,
        Window_Current_Keyboard_Volume_Windows,
        Window_Current_Keyboard_Brightness_Windows,
        Window_Current_Keyboard_Brightness_MCU,
        Window_Current_Keyboard_Game_Google_Dinosaur,

    };

    enum Lvgl_Key_Trigger_State
    {
        Lvgl_Key_Trigger_State_NULL = 0,
        Lvgl_Key_Trigger_State_Key1,
        Lvgl_Key_Trigger_State_Key2,
        Lvgl_Key_Trigger_State_Key3,
        Lvgl_Key_Trigger_State_Key4,
        Lvgl_Key_Trigger_State_Back,
        Lvgl_Key_Trigger_State_Back_Home,
        Lvgl_Key_Trigger_State_Next_Page,
        Lvgl_Key_Trigger_State_Previous_Page,
        Lvgl_Device_2_Trigger_State_Key1,
    };

    enum Window_Keyboard_Key_Trigger_State
    {
        Window_Keyboard_Key_Trigger_State_NULL = 0,
        Window_Keyboard_Key_Trigger_State_Key1,
        Window_Keyboard_Key_Trigger_State_Key2,
        Window_Keyboard_Key_Trigger_State_Key3,
        Window_Keyboard_Key_Trigger_State_Key4,
        Window_Keyboard_Key_Trigger_State_Key1_Key2,
        Window_Keyboard_Key_Trigger_State_Key1_Key3,
    };

    // Window
    uint8_t Window_Current_State = Window_Current_NULL; // 当前正在运行的窗口
    uint8_t Window_Initialization_Number = 0;           // 要窗口初始化项目的个数
    uint32_t Window_Load_Anim_Delay = 4294967295LL;     // 窗口等待动画加载延时，初始化设置最大

    // Window Flag
    bool Window_Unlock_Flag = false;
    bool Window_Load_Anim_Delay_Lock_Flag = false;

    // Window Clock
    uint8_t Window_Clock_Current_Second = 0;
    uint8_t Window_Clock_Current_Minute = 0;
    uint8_t Window_Clock_Current_Hour = 0;
    uint8_t Window_Clock_Refresh_Second = 0;
    uint8_t Window_Clock_Refresh_Minute = 0;
    uint8_t Window_Clock_Refresh_Hour = 0;

    // Window Keyboard
    uint8_t Window_Keyboard_Key_Trigger_State = Window_Keyboard_Key_Trigger_State_NULL;
    uint8_t Window_Keyboard_Key_Attribute[5] = {};
    bool Window_Keyboard_Key_Lock[3] = {};
    uint8_t Window_Keyboard_Key_Press_Delay[3] = {};

    // Lvgl Key
    uint8_t Lvgl_Key_Trigger_State = Lvgl_Key_Trigger_State_NULL;

    // other
    int LCD_Screen_Brightness = 255;

    // Device
    uint8_t Device_Number = 0;

    void LCD_CS_Set(uint8_t device_address, uint8_t lcd_cs, uint32_t delay_ms);
};

void LV_Delay(uint32_t lv_delay);
void Window_Clock_Loop(void);
void Window_Home_Loop(void);
void Window_Home_2_Loop(void);
void Window_Key_Mode_Keyboard_Loop(void);
void Window_Keyboard_CtrlCV_Initialization(void);
void Window_Keyboard_CtrlCV_Loop(void);
void Window_Keyboard_OSU_Initialization(void);
void Window_Keyboard_OSU_Loop(void);
void Window_Keyboard_Volume_Initialization(void);
void Window_Keyboard_Volume_Loop(void);
void Window_Key_Mode_Volume_Loop(void);
void Window_Key_Mode_Brightness_Loop(void);
void Window_Keyboard_Brightness_Windows_Initialization(void);
void Window_Keyboard_Brightness_Windows_Loop(void);
void Window_Keyboard_Brightness_MCU_Initialization(void);
void Window_Keyboard_Brightness_MCU_Loop(void);
void Window_Key_Mode_Game_Loop(void);
void Window_Keyboard_Game_Goole_Dinosaur(void);

void Window_Home_Data_Refresh(lv_ui *ui,
                              const lv_img_dsc_t *img_reserved, const lv_img_dsc_t *img_pressed);
void Window_Key_Mode_Data_Refresh(lv_ui *ui,
                                  const lv_img_dsc_t *img_reserved, const lv_img_dsc_t *img_pressed);
void Window_Keyboard_Data_Refresh(lv_ui *ui,
                                  const lv_img_dsc_t *img_reserved, const lv_img_dsc_t *img_pressed);

void custom_init(lv_ui *ui);

extern TFT_eSPI *tft;
extern My_Lvgl_UI My_UI;
extern USBHIDKeyboard Keyboard;
extern USBHIDConsumerControl ConsumerControl;
extern std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus;

extern uint8_t T_Keyboard_S3_Pro_Key_Trigger_Loop(void);
extern uint8_t Lvgl_Google_Dinosaur_KEY_Trigger_Loop(void);
