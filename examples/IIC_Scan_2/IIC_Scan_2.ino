/*
 * @Description: IIC device scanning test
 * @Author: LILYGO_L
 * @Date: 2023-12-26 14:59:18
 * @LastEditTime: 2024-09-05 11:38:31
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Wire.h>
#include "pin_config.h"

#define SDA IIC_SDA
#define SCL IIC_SCL
// #define SDA IIC_SDA_2
// #define SCL IIC_SCL_2

void scan_i2c_device(TwoWire &i2c)
{
    Serial.println("Scanning for I2C devices ...");
    Serial.print("      ");
    for (int i = 0; i < 0x10; i++)
    {
        Serial.printf("0x%02X|", i);
    }
    uint8_t error;
    for (int j = 0; j < 0x80; j += 0x10)
    {
        Serial.println();
        Serial.printf("0x%02X |", j);
        for (int i = 0; i < 0x10; i++)
        {
            Wire.beginTransmission(i | j);
            error = Wire.endTransmission();
            if (error == 0)
                Serial.printf("0x%02X|", i | j);
            else
                Serial.print(" -- |");
        }
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    Wire.begin(SDA, SCL);
    scan_i2c_device(Wire);
}

void loop()
{
    scan_i2c_device(Wire);
    delay(5000);
}