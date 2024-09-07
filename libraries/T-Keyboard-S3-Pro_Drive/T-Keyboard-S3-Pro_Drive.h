/*
 * @Description: None
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-12-28 14:08:27
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-07-17 10:59:13
 * @License: GPL 3.0
 */
#pragma once

#include <iostream>
#include <vector>

#define T_KEYBOARD_S3_PRO_WR_LCD_CS 0x01
#define T_KEYBOARD_S3_PRO_RD_KEY_TRIGGER 0x02
#define T_KEYBOARD_S3_PRO_WR_LED_MODE 0x03
#define T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS 0x04
#define T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_H 0x05
#define T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_L 0x06
#define T_KEYBOARD_S3_PRO_WR_LED_COLOR_STATURATION 0x07
#define T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1 0x08
#define T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2 0x09
#define T_KEYBOARD_S3_PRO_RD_DRIVE_FIRMWARE_VERSION 0x10

struct T_Keyboard_S3_Pro_Device_KEY
{
    uint8_t ID;
    uint8_t Trigger_Data;
};

class T_Keyboaed_S3_Drive
{
public:

};
