/*
 * @Description: None
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-08-29 13:34:55
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-05-23 10:29:55
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "pin_config.h"

uint8_t i = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(GPIO_NUM_38, OUTPUT);

    digitalWrite(GPIO_NUM_38, i);
}

void loop()
{
    digitalWrite(GPIO_NUM_38, i);
    i = !i;
    delay(3000);
}