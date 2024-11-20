<!--
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2023-09-11 16:13:14
 * @LastEditTime: 2024-11-20 10:08:04
 * @License: GPL 3.0
-->
<h1 align = "center">T-Keyboard-S3-Pro_ESP32S3</h1>

## **[English](./README.md) | 中文**

## 目录
- [描述](#描述)
- [预览](#预览)
- [快速开始](#软件部署)
- [引脚总览](#引脚总览)
- [相关测试](#相关测试)

## 描述

此处为T-Keyboard-S3-Pro板子的ESP32S3软件配置分支，您可以使用Arduino IDE或者PlatformIO IDE来编译他。

不同分支有不同的编译库版本，这里需要注意一下。

## 预览

### PCB板

### 渲染图

## 软件部署

### 示例支持

| Example | Description | Picture |
| ------  | ------ | ------ | 
| [GFX](./examples/GFX) |  |  |
| [IIC_Scan_2](./examples/IIC_Scan_2) |  |  |
| [Keyboard](./examples/Keyboard) |  |  |
| [Original_Test](./examples/Original_Test) |  |  |
| [Original_Test_2](./examples/Original_Test_2) | 出厂初始测试文件 |  |
| [Rotary_Encoder](./examples/Rotary_Encoder) |  |  |
| [T-Keyboard-S3-Pro_IIC_Command](./examples/T-Keyboard-S3-Pro_IIC_Command) |  |  |
| [T-Keyboard-S3-Pro_IIC_Scan](./examples/T-Keyboard-S3-Pro_IIC_Scan) |  |  |
| [Shortcut-Keys](./examples/Shortcut-Keys) |  |  |
| [Lvgl_UI](./examples/Lvgl_UI) |  |  |

| Firmware | Description | Picture |
| ------  | ------  | ------ |
| [Original_Test_2](./firmware/[ESP32S3][T-Keyboard-S3-Pro_V1.0][Original_Test_2]_firmware_V1.0.0.bin) | 主机设备出厂初始测试文件 |  |
| [Lvgl_UI](./firmware/[ESP32S3][T-Keyboard-S3-Pro_V1.0][Lvgl_UI]_firmware_V1.0.0.bin) | 主机设备出厂初始测试文件 |  |

### IDE

#### PlatformIO
1. 安装[VisualStudioCode](https://code.visualstudio.com/Download)，根据你的系统类型选择安装。

2. 打开VisualStudioCode软件侧边栏的“扩展”（或者使用<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>X</kbd>打开扩展），搜索“PlatformIO IDE”扩展并下载。

3. 在安装扩展的期间，你可以前往GitHub下载程序，你可以通过点击带绿色字样的“<> Code”下载主分支程序，也通过侧边栏下载“Releases”版本程序。

4. 扩展安装完成后，打开侧边栏的资源管理器（或者使用<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>E</kbd>打开），点击“打开文件夹”，找到刚刚你下载的项目代码（整个文件夹），点击“添加”，此时项目文件就添加到你的工作区了。

5. 打开项目文件中的“platformio.ini”（添加文件夹成功后PlatformIO会自动打开对应文件夹的“platformio.ini”）,在“[platformio]”目录下取消注释选择你需要烧录的示例程序（以“default_envs = xxx”为标头），然后点击左下角的“<kbd>[√](image/4.png)</kbd>”进行编译，如果编译无误，将单片机连接电脑，点击左下角“<kbd>[→](image/5.png)</kbd>”即可进行烧录。

#### Arduino
1. 安装[Arduino](https://www.arduino.cc/en/software)，根据你的系统类型选择安装。

2. 打开项目文件夹的“example”目录，选择示例项目文件夹，打开以“.ino”结尾的文件即可打开Arduino IDE项目工作区。

3. 打开右上角“工具”菜单栏->选择“开发板”->“开发板管理器”，找到或者搜索“esp32”，下载作者名为“Espressif Systems”的开发板文件。接着返回“开发板”菜单栏，选择“ESP32 Arduino”开发板下的开发板类型，选择的开发板类型由“platformio.ini”文件中以[env]目录下的“board = xxx”标头为准，如果没有对应的开发板，则需要自己手动添加项目文件夹下“board”目录下的开发板。

4. 打开菜单栏“[文件](image/6.png)”->“[首选项](image/6.png)”，找到“[项目文件夹位置](image/7.png)”这一栏，将项目目录下的“libraries”文件夹里的所有库文件连带文件夹复制粘贴到这个目录下的“libraries”里边。

5. 在 "工具 "菜单中选择正确的设置，如下表所示。

##### ESP32-S3
| Setting                               | Value                                 |
| :-------------------------------: | :-------------------------------: |
| Board                                 | ESP32S3 Dev Module           |
| Upload Speed                     | 921600                               |
| USB Mode                           | Hardware CDC and JTAG     |
| USB CDC On Boot                | Enabled                              |
| USB Firmware MSC On Boot | Disabled                             |
| USB DFU On Boot                | Disabled                             |
| CPU Frequency                   | 240MHz (WiFi)                    |
| Flash Mode                         | QIO 80MHz                         |
| Flash Size                           | 16MB (128Mb)                    |
| Core Debug Level                | None                                 |
| Partition Scheme                | 16M Flash (3MB APP/9.9MB FATFS) |
| PSRAM                                | OPI PSRAM                         |
| Arduino Runs On                  | Core 1                               |
| Events Run On                     | Core 1                               |        

6. 选择正确的端口。

7. 点击右上角“<kbd>[√](image/8.png)</kbd>”进行编译，如果编译无误，将单片机连接电脑，点击右上角“<kbd>[→](image/9.png)</kbd>”即可进行烧录。

### firmware烧录
1. 打开项目文件“tools”找到ESP32烧录工具，打开。

2. 选择正确的烧录芯片以及烧录方式点击“OK”，如图所示根据步骤1->2->3->4->5即可烧录程序，如果烧录不成功，请按住“BOOT-0”键再下载烧录。

3. 烧录文件在项目文件根目录“[firmware](./firmware/)”文件下，里面有对firmware文件版本的说明，选择合适的版本下载即可。

<p align="center" width="100%">
    <img src="image/10.png" alt="example">
    <img src="image/11.png" alt="example">
</p>


## 引脚总览

| IIC_1引脚  | ESP32S3引脚|
| :------------------: | :------------------:|
| SDA         | IO42     |
| SCL         | IO2       |

| IIC_2引脚（外扩）  | ESP32S3引脚|
| :------------------: | :------------------:|
| SDA         | IO6       |
| SCL         | IO7       |

| LCD屏幕引脚  | ESP32S3引脚|
| :------------------: | :------------------:|
| MOSI         | IO40       |
| SCLK         | IO41       |
| DC         | IO39       |
| RST         | IO38       |
| BL         | IO1       |

| 旋转编码器引脚  | ESP32S3引脚|
| :------------------: | :------------------:|
| KNOB_DATA_A         | IO4       |
| KNOB_DATA_B         | IO5       |

## 相关测试
