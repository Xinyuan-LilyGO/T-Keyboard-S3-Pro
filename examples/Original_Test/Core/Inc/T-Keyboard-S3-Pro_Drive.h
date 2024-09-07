/*
 * @Description: None
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-12-28 14:08:27
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2023-12-28 15:26:18
 * @License: GPL 3.0
 */
#ifndef __T_KEYBOARD_S3_PRO_DRIVE_H
#define __T_KEYBOARD_S3_PRO_DRIVE_H

#include "Binary.h"

struct T_Keyboard_S3_Pro
{
    uint8_t KEY_Trigger_Flag;   // 按键触发标志
    uint8_t KEY_Trigger_Number; // 按键触发号，使用0Bxxxxxxxx来标志
};

extern struct T_Keyboard_S3_Pro My_Device;

#endif
