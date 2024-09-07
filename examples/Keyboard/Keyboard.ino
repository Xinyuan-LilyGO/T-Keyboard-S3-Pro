/*
 * @Description: USB Device Keyboard HID Slave Test
 * @Author: LILYGO_L
 * @Date: 2024-01-04 17:53:29
 * @LastEditTime: 2024-09-05 11:42:01
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "T-Keyboard-S3-Pro_Drive.h"
#include "pin_config.h"
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <USBHIDConsumerControl.h>
#include "Arduino_DriveBus_Library.h"

// #define KEY1_SET KEY_LEFT_CTRL
// #define KEY2_SET 'c'
// #define KEY3_SET 'v'
// #define KEY4_SET KEY_BACKSPACE
// #define KEY5_SET KEY_RETURN

#define KEY1_SET 'd'
#define KEY2_SET 'f'
#define KEY3_SET 'j'
#define KEY4_SET 'k'
#define KEY5_SET KEY_RETURN

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

uint8_t IIC_Master_Receive_Data;

size_t IIC_Bus_CycleTime = 0;
size_t KNOB_CycleTime = 0;

uint8_t KEY1_Lock = 0;
uint8_t KEY1_Press_Delay = 0;
uint8_t KEY2_Lock = 0;
uint8_t KEY2_Press_Delay = 0;
uint8_t KEY3_Lock = 0;
uint8_t KEY3_Press_Delay = 0;
uint8_t KEY4_Lock = 0;
uint8_t KEY4_Press_Delay = 0;
uint8_t KEY5_Lock = 0;
uint8_t KEY5_Press_Delay = 0;

std::vector<unsigned char> IIC_Device_ID_Registry_Scan;

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::vector<T_Keyboard_S3_Pro_Device_KEY> KEY_Trigger;

USBHIDKeyboard Keyboard;
USBHIDConsumerControl ConsumerControl;

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
        // delay(10);
    }
}

void IIC_KEY_Read_Loop(void)
{
    if (IIC_Device_ID_Registry_Scan.size() > 0)
    {
        // 扫描到的所有设备都接收数据
        for (int i = 0; i < IIC_Device_ID_Registry_Scan.size(); i++)
        {
            IIC_Bus->IIC_ReadC8_Data(IIC_Device_ID_Registry_Scan[i], T_KEYBOARD_S3_PRO_RD_KEY_TRIGGER,
                                     &IIC_Master_Receive_Data, 1);

            T_Keyboard_S3_Pro_Device_KEY key_trigger_temp;
            key_trigger_temp.ID = IIC_Device_ID_Registry_Scan[i];
            key_trigger_temp.Trigger_Data = IIC_Master_Receive_Data;

            KEY_Trigger.push_back(key_trigger_temp);
        }
    }
}

void IIC_KEY_Trigger_Loop(void)
{
    if (KEY_Trigger.size() > 0)
    {
        switch (KEY_Trigger[0].ID)
        {
        case 0x01:
            if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1)
            {
                if (KEY1_Lock == 0)
                {
                    Keyboard.press(KEY1_SET);
                }

                if (KEY1_Press_Delay >= 100)
                {
                    KEY1_Press_Delay = 100;
                    Keyboard.press(KEY1_SET);
                }
                KEY1_Press_Delay++;
                KEY1_Lock = 1;
            }
            else
            {
                Keyboard.release(KEY1_SET);
                KEY1_Press_Delay = 0;
                KEY1_Lock = 0;
            }

            if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1)
            {
                if (KEY2_Lock == 0)
                {
                    Keyboard.press(KEY2_SET);
                }

                if (KEY2_Press_Delay >= 100)
                {
                    KEY2_Press_Delay = 100;
                    Keyboard.press(KEY2_SET);
                }
                KEY2_Press_Delay++;
                KEY2_Lock = 1;
            }
            else
            {
                Keyboard.release(KEY2_SET);
                KEY2_Press_Delay = 0;
                KEY2_Lock = 0;
            }

            if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1)
            {
                if (KEY3_Lock == 0)
                {
                    Keyboard.press(KEY3_SET);
                }

                if (KEY3_Press_Delay >= 100)
                {
                    KEY3_Press_Delay = 100;
                    Keyboard.press(KEY3_SET);
                }
                KEY3_Press_Delay++;
                KEY3_Lock = 1;
            }
            else
            {
                Keyboard.release(KEY3_SET);
                KEY3_Press_Delay = 0;
                KEY3_Lock = 0;
            }

            if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1)
            {
                if (KEY4_Lock == 0)
                {
                    Keyboard.press(KEY4_SET);
                }

                if (KEY4_Press_Delay >= 100)
                {
                    KEY4_Press_Delay = 100;
                    Keyboard.press(KEY4_SET);
                }
                KEY4_Press_Delay++;
                KEY4_Lock = 1;
            }
            else
            {
                Keyboard.release(KEY4_SET);
                KEY4_Press_Delay = 0;
                KEY4_Lock = 0;
            }

            if ((KEY_Trigger[0].Trigger_Data & 0B00000001) == 1)
            {
                if (KEY5_Lock == 0)
                {
                    Keyboard.press(KEY5_SET);
                }

                if (KEY5_Press_Delay >= 100)
                {
                    KEY5_Press_Delay = 100;
                    Keyboard.press(KEY5_SET);
                }
                KEY5_Press_Delay++;
                KEY5_Lock = 1;
            }
            else
            {
                Keyboard.release(KEY5_SET);
                KEY5_Press_Delay = 0;
                KEY5_Lock = 0;
            }
            break;

        default:
            Keyboard.release(KEY1_SET);
            Keyboard.release(KEY2_SET);
            Keyboard.release(KEY3_SET);
            Keyboard.release(KEY4_SET);
            Keyboard.release(KEY5_SET);
            break;
        }
        KEY_Trigger.erase(KEY_Trigger.begin());
    }
}

void KNOB_Trigger_Loop(void)
{
    KNOB_Logical_Scan_Loop();

    if (KNOB_Trigger_Flag == true)
    {
        KNOB_Trigger_Flag = false;

        switch (KNOB_State_Flag)
        {
        case KNOB_State::KNOB_INCREMENT:
            KNOB_Data++;
            Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);

            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);

            break;
        case KNOB_State::KNOB_DECREMENT:
            KNOB_Data--;
            Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);

            ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);

            break;

        default:
            break;
        }
    }
    else
    {
        ConsumerControl.release();
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
            IIC_Bus->IIC_Device_7Bit_Scan(&IIC_Device_ID_Registry_Scan);

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

    while (IIC_Bus->begin() == false)
    {
        Serial.println("IIC_Bus initialization fail");
        delay(2000);
    }
    Serial.println("IIC_Bus initialization successfully");

    xTaskCreatePinnedToCore(Task1, "Task1", 10000, NULL, 1, NULL, 1);

    delay(1000);

    Keyboard.begin();
    ConsumerControl.begin();
    USB.begin();
}

void loop()
{
    IIC_KEY_Read_Loop();

    IIC_KEY_Trigger_Loop();

    if (millis() > KNOB_CycleTime)
    {
        KNOB_Logical_Scan_Loop();
        KNOB_CycleTime = millis() + 20; // 20ms
    }

    KNOB_Trigger_Loop();
}
