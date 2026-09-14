/*
 * Minimal all-white test extracted directly from Original_Test_2.
 * Final state: all LCDs white, all WS2812 LEDs white at brightness 10.
 */
#include <Arduino.h>
#include "Arduino_DriveBus_Library.h"
#include "T-Keyboard-S3-Pro_Drive.h"
#include "Arduino_GFX_Library.h"
#include "pin_config.h"

#define IIC_LCD_CS_DEVICE_DELAY 20
#define WS2812_BRIGHTNESS 10

std::vector<unsigned char> IIC_Device_ID_Scan;
std::vector<unsigned char> IIC_Device_ID_Registry;

bool IIC_Device_ID_State = false;
size_t IIC_Bus_CycleTime = 0;

Arduino_DataBus *bus = new Arduino_HWSPI(
    LCD_DC /* DC */, -1 /* CS */, LCD_SCLK /* SCK */,
    LCD_MOSI /* MOSI */, -1 /* MISO */);

Arduino_GFX *gfx = new Arduino_GC9107(
    bus, -1 /* RST */, 0 /* rotation */, true /* IPS */,
    LCD_WIDTH /* width */, LCD_HEIGHT /* height */,
    2 /* col offset 1 */, 1 /* row offset 1 */,
    0 /* col offset 2 */, 0 /* row offset 2 */);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Select_Screen_All(std::vector<unsigned char> device_id, bool select)
{
    if (select == true)
    {
        for (int i = 0; i < device_id.size(); i++)
        {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111);
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }
    }
    else
    {
        for (int i = 0; i < device_id.size(); i++)
        {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000);
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }
    }
}

void LCD_Initialization(std::vector<unsigned char> device_id)
{
    Select_Screen_All(device_id, true);

    // All LCD CS pins must be selected while every LCD is initialized.
    gfx->begin(4500000);
    gfx->fillScreen(BLACK);
    delay(500);

    Select_Screen_All(device_id, false);
}

void LCD_Show_All_White(std::vector<unsigned char> device_id)
{
    Select_Screen_All(device_id, true);
    gfx->fillScreen(WHITE);
    delay(100);
    Select_Screen_All(device_id, false);
}

void WS2812_Show_All_White(std::vector<unsigned char> device_id)
{
    // Enter free-color mode directly. Do not run the red/green/blue test mode.
    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(device_id[i],
                               T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000010);
    }
    delay(IIC_LCD_CS_DEVICE_DELAY);

    for (int i = 0; i < device_id.size(); i++)
    {
        IIC_Bus->IIC_WriteC8D8(
            device_id[i], T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_H,
            (uint8_t)((uint16_t)360 >> 8));
        IIC_Bus->IIC_WriteC8D8(
            device_id[i], T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_L,
            (uint8_t)360);
        IIC_Bus->IIC_WriteC8D8(
            device_id[i], T_KEYBOARD_S3_PRO_WR_LED_COLOR_STATURATION, 0);
        IIC_Bus->IIC_WriteC8D8(
            device_id[i], T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 0);
        IIC_Bus->IIC_WriteC8D8(
            device_id[i], T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B10000000);
        IIC_Bus->IIC_WriteC8D8(
            device_id[i], T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111);
        IIC_Bus->IIC_WriteC8D8(
            device_id[i], T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111);
        IIC_Bus->IIC_WriteC8D8(
            device_id[i], T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000);
    }

    for (int brightness = 0; brightness <= WS2812_BRIGHTNESS; brightness++)
    {
        for (int i = 0; i < device_id.size(); i++)
        {
            IIC_Bus->IIC_WriteC8D8(
                device_id[i], T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, brightness);
            IIC_Bus->IIC_WriteC8D8(
                device_id[i], T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111);
            IIC_Bus->IIC_WriteC8D8(
                device_id[i], T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111);
            IIC_Bus->IIC_WriteC8D8(
                device_id[i], T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000);
        }
        delay(20);
    }
}

void Task1(void *pvParameters)
{
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

            IIC_Bus_CycleTime = millis() + 10;
        }
        delay(1);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting all-white test");

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
    ledcWrite(1, 0);

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
        if (IIC_Device_ID_State == true && IIC_Device_ID_Scan.empty() == false)
        {
            delay(100);
            IIC_Device_ID_Registry = IIC_Device_ID_Scan;

            Serial.printf("Found %u I2C device(s):",
                          (unsigned int)IIC_Device_ID_Registry.size());
            for (int i = 0; i < IIC_Device_ID_Registry.size(); i++)
            {
                Serial.printf(" 0x%02X", IIC_Device_ID_Registry[i]);
            }
            Serial.println();

            LCD_Initialization(IIC_Device_ID_Registry);
            IIC_Device_ID_State = false;
            break;
        }

        Serial.println("IIC ID not found");
        delay(1000);
    }

    LCD_Show_All_White(IIC_Device_ID_Registry);
    WS2812_Show_All_White(IIC_Device_ID_Registry);
    LCD_Show_All_White(IIC_Device_ID_Registry);

    Serial.println("All LCDs and WS2812 LEDs are white");
}

void loop()
{
    delay(1000);
}
