/*
 * @Description: Search for the current address of the T-Keyboard-S3-Pro slave device.
 * @Author: LILYGO_L
 * @Date: 2023-12-26 16:20:20
 * @LastEditTime: 2024-09-05 13:50:26
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    while (IIC_Bus->begin() == false)
    {
        Serial.println("IIC_Bus initialization fail");
        delay(2000);
    }
    Serial.println("IIC_Bus initialization successfully");
}

void loop()
{
    std::vector<unsigned char> iic_device_address;

    if (IIC_Bus->IIC_Device_7Bit_Scan(&iic_device_address) == true)
    {
        for (int i = 0; i < iic_device_address.size(); i++)
        {
            Serial.printf("Find IIC_ID[%d]: 0x", i);
            if (iic_device_address[i] < 0x10)
            {
                Serial.print("0");
                Serial.print(iic_device_address[i], HEX);
            }
            else
            {
                Serial.print(iic_device_address[i], HEX);
            }
            Serial.println();
        }
    }

    delay(3000);
}