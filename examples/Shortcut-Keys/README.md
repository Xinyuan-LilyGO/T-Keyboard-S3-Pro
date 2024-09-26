## **English | [中文](./README_CN.md)**

## Example description
- This example is a custom keyboard example, through the T-Keyboard-S3-Pro to achieve the custom keyboard function, through the USB connection to the computer, the computer will automatically install the keyboard driver, and displayed as HID device, you can customize the keyboard keys, to achieve the shortcut key function.

## Principle of realization
- T-Keyboard-S3-Pro connects to the computer through USB to simulate the Keyboard device. When pressing the T-Keyboard-S3-Pro button, the program will enter 'win+R' to open the running window, enter the user's application path, and press enter to open the user's application.

## Custom usage instructions

### Change application path

- 1. Download the open source program from github, open the folder through VScode, locate and open the `platformio.ini` file, select `default_envs = Shortcut-Keys`, and comment out all the other `default_envs`.

- 2. Find and open `examples/Shortcut-Keys/ Shortcut-Keys.cpp`
Found under /*UERS Define Local APP address on PC */ comment:
    ```c
    char *APP_File1_addr = "xx.exe\n";
    ```
    Where `"xx.exe"` is the file location of the computer application.

- 3. Find the application path you want to open on the PC and copy it. Replace `"xx.exe"` with your local application path and add `\n`. (If you can't find the file path, you can right-click and select 'Open the file location')
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
<a href="https://xiazai.zol.com.cn/detail/48/475031.shtml#xiazaic-topb-box-2022" title="image2lcd download">image2lcd download </a>
</p>

- 2. Open icon_16Bit.h and comment out the first eight elements of the array so that there is no edge noise when the LCD displays the icon.
```c
const unsigned char gImage_edge[32776] = {/*0X00,0X10,0X80,0X00,0X80,0X00,0X01,0X1B*/
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
...}
```

- 3. Find the following program in the `void Iocn_Show(std::vector<unsigned char> device_id)`.
 ```c
gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_edge, 128, 128);
```
Lines 351，360，369，378  of the program.For example, replace the LCD icon of the first key:

The third parameter:`gImage_edge`, is replaced with the name of the icon array you define, which changes the icon of the first button LCD.

The fourth parameter and the fifth parameter：The size of the icon displayed here is`128*128`, so fill in the parameter `128,128`. If the image size you selected is`100*100`, change the 4th and 5th parameters to `100,100`.

- 4. `Note`: image2Lcd software can only recognize some formats of the picture, if your picture is not suitable, please use the relevant software to convert the picture to the format that image2lcd can recognize, can not change the file suffix, so displayed on the screen may display abnormal. 

### Software burning
- 1. Press and hold the BOOT button of the T-keycurt-S3-Pro, insert it into the USB port of the computer, and release the BOOT button.
Click the Build button in the lower left corner,![Build](/image/4.png)
After compiling, there is no exception, click the UPload button,![Download](/image/5.png)
Burn the program to T-keycurt-S3-pro, wait for the program to finish burning, then click the reset button.

- 2. Connect the T-Keyboard-S3-Pro to your computer, the keyboard driver will automatically be installed on your computer, and the device Manager will open (shortcut keys: `win+x`), open the Keyboard category and display 'HID Keyboard Device' device, indicating that T-Keyboard-S3-Pro has been recognized as a keyboard peripheral.