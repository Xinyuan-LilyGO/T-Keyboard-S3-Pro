## **[English](./README.md) | 中文**

## 示例描述
- 该实例为自定义键盘示例，通过T-Keyboard-S3-Pro实现自定义键盘功能，通过USB连接电脑，电脑上会自动安装键盘驱动，并显示为HID设备，可以自定义键盘按键，实现快捷键功能。

## 实现原理
- T-Keyboard-S3-Pro通过USB连接电脑，模拟键盘设备，当按下T-Keyboard-S3-Pro按键时，程序会输入`win+R`打开运行窗口，输入用户的应用程序路径，并回车，即可打开用户的应用程序。

## 自定义使用说明

### 更换应用路径

- 1. 从github上下载开源程序，通过VScode打开文件夹，找到并打开`platformio.ini`文件，选择`default_envs = Shortcut-Keys`，其他`default_envs`全注释掉。

- 2. 找到并打开`examples/Shortcut-Keys/Shortcut-Keys.cpp`
    在/*UERS Define Local APP address on PC */注释下面找到:
    ```c
    char *APP_File1_addr = "xx.exe\n";
    ```
    其中`"xx.exe"`为电脑应用程序的文件位置。

- 3. 找到PC端需要打开的应用路径并复制，将替换`"xx.exe"`替换为你的本地应用路径，加`\n`即可。(找不到文件路径可以右键鼠标选择`打开所在文件位置`)
    例如：
    ```c
    char *APP_File1_addr = "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe\n"
    ```
    
- 4. 注意：文件夹之间用`\\`隔开，复制文件路径的时候只有`\`,请自行添加。
    该路径必须为英文，不可出现其他的字符，否则无法打开。

### 更换LED屏幕显示
- 1. 准备显示的图标或者图片,这里演示的图片大小为`128*128`，推荐使用image2Lcd软件将图片或图标转为数组形式，或者你也可以用其他软件将图片转为数组形式，将数组复制到icon_16Bit.h文件中并保存。

- 2. 打开icon_16Bit.h,注释掉数组前八个元素，这样LCD显示图标的时候不会出现边缘噪声。
```c
const unsigned char gImage_edge[32776] = {/*0X00,0X10,0X80,0X00,0X80,0X00,0X01,0X1B*/
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
...}
```
<p>
<img src="/image/image2lcd.png" alt="Stanford-Alpaca" style="width: 70%; min-width: 100px; display: block; margin: auto;">    
</p>
<a href="https://xiazai.zol.com.cn/detail/48/475031.shtml#xiazaic-topb-box-2022" title="image2lcd download">image2lcd download</a>

- 2. 找到函数`void Iocn_Show(std::vector<unsigned char> device_id)`中的以下函数：
```c
gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_edge, 128, 128);
```
分别在程序的第351,60,369,378行，以更换第一个按键LCD图标举例：

第三个参数`gImage_edge`:参数替换为你自己定义的图标数组名称即改变第一个按键的LCD的图标。

第四个参数和第五个参数:显示的图标大小，这里图片大小为`128*128`，所以填写参数为`128,128`。如果你选择的图片大小为`100*100`,请将第4个和第5个参数修改为`100,100`。


- 4. 注意：image2Lcd软件只能识别一些格式的图片，如果你的图片不适合，请通过相关软件将图片转为image2lcd能识别的格式，不能改文件后缀名，那样显示到屏幕可能会显示不正常。


## 软件烧录
- 1. 按住T-Keyboard-S3-Pro的BOOT(旋钮)键，插入电脑USB端口后松开BOOT(旋钮)键，
点击左下角Build按钮，![编译](/image/4.png)
编译好后无异常,在点击UPload按钮，![下载](/image/5.png)
烧录程序到T-Keyboard-S3-Pro中，等待程序烧录完成,在点击reset按键即可。

- 2. 将T-Keyboard-S3-Pro连接到电脑上，电脑上会自动安装键盘驱动，打开设备管理器(快捷键：`win+x`)，打开键盘类别，显示为`HID Keyboard Device`设备，则表明T-Keyboard-S3-Pro已经被识别为键盘外设。