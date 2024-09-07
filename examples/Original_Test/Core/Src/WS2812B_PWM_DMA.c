/*
 * @Description: None
 * @version: None
 * @Author: None
 * @Date: 2022-10-18 00:14:49
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-07-17 09:12:54
 */
#include "string.h"
#include "tim.h"
#include "WS2812B_PWM_DMA.h"

#define TIM_Handle htim3
#define TIM_CHANNEL TIM_CHANNEL_2

#define CODE_1 53          // 1 码所需的占空比(按80来切分)
#define CODE_0 27          // 0 码所需的占空比(按80来切分)
#define RESET_PULSE 225    // 大于225个复位电平
#define NUM_LEDS 14        // Led总个数
#define LED_DATA_LENGTH 24 // 单个Led数据长度,WS2812B单个Led一般需要24个字节

unsigned short static RGB_Buffur[RESET_PULSE + (NUM_LEDS * LED_DATA_LENGTH)] = {0};
unsigned short static RESET_Buffur[RESET_PULSE] = {0};

/////////////////////////////////////////////////////////////////////////
// Bitmap点阵数据表                                                    //
// 图片:横向8点取模左高位,数据排列:从左到右从上到下   //
// 图片尺寸: 8 * 8                                                     //
/////////////////////////////////////////////////////////////////////////
unsigned char Data1[] = // 数据表【C】
    {0x00, 0x1C, 0x20, 0x20, 0x20, 0x22, 0x1C, 0x00};
unsigned char Data2[] = // 数据表【i】
    {0x00, 0x08, 0x00, 0x08, 0x08, 0x08, 0x08, 0x00};
unsigned char Data3[] = // 数据表【a】
    {0x00, 0x38, 0x04, 0x1C, 0x24, 0x24, 0x1A, 0x00};
unsigned char Data4[] = // 数据表【l】
    {0x00, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00};
unsigned char Data5[] = // 数据表【o】
    {0x00, 0x18, 0x24, 0x24, 0x24, 0x24, 0x18, 0x00};

/////////////////////////////////////////////////////////////////////////
// Bitmap点阵数据表                                                    //
// 图片:纵向8点取模下高位,数据排列:从左到右从上到下   //
// 图片尺寸: 128 * 8                                                   //
/////////////////////////////////////////////////////////////////////////
unsigned short Data6[] = // 数据表【Ciallo】
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x24, 0x54, 0x54, 0x38, 0x40, 0x00, 0x00, 0x00,
     0x00, 0x41, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x41, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00,
     0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/**
 * @brief 翻转数组内部数据函数
 *              步骤1.翻转数组的前/后n位元素(如需左移取前,如需右移取后)
 *              步骤2.翻转数组剩下的元素
 *              步骤3.再翻转整个数组,然后就实现了循环左移n位的功能
 * @param Array 要翻转操作的数组
 * @param Left 数组的左边的数据位(Left(0)->Right(x))
 * @param Right 数组的右边的数据位(Left(0)->Right(x))
 * @return
 * @Date 2022-10-20 17:38:09
 */
void Reverse_Array(unsigned short *p, unsigned short Left, unsigned short Right)
{
    for (unsigned short L = Left, R = Right; L < R; L++, R--)
    {
        p[L] = p[L] ^ p[R]; // 任何数与0异或的结果都是它本身,任何数与1异或的结果都是它的相反数
        p[R] = p[L] ^ p[R]; // L ^ R ^ R = L ^ 0 = L
        p[L] = p[L] ^ p[R]; // L ^ R ^ L = R ^ 0 = R
    }
}

/**
 * @brief 灯珠位码数组平移函数1,可左可右效率较低(异或原理)
 * @param Direction 平移灯珠的方向
 * @param N 平移灯珠的个数
 * @return
 * @Date 2022-10-21 08:55:18
 */
void WS2812B_Shift1(unsigned char Direction, unsigned char N)
{
    // 跳过复位电平
    unsigned short *p = (RGB_Buffur + RESET_PULSE);

    if (Direction == Shift_Left)
    {
        // RGB一共24个元素(0x00,因为是用PWM+DMA发数据,发的是0x00来代表1和0位所以才会有这么多元素)所以要N*LED_DATA_LENGTH
        Reverse_Array(p, 0, (N * LED_DATA_LENGTH - 1)); // 这里以及下面的-1是因为这里的参数是填入数组里的,数组从0开始,总是会多一个元素0x00,所以要-1
        Reverse_Array(p, (N * LED_DATA_LENGTH), (NUM_LEDS * LED_DATA_LENGTH - 1));
        Reverse_Array(p, 0, (NUM_LEDS * LED_DATA_LENGTH - 1));
    }
    if (Direction == Shift_Right)
    {
        // 数据往右移动,N越大所翻转的元素就越来越少,与往左移正好相反,N越大左移动所翻转的元素就越多
        Reverse_Array(p, 0, (NUM_LEDS * LED_DATA_LENGTH - N * LED_DATA_LENGTH - 1)); // 这里以及下面的-1是因为这里的参数是填入数组里的,数组从0开始,总是会多一个元素0x00,所以要-1
        Reverse_Array(p, (NUM_LEDS * LED_DATA_LENGTH - N * LED_DATA_LENGTH), (NUM_LEDS * LED_DATA_LENGTH - 1));
        Reverse_Array(p, 0, (NUM_LEDS * LED_DATA_LENGTH - 1));
    }
}

/**
 * @brief 灯珠位码数组平移函数2,可左可右效率较高(memmove代码操作内存原理)
 * @param Direction 平移灯珠的方向
 * @param N 平移灯珠的个数
 * @return
 * @Date 2022-10-21 08:55:18
 */
void WS2812B_Shift2(unsigned char Direction, unsigned char N)
{
    // 跳过复位电平
    unsigned short *p = (RGB_Buffur + RESET_PULSE);
    unsigned short Buffer[480] = {0}; // 【特别注意】只能设480,设置多了程序会出现硬件故障

    if (Direction == Shift_Left)
    {
        for (unsigned short i = 0; i < (N * LED_DATA_LENGTH); i++)
        {
            Buffer[i] = p[i];
        }
        memmove(p, p + (N * LED_DATA_LENGTH), ((NUM_LEDS * LED_DATA_LENGTH) - (N * LED_DATA_LENGTH)) * sizeof(unsigned short));
        for (unsigned short j = 0; j < (N * LED_DATA_LENGTH); j++)
        {
            p[(NUM_LEDS * LED_DATA_LENGTH) - (N * LED_DATA_LENGTH) + j] = Buffer[j];
        }
    }
    if (Direction == Shift_Right)
    {
        for (unsigned short i = 0; i < (N * LED_DATA_LENGTH); i++)
        {
            Buffer[i] = p[(NUM_LEDS * LED_DATA_LENGTH) - (N * LED_DATA_LENGTH) + i];
        }
        memmove(p + (N * LED_DATA_LENGTH), p, ((NUM_LEDS * LED_DATA_LENGTH) - (N * LED_DATA_LENGTH)) * sizeof(unsigned short));
        for (unsigned short j = 0; j < (N * LED_DATA_LENGTH); j++)
        {
            p[j] = Buffer[j];
        }
    }
}

/**
 * @brief 将HSV颜色空间转换为RGB颜色空间
 *            因为HSV使用起来更加直观 方便,所以代码逻辑部分使用HSV.但WS2812B RGB-LED灯珠的驱动使用的是RGB,所以需要转换.
 *
 * @param  H HSV颜色空间的H:色调(单位°),范围0~360,(Hue 调整颜色,0°-红色,120°-绿色,240°-蓝色,以此类推)
 * @param  S HSV颜色空间的S:饱和度(单位%),范围0~100,(Saturation 饱和度高,颜色深而艳;饱和度低,颜色浅而发白)
 * @param  V HSV颜色空间的V:明度(单位%),范围0~100,(Value 控制明暗,明度越高亮度越亮,越低亮度越低)
 * @param  *R RGB-R值的指针
 * @param  *G RGB-G值的指针
 * @param  *B RGB-B值的指针
 * @return
 * @Date 2022-10-21 08:59:44
 */
void Led_HSVToRGB(unsigned int H, unsigned int S, unsigned int V, unsigned int *R, unsigned int *G, unsigned int *B)
{
    H %= 360; // H -> [0,360]
    unsigned int rgb_max = V * 2.55f;
    unsigned int rgb_min = rgb_max * (100 - S) / 100.0f;

    unsigned int i = H / 60;
    unsigned int diff = H % 60;

    // RGB adjustment amount by hue
    unsigned int rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i)
    {
    case 0:
        *R = rgb_max;
        *G = rgb_min + rgb_adj;
        *B = rgb_min;
        break;
    case 1:
        *R = rgb_max - rgb_adj;
        *G = rgb_max;
        *B = rgb_min;
        break;
    case 2:
        *R = rgb_min;
        *G = rgb_max;
        *B = rgb_min + rgb_adj;
        break;
    case 3:
        *R = rgb_min;
        *G = rgb_max - rgb_adj;
        *B = rgb_max;
        break;
    case 4:
        *R = rgb_min + rgb_adj;
        *G = rgb_min;
        *B = rgb_max;
        break;
    default:
        *R = rgb_max;
        *G = rgb_min;
        *B = rgb_max - rgb_adj;
        break;
    }
}

void Led_RGBToHSV(uint16_t *h, uint16_t *s, uint16_t *v, uint8_t r, uint8_t g, uint8_t b)
{
}

/**
 * @brief 单独设置灯珠R,G,B属性函数(设置前需要调用ClearAll清屏函数一次(让所有灯熄灭一次)让程序知道你的板子上一共有多少个灯珠,
 *           设置完毕后需在最后加上Show显示刷新函数)
 * @param R 设置R颜色参数(格式0x00)
 * @param G 设置G颜色参数(格式0x00)
 * @param B 设置B颜色参数(格式0x00)
 * @param Location 单个灯珠位置
 * @return
 * @Date 2022-10-18 20:46:57
 */
void WS2812B_SetRGB(unsigned char R, unsigned char G, unsigned char B, unsigned short Location)
{
    // 指针地址赋值:跳过复位电平地址,定位到需要操作的灯的位置
    unsigned short *p = (RGB_Buffur + RESET_PULSE) + (Location * LED_DATA_LENGTH);

    for (unsigned char i = 0; i < 8; i++)
    {
        // 指针地址定位操作数据
        p[i] = (G << i) & (0x80) ? CODE_1 : CODE_0;
        p[i + 8] = (R << i) & (0x80) ? CODE_1 : CODE_0;
        p[i + 16] = (B << i) & (0x80) ? CODE_1 : CODE_0;
    }
}

/**
 * @brief 单独设置灯珠颜色属性函数(设置前需要调用ClearAll清屏函数一次(让所有灯熄灭一次)让程序知道你的板子上一共有多少个灯珠,
 *           设置完毕后需在最后加上Show显示刷新函数)
 * @param RGB RGB颜色参数(格式0x000000)
 * @param Location 单个灯珠位置
 * @return
 * @Date 2022-10-18 20:49:19
 */
void WS2812B_SetColour(unsigned int RGB, unsigned short Location)
{
    unsigned int GRB = 0;
    // 指针地址赋值:跳过复位电平地址,定位到需要操作的灯的位置
    unsigned short *p = (RGB_Buffur + RESET_PULSE) + (Location * LED_DATA_LENGTH);

    GRB = ((RGB & 0x00FF00) << 8) + ((RGB & 0xFF0000) >> 8) + (RGB & 0x0000FF);
    for (unsigned char i = 0; i < 24; i++)
    {
        // 指针地址定位操作数据
        p[i] = (GRB << i) & (0x800000) ? CODE_1 : CODE_0;
    }
}

/**
 * @brief 单独设置灯珠HSV属性函数(设置前需要调用ClearAll清屏函数一次(让所有灯熄灭一次)让程序知道你的板子上一共有多少个灯珠,
 *           设置完毕后需在最后加上Show显示刷新函数)
 * @param H 设置H色调参数(格式0-360)
 * @param S 设置S饱和度参数(格式0-100)
 * @param V 设置V明度参数(格式0-100)
 * @param Location
 * @return
 * @Date 2022-10-19 22:52:06
 */
void WS2812B_SetHSV(unsigned short H, unsigned short S, unsigned short V, unsigned short Location)
{
    unsigned int R = 0, G = 0, B = 0;
    Led_HSVToRGB(H, S, V, &R, &G, &B);

    // 指针地址赋值:跳过复位电平地址,定位到需要操作的灯的位置
    unsigned short *p = (RGB_Buffur + RESET_PULSE) + (Location * LED_DATA_LENGTH);

    for (unsigned char i = 0; i < 8; i++)
    {
        // 指针地址定位操作数据
        p[i] = (G << i) & (0x80) ? CODE_1 : CODE_0;
        p[i + 8] = (R << i) & (0x80) ? CODE_1 : CODE_0;
        p[i + 16] = (B << i) & (0x80) ? CODE_1 : CODE_0;
    }
}

/**
 * @brief 灯珠刷新函数
 * @return
 * @Date 2022-10-18 20:51:10
 */
void WS2812B_Show(void)
{
    // CCR捕获比较器的值,0%占空比(因为ARR设置100,所以占空比为0-100个数字,填0就是0%占空比)
    // DMA每一个定时器周期就搬运一个Pulse数组里正在搬运的值(比如说搬运0的时候就设置0%占空比)到定时器CCR中,
    // 定时器将产生一个1.25us的全低电平,225个循环总计时281.25us>280us,这个281.25us的低电平作为WS2812B的复位信号
    HAL_TIM_PWM_Start_DMA(&TIM_Handle, TIM_CHANNEL, (unsigned int *)RGB_Buffur, RESET_PULSE + (NUM_LEDS * LED_DATA_LENGTH));
}

/**
 * @brief 灯珠复位函数
 * @return
 * @Date 2022-10-18 20:51:29
 */
void WS2812B_Reset(void)
{
    HAL_TIM_PWM_Start_DMA(&TIM_Handle, TIM_CHANNEL, (unsigned int *)RESET_Buffur, RESET_PULSE);
}

/**
 * @brief 所有灯珠熄灭函数
 * @return
 * @Date 2022-10-18 20:51:41
 */
void WS2812B_ClearAll(void)
{
    for (unsigned char i = 0; i < NUM_LEDS; i++)
    {
        WS2812B_SetRGB(0x00, 0x00, 0x00, i);
    }
    WS2812B_Show();
}

/**
 * @brief 多个灯珠按顺序点亮函数(R,G,B单独设置)
 * @param R 设置R颜色参数(格式0x00)
 * @param G 设置G颜色参数(格式0x00)
 * @param B 设置B颜色参数(格式0x00)
 * @param Sequential Sequential设置多少个数就点亮多少个灯珠
 * @return
 * @Date 2022-10-18 20:52:02
 */
void WS2812B_SetSequentialRGB(unsigned char R, unsigned char G, unsigned char B, unsigned short Sequential)
{
    for (unsigned char i = 0; i < Sequential; i++)
    {
        // 设置RGB颜色和所需设置的灯的位置(从0开始)
        WS2812B_SetRGB(R, G, B, i);
    }

    // 2.刷新显示(数据发送)
    WS2812B_Show();
}

/**
 * @brief 多个灯珠按顺序点亮函数(RGB并行设置)
 * @param RGB RGB颜色参数(格式0x000000)
 * @param Sequential
 * @return
 * @Date 2022-10-18 20:53:23
 */
void WS2812B_SetSequentialColour(unsigned int RGB, unsigned short Sequential)
{
    for (unsigned char i = 0; i < Sequential; i++)
    {
        WS2812B_SetColour(RGB, i);
    }

    WS2812B_Show();
}

/**
 * @brief 多个灯珠按顺序点亮函数(H,V,S单独设置)
 * @param H 设置H色调参数(格式0-360)
 * @param S 设置S饱和度参数(格式0-360)
 * @param V 设置V明度参数(格式0-360)
 * @param Sequential
 * @return
 * @Date 2022-10-20 09:29:04
 */
void WS2812B_SetSequentialHSV(unsigned short H, unsigned short S, unsigned short V, unsigned short Sequential)
{
    for (unsigned char i = 0; i < Sequential; i++)
    {
        WS2812B_SetHSV(H, S, V, i);
    }

    WS2812B_Show();
}

/**
 * @brief 8x8像素字符静态显示
 * @param RGB 字符颜色
 * @param *Data 8x8像素字符数据(取模格式:横向8点取模左高位,数据排列:从左到右从上到下)
 * @return
 * @Date 2022-10-19 11:11:29
 */
void WS2812B_Draw8x8RGBChar(unsigned int RGB, unsigned char *Data)
{
    WS2812B_ClearAll(); // 【特别注意】:此处一定要调用一下清屏函数让系统知道你的板子上一共有多少个灯珠,否则会显示失败

    for (unsigned char i = 0; i < 8; i++)
    {
        for (unsigned char j = 0; j < 8; j++) // 可以通过加减j来左右平位所设定的字符
        {
            if ((Data[i] << j) & (0x80))
            {
                WS2812B_SetColour(RGB, i * 8 + j);
            }
        }
    }

    WS2812B_Show();
}

/**
 * @brief (无限X轴)x8像素RGB字符串动态流动显示
 * @param RGB 字符串颜色
 * @param *Data (无限X轴)x8像素字符串数据(取模格式:纵向8点取模下高位,数据排列:从左到右从上到下)
 * @param X_Deviation X方向上的像素点数(建议设置8的倍数,比如128)
 * @param Delay 字符串移动速度(越小越快)
 * @return
 * @Date 2022-10-19 11:33:09
 */
void WS2812B_DrawRGBString_InfiniteX(unsigned int RGB, unsigned short *Data, unsigned short X_Deviation, unsigned short Delay)
{
    unsigned int X_Deviation2 = 0;
    for (unsigned int k = 0; k < X_Deviation - 8; k++)
    {
        WS2812B_ClearAll();                                                // 【特别注意】:此处一定要调用一下清屏函数让系统知道你的板子上一共有多少个灯珠,否则会显示失败
        for (unsigned int i = 0 + X_Deviation2; i < 8 + X_Deviation2; i++) // 从(0+x)---(7+x)数组值流动读取
        {
            for (unsigned char j = 0; j < 8; j++) // 读取单位(0x00)值
            {
                if ((Data[i] << j) & (0x80))
                {
                    WS2812B_SetColour(RGB, ((i - X_Deviation2) * 8) + j); // 单排8个灯数据写入(取模表要和原来的镜像翻转,因为灯的点亮是从左到右一个一个的一共8个)
                }
            }
        }
        WS2812B_Show();
        X_Deviation2++;
        HAL_Delay(Delay);
    }
}

/**
 * @brief (无限X轴)x8像素HSV字符串动态流动显示
 * @param H 设置H色调参数(格式0-360)
 * @param S 设置S饱和度参数(格式0-360)
 * @param V 设置V明度参数(格式0-360)
 * @param *Data (无限X轴)x8像素字符串数据(取模格式:纵向8点取模下高位,数据排列:从左到右从上到下)
 * @param X_Deviation X方向上的像素点数(建议设置8的倍数,比如128)
 * @param Delay 字符串移动速度(越小越快)
 * @return
 * @Date 2022-10-21 10:58:37
 */
void WS2812B_DrawHSVString_InfiniteX(unsigned short H, unsigned short S, unsigned short V, unsigned short *Data, unsigned short X_Deviation, unsigned short Delay)
{
    unsigned int X_Deviation2 = 0;
    for (unsigned int k = 0; k < X_Deviation - 8; k++)
    {
        WS2812B_ClearAll();                                                // 【特别注意】:此处一定要调用一下清屏函数让系统知道你的板子上一共有多少个灯珠,否则会显示失败
        for (unsigned int i = 0 + X_Deviation2; i < 8 + X_Deviation2; i++) // 从(0+x)---(7+x)数组值流动读取
        {
            for (unsigned char j = 0; j < 8; j++) // 读取单位(0x00)值
            {
                if ((Data[i] << j) & (0x80))
                {
                    WS2812B_SetHSV(H, S, V, ((i - X_Deviation2) * 8) + j); // 单排8个灯数据写入(取模表要和原来的镜像翻转,因为灯的点亮是从左到右一个一个的一共8个)
                }
            }
        }
        WS2812B_Show();
        X_Deviation2++;
        HAL_Delay(Delay);
    }
}

/**
 * @brief (无限X轴)x8像素彩色字符串动态流动显示
 * @param V 字符串的显示亮度
 * @param *Data (无限X轴)x8像素字符串数据(取模格式:纵向8点取模下高位,数据排列:从左到右从上到下)
 * @param X_Deviation X方向上的像素点数(建议设置8的倍数,比如128)
 * @param Delay 字符串移动速度(越小越快)
 * @return
 * @Date 2022-10-21 11:02:05
 */
void WS2812B_DrawColourString_InfiniteX(unsigned short V, unsigned short *Data, unsigned short X_Deviation, unsigned short Delay)
{
    unsigned int X_Deviation2 = 0;
    for (unsigned int k = 0; k < X_Deviation - 8; k++)
    {
        WS2812B_ClearAll();                                                // 【特别注意】:此处一定要调用一下清屏函数让系统知道你的板子上一共有多少个灯珠,否则会显示失败
        for (unsigned int i = 0 + X_Deviation2; i < 8 + X_Deviation2; i++) // 从(0+x)---(7+x)数组值流动读取
        {
            for (unsigned char j = 0; j < 8; j++) // 读取单位(0x00)值
            {
                if ((Data[i] << j) & (0x80))
                {
                    WS2812B_SetHSV(i * j / 2, 95, V, ((i - X_Deviation2) * 8) + j); // 单排8个灯数据写入(取模表要和原来的镜像翻转,因为灯的点亮是从左到右一个一个的一共8个)
                }
            }
        }
        WS2812B_Show();
        X_Deviation2++;
        HAL_Delay(Delay);
    }
}

/**
 * @brief 测试1,红绿蓝切换测试
 * @return
 * @Date 2022-10-18 20:55:10
 */
void WS2812B_Test1(void)
{
    WS2812B_SetSequentialColour(Red, NUM_LEDS);
    HAL_Delay(1000);

    WS2812B_SetSequentialColour(Green, NUM_LEDS);
    HAL_Delay(1000);

    WS2812B_SetSequentialColour(Blue, NUM_LEDS);
    HAL_Delay(1000);

    WS2812B_SetSequentialColour(Black, NUM_LEDS);
    HAL_Delay(1000);
}

/**
 * @brief 测试2,红绿蓝4个一排递推流动
 * @param Test2_Colour1 颜色标注1
 * @param Test2_Colour2 颜色标注2
 * @param Test2_Colour3 颜色标注3
 * @param Test2_Delay1 Led切换时间
 * @return
 * @Date 2022-10-19 12:49:30
 */
void WS2812B_Test2(unsigned int Test2_Colour1, unsigned int Test2_Colour2, unsigned int Test2_Colour3, unsigned short Test2_Delay1)
{
    for (unsigned char i = 60; i < 72; i++)
    {
        switch (i / 4)
        {
        case 0:
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 1:
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 2:
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 3:
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 4:
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 5:
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 6:
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 7:
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 8:
            WS2812B_SetColour(Test2_Colour3, i - 32);
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 9:
            WS2812B_SetColour(Test2_Colour1, i - 36);
            WS2812B_SetColour(Test2_Colour3, i - 32);
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 10:
            WS2812B_SetColour(Test2_Colour2, i - 40);
            WS2812B_SetColour(Test2_Colour1, i - 36);
            WS2812B_SetColour(Test2_Colour3, i - 32);
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 11:
            WS2812B_SetColour(Test2_Colour3, i - 44);
            WS2812B_SetColour(Test2_Colour2, i - 40);
            WS2812B_SetColour(Test2_Colour1, i - 36);
            WS2812B_SetColour(Test2_Colour3, i - 32);
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 12:
            WS2812B_SetColour(Test2_Colour1, i - 48);
            WS2812B_SetColour(Test2_Colour3, i - 44);
            WS2812B_SetColour(Test2_Colour2, i - 40);
            WS2812B_SetColour(Test2_Colour1, i - 36);
            WS2812B_SetColour(Test2_Colour3, i - 32);
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;

        case 13:
            WS2812B_SetColour(Test2_Colour2, i - 52);
            WS2812B_SetColour(Test2_Colour1, i - 48);
            WS2812B_SetColour(Test2_Colour3, i - 44);
            WS2812B_SetColour(Test2_Colour2, i - 40);
            WS2812B_SetColour(Test2_Colour1, i - 36);
            WS2812B_SetColour(Test2_Colour3, i - 32);
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 14:
            WS2812B_SetColour(Test2_Colour3, i - 56);
            WS2812B_SetColour(Test2_Colour2, i - 52);
            WS2812B_SetColour(Test2_Colour1, i - 48);
            WS2812B_SetColour(Test2_Colour3, i - 44);
            WS2812B_SetColour(Test2_Colour2, i - 40);
            WS2812B_SetColour(Test2_Colour1, i - 36);
            WS2812B_SetColour(Test2_Colour3, i - 32);
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 15:
            WS2812B_SetColour(Test2_Colour1, i - 60);
            WS2812B_SetColour(Test2_Colour3, i - 56);
            WS2812B_SetColour(Test2_Colour2, i - 52);
            WS2812B_SetColour(Test2_Colour1, i - 48);
            WS2812B_SetColour(Test2_Colour3, i - 44);
            WS2812B_SetColour(Test2_Colour2, i - 40);
            WS2812B_SetColour(Test2_Colour1, i - 36);
            WS2812B_SetColour(Test2_Colour3, i - 32);
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 16:
            WS2812B_SetColour(Test2_Colour2, i - 64);
            WS2812B_SetColour(Test2_Colour1, i - 60);
            WS2812B_SetColour(Test2_Colour3, i - 56);
            WS2812B_SetColour(Test2_Colour2, i - 52);
            WS2812B_SetColour(Test2_Colour1, i - 48);
            WS2812B_SetColour(Test2_Colour3, i - 44);
            WS2812B_SetColour(Test2_Colour2, i - 40);
            WS2812B_SetColour(Test2_Colour1, i - 36);
            WS2812B_SetColour(Test2_Colour3, i - 32);
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        case 17:
            WS2812B_SetColour(Test2_Colour3, i - 68);
            WS2812B_SetColour(Test2_Colour2, i - 64);
            WS2812B_SetColour(Test2_Colour1, i - 60);
            WS2812B_SetColour(Test2_Colour3, i - 56);
            WS2812B_SetColour(Test2_Colour2, i - 52);
            WS2812B_SetColour(Test2_Colour1, i - 48);
            WS2812B_SetColour(Test2_Colour3, i - 44);
            WS2812B_SetColour(Test2_Colour2, i - 40);
            WS2812B_SetColour(Test2_Colour1, i - 36);
            WS2812B_SetColour(Test2_Colour3, i - 32);
            WS2812B_SetColour(Test2_Colour2, i - 28);
            WS2812B_SetColour(Test2_Colour1, i - 24);
            WS2812B_SetColour(Test2_Colour3, i - 20);
            WS2812B_SetColour(Test2_Colour2, i - 16);
            WS2812B_SetColour(Test2_Colour1, i - 12);
            WS2812B_SetColour(Test2_Colour3, i - 8);
            WS2812B_SetColour(Test2_Colour2, i - 4);
            WS2812B_SetColour(Test2_Colour1, i);
            WS2812B_Show();
            HAL_Delay(Test2_Delay1);
            break;
        }
    }
}

/**
 * @brief 测试3,Ciallo字符循环切换显示
 * @return
 * @Date 2022-10-18 23:43:16
 */
void WS2812B_Test3(void)
{
    WS2812B_Draw8x8RGBChar(Red, Data1);
    HAL_Delay(500);
    WS2812B_Draw8x8RGBChar(Brown, Data2);
    HAL_Delay(500);
    WS2812B_Draw8x8RGBChar(DarkGreen, Data3);
    HAL_Delay(500);
    WS2812B_Draw8x8RGBChar(Yellow, Data4);
    HAL_Delay(500);
    WS2812B_Draw8x8RGBChar(Fuchsia, Data4);
    HAL_Delay(500);
    WS2812B_Draw8x8RGBChar(Cyan, Data5);
    HAL_Delay(500);
}

/**
 * @brief 测试4,Ciallo字符可选RGB颜色流动显示
 * @return
 * @Date 2022-10-19 11:49:35
 */
void WS2812B_Test4(void)
{
    WS2812B_DrawRGBString_InfiniteX(Brown, Data6, 128, 80);
}

/**
 * @brief 测试5,所有灯色按HSV色调循环显示
 * @return
 * @Date 2022-10-20 09:45:43
 */
void WS2812B_Test5(void)
{
    for (unsigned short i = 0; i < 361; i++)
    {
        WS2812B_SetSequentialHSV(i, 100, 20, NUM_LEDS);
        HAL_Delay(30);
    }
}

/**
 * @brief 测试6,平移灯珠位码函数测试,彩虹色灯珠流动显示
 * @return
 * @Date 2022-10-20 09:45:43
 */
void WS2812B_Test6(void)
{
    static unsigned char Test6_Count1 = 0;
    for (unsigned char i = Test6_Count1; i < NUM_LEDS; i++)
    {
        WS2812B_SetHSV(i * 5.625f, 100, 20, i);
        WS2812B_Show();
        HAL_Delay(50);
        Test6_Count1++;
    }

    // WS2812B_Shift1(Shift_Left, 1);
    WS2812B_Shift2(Shift_Right, 1);
    WS2812B_Show();
    HAL_Delay(50);
}

/**
 * @brief 测试7,白灯模拟水滴流动
 * @param N 灭灯间隙设定(N越大白灯数量越少)
 * @return
 * @Date 2022-10-21 09:53:01
 */
void WS2812B_Test7(unsigned char N)
{
    static unsigned char Test7_Count1 = 0;
    for (unsigned char i = Test7_Count1; i < NUM_LEDS; i++)
    {
        if (i == 0 || i == 25 || i == 35 || i == 45 || i == 55)
        {
            for (unsigned char j = 0; j < N; j++)
            {
                WS2812B_SetHSV(i * 5.625f, 0, 0, i++);
                WS2812B_Show();
                HAL_Delay(50);
                Test7_Count1++;
            }
        }
        WS2812B_SetHSV(i * 5.625f, 0, 20, i);
        WS2812B_Show();
        HAL_Delay(50);
        Test7_Count1++;
    }

    // WS2812B_Shift1(Shift_Left, 1);
    WS2812B_Shift2(Shift_Right, 1);
    WS2812B_Show();
    HAL_Delay(100);
}

/**
 * @brief 测试8,Ciallo字符可选HSV颜色流动显示
 * @return
 * @Date 2022-10-21 11:04:09
 */
void WS2812B_Test8(void)
{
    WS2812B_DrawHSVString_InfiniteX(30, 90, 20, Data6, 128, 80);
}

/**
 * @brief 测试9,Ciallo字符彩色颜色流动显示,可选亮度
 * @return
 * @Date 2022-10-21 11:05:28
 */
void WS2812B_Test9(void)
{
    WS2812B_DrawColourString_InfiniteX(20, Data6, 128, 80);
}
