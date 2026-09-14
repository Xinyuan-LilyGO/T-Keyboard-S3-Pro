/*
 * @Description: None
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-07-10 19:41:08
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-01-11 16:59:55
 * @License: GPL 3.0
 */
// T-Keyboard-S3-Pro: share the driver selection with Arduino_GFX examples.
#include "../../common/pin_config.h"
#define USER_SETUP_ID 303
#define ST7735_DRIVER
#define TFT_WIDTH LCD_WIDTH
#define TFT_HEIGHT LCD_HEIGHT

#if T_KEYBOARD_S3_PRO_LCD_DRIVER == T_KEYBOARD_S3_PRO_LCD_DRIVER_GC9107
// Preserve the original GC9107-compatible TFT_eSPI setup.
#define ST7735_GREENTAB2
#elif T_KEYBOARD_S3_PRO_LCD_DRIVER == T_KEYBOARD_S3_PRO_LCD_DRIVER_ST7735
#define T_KEYBOARD_S3_PRO_ST7735_INIT
#define CGRAM_OFFSET
#endif

// #define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue
#define TFT_RGB_ORDER TFT_BGR // Colour order Blue-Green-Red

#define TFT_INVERSION_ON
// #define TFT_INVERSION_OFF

// LCD chip select and reset are managed by the keyboard initialization code.
#define TFT_MISO -1
#define TFT_MOSI LCD_MOSI
#define TFT_SCLK LCD_SCLK
#define TFT_DC LCD_DC // Data Command control pin
#define TFT_RST -1 // Reset pin (could connect to RST pin)
#define TFT_BL -1 // LED back-light

#define LOAD_GLCD  // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2 // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4 // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6 // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7 // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8 // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
// #define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

#define SPI_FREQUENCY 10000000
// #define SPI_FREQUENCY  20000000
// #define SPI_FREQUENCY 27000000
// #define SPI_FREQUENCY 40000000


#define SPI_TOUCH_FREQUENCY 2500000

// #define SUPPORT_TRANSACTIONS
