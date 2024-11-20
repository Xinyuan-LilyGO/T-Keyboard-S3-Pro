## **English | [中文](./README_CN.md)**

## Example description
- This example is a custom keyboard example, through the T-Keyboard-S3-Pro to achieve the custom keyboard function, through the USB connection to the computer, the computer will automatically install the keyboard driver, and displayed as HID device, you can customize the keyboard keys, to achieve the shortcut key function.

## Principle of realization
- T-Keyboard-S3-Pro connects to the computer through USB to simulate the Keyboard device. When pressing the T-Keyboard-S3-Pro button, the program will enter 'win+R' to open the running window, enter the user's application path, and press enter to open the user's application.

## Custom usage instructions

### Change application path

- 1. Download the open source program from github, open the folder through VScode, locate and open the `platformio.ini` file, select `default_envs = Shortcut-Keys`, and comment out all the other `default_envs`.

- 2. Find and open file`examples/Shortcut-Keys/KNOB-Shortcut-Keys.cpp`
    Find below `/*UERS Define Local APP address on PC */`comments:
    ```c
    char *APP_File1_addr = "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe\n";
    char *APP_File2_addr = "C:\\Program Files (x86)\\Tencent\\QQMusic\\QQMusic\n";
    char *APP_File3_addr = "C:\\Program Files (x86)\\360\\Total Security\\QHSafeMain\n";
    char *APP_File4_addr = "notepad.exe\n";
    ```
`APP_File1_addr` and `ICON_KEY1` correspond to the local file location and icon of an application. If you want to replace these two data, please replace them.

- 3. Find the application path you want to open on the PC and copy it, replacing `APP_File(x)_addr` with your local application path and adding `\n`. (If you can't find the file path, you can right-click and select 'Open the file location')
    eg：
    ```c
    char *APP_File1_addr = "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe\n"
    ```

- 4. `Note`: Folders are separated by `\\`, only `\` when copying the file path, please add it yourself.
    The path must be in English, no other characters, otherwise it cannot be opened.

### Replace the LED screen display

- 1. Ready to display the icon or picture, the size of the picture demonstrated here is `128*128`, it is recommended to use image2Lcd software to convert the image or icon into array form, or you can also use other software to convert the image into array form, copy the array to icon_16Bit.h file and save.
<p>
<img src="/image/image2lcd.png" alt="Stanford-Alpaca" style="width: 70%; min-width: 100px; display: block; margin: auto;">
</p>

- 2. Open icon_16Bit.h and comment out the first eight elements of the array so that there is no edge noise when the LCD displays the icon.
```c
const unsigned char gImage_edge[32776] = {/*0X00,0X10,0X80,0X00,0X80,0X00,0X01,0X1B*/
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
...}
```
`image2lcd` in the `tool` folder, fetch yourself.

- 3. Found under the `/*UERS Define Local APP address on PC */` comment:
```c
    uint16_t *ICON_KEY1 = (uint16_t *)gImage_edge;
    uint16_t *ICON_KEY2 = (uint16_t *)gImage_qqmusic;
    uint16_t *ICON_KEY3 = (uint16_t *)gImage_360;
    uint16_t *ICON_KEY4 = (uint16_t *)gImage_note;
```
Assign the icon array produced by icon_16Bit.h to variables such as` ICON_KEY(x) `according to your needs.

`APP_File1_addr` and `ICON_KEY1` correspond to the local file location and icon of an application. If you want to replace these two data, please replace them.

- 4. If the icon you export is not `128*128` size, please modify the code according to the following action, find the following code:
```c
void Iocn_Show(std::vector<unsigned char> device_id, uint8_t Show_LCD)
{
    if (Show_LCD & 0x01)
    {
        for (int i = 0; i < device_id.size(); i++)
        {
            IIC_Bus->IIC_WriteC8D8(device_id[i],
                                   T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00010000); // 选定屏幕1 LCD_1
            delay(IIC_LCD_CS_DEVICE_DELAY);
        }
        gfx->draw16bitRGBBitmap(0, 0, ICON_KEY1, 128, 128);
        delay(10);
    }
...
}
```
Take the first key1 for example,where` gfx->draw16bitRGBBitmap(0,0, ICON_KEY1, 128, 128); `
The fourth parameter and the fifth parameter：The size of the icon displayed here is`128*128`, so fill in the parameter `128,128`. If the image size you selected is`100*100`, change the 4th and 5th parameters to `100,100`.

- 5. `Note`: image2Lcd software can only recognize some formats of the picture, if your picture is not suitable, please use the relevant software to convert the picture to the format that image2lcd can recognize, can not change the file suffix, so displayed on the screen may display abnormal. 

### Software burning
- 1. Press and hold the BOOT button of the T-keycurt-S3-Pro, insert it into the USB port of the computer, and release the BOOT button.
Click the Build button in the lower left corner,![Build](/image/4.png)
After compiling, there is no exception, click the UPload button,![Download](/image/5.png)
Burn the program to T-keycurt-S3-pro, wait for the program to finish burning, then click the reset button.

- 2. Connect the T-Keyboard-S3-Pro to your computer, the keyboard driver will automatically be installed on your computer, and the device Manager will open (shortcut keys: `win+x`), open the Keyboard category and display 'HID Keyboard Device' device, indicating that T-Keyboard-S3-Pro has been recognized as a keyboard peripheral.

##  Q&A  
- 1. How do I get the icon file for application ICONS? There are two ways to get your app icon:

        a. Locate iconViewer3.02 under the tools folder in this directory and open it, unzip the file, and install iconViewer3.02 depending on your PC's x64 or x86 version
<img src="/image/d.png" alt="Stanford-Alpaca" style="width: 70%; min-width: 100px; display: block; margin: auto;">    
Once installed, then right-click on your app icon on your computer and go to the location of the file
<img src="/image/a.png" alt="Stanford-Alpaca" style="width: 50%; min-width: 100px; display: block; margin: auto;"> 
exe file right-click property will be found
<img src="/image/b.png" alt="Stanford-Alpaca" style="width: 70%; min-width: 100px; display: block; margin: auto;"> 
If you can see the Icon icon in the ICON bar, you can save it
<img src="/image/c.png" alt="Stanford-Alpaca" style="width: 70%; min-width: 100px; display: block; margin: auto;"> 
        b. Download the icon icon of the corresponding application from the ICON website.