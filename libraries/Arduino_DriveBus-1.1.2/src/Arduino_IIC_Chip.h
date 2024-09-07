/*
 * @Description(CN):
 *      属于Arduino_IIC的一个分支 用于存储IIC相关的Power芯片数据
 *
 * @Description(EN):
 *      A sub-branch of Arduino_IIC, used for storing IIC-related Power chip data
 *
 * @version: V1.0.0
 * @Author: Xk_w
 * @Date: 2023-11-16 16:58:05
 * @LastEditors: Xk_w
 * @LastEditTime: 2023-12-08 13:01:25
 * @License: GPL 3.0
 */
#pragma once

#include <Arduino.h>

class Arduino_IIC_Power
{
public:
    enum Device_State
    {
        POWER_DEVICE_ON,  // 设备功能关
        POWER_DEVICE_OFF, // 设备功能开
    };

    enum Device
    {
        POWER_DEVICE_ADC_MEASURE,          // ADC测量功能
        POWER_DEVICE_HIZ_MODE,             // 高阻态模式（内部总线输出切换为只能电池输出）
        POWER_DEVICE_WATCHDOG_TIMER_RESET, // 看门狗定时器重置（喂狗）
        POWER_DEVICE_OTG_MODE,             // OTG模式
        POWER_DEVICE_CHARGING_MODE,        // 充电模式
        POWER_DEVICE_WATCHDOG_MODE,        // 看门狗模式
        POWER_DEVICE_NTC_MODE,             // 电池NTC热检测模式
        POWER_DEVICE_PCB_OTP_MODE,         // PCB过温保护模式
    };

    enum Device_Value
    {
        POWER_DEVICE_MINIMUM_INPUT_VOLTAGE_LIMIT,                  // 设置最小输入电压限制值
        POWER_DEVICE_CHARGING_TARGET_VOLTAGE_LIMIT,                // 设置充电目标电压限制值
        POWER_DEVICE_MINIMUM_SYSTEM_VOLTAGE_LIMIT,                 // 设置最小系统电压限制值
        POWER_DEVICE_SYSTEM_VOLTAGE_LIMIT,                         // 设置系统电压限制值
        POWER_DEVICE_OTG_VOLTAGE_LIMIT,                            // 设置OTG电压输出值
        POWER_DEVICE_INPUT_CURRENT_LIMIT,                          // 设置输入电流限制值
        POWER_DEVICE_FAST_CHARGING_CURRENT_LIMIT,                  // 设置快速充电电流限制值
        POWER_DEVICE_PRECHARGE_CHARGING_CURRENT_LIMIT,             // 设置预充电电流限制值
        POWER_DEVICE_TERMINATION_CHARGING_CURRENT_LIMIT,           // 设置终止充电电流限制值
        POWER_DEVICE_TERMINATION_PRECHARGE_CHARGING_CURRENT_LIMIT, // 设置终止充电和预充电电流限制值
        POWER_DEVICE_OTG_CHARGING_LIMIT,                           // 设置OTG电流输出值
        POWER_DEVICE_BAT_TO_SYS_DISCHARGE_CURRENT_LIMIT,           // BAT到SYS的放电电流限制值

        POWER_DEVICE_WATCHDOG_TIMER,               // 设置看门狗定时器值
        POWER_DEVICE_THERMAL_REGULATION_THRESHOLD, // 设置热调节阈值值
        POWER_DEVICE_BATTERY_UVLO_THRESHOLD        // 设置电池低电压锁定阈值
    };

    enum Status_Information
    {
        POWER_BUS_STATUS,                // 总线状态
        POWER_BUS_CONNECTION_STATUS,     // 总线连接状态
        POWER_CHARGING_STATUS,           // 充电状态
        POWER_INPUT_SOURCE_STATUS,       // 输入源的状态
        POWER_INPUT_USB_STATUS,          // 输入USB状态
        POWER_SYSTEM_VOLTAGE_STATUS,     // 系统电压状态
        POWER_THERMAL_REGULATION_STATUS, // 热调节状态

        POWER_WATCHDOG_FAULT_STATUS,            // 看门狗故障状态（故障后自动重置限制类的功能值）
        POWER_INPUT_FAULT_STATUS,               // 输入故障状态
        POWER_THERMAL_SHUTDOWN_FAULT_STATUS,    // 热关闭故障状态
        POWER_OTG_FAULT_STATUS,                 // 升压OTG模式故障状态
        POWER_CHARGING_FAULT_STATUS,            // 充电故障状态
        POWER_BATTERY_FAULT_STATUS,             // 电池故障状态
        POWER_SAFETY_TIMER_STATUS_FAULT_STATUS, // 安全定时器故障状态
        POWER_NTC_FAULT_STATUS,                 // NTC故障状态
    };

    enum Value_Information
    {
        POWER_INPUT_VOLTAGE,                // 输入电压值
        POWER_BATTERY_VOLTAGE,              // 电池电压值
        POWER_SYSTEM_VOLTAGE,               // 系统电压值
        POWER_NTC_VOLTAGE_PERCENTAGE,       // NTC电压百分比值
        POWER_CHARGING_CURRENT,             // 充电电流值
        POWER_THERMAL_REGULATION_THRESHOLD, // 热调节阈值值

        POWER_MINIMUM_INPUT_VOLTAGE_LIMIT,                  // 最小输入电压限制值
        POWER_CHARGING_TARGET_VOLTAGE_LIMIT,                // 充电目标电压限制值
        POWER_MINIMUM_SYSTEM_VOLTAGE_LIMIT,                 // 最小系统电压限制值
        POWER_SYSTEM_VOLTAGE_LIMIT,                         // 系统电压限制值
        POWER_OTG_VOLTAGE_LIMIT,                            // OTG模式电压限制
        POWER_INPUT_CURRENT_LIMIT,                          // 输入电流限制值
        POWER_FAST_CHARGING_CURRENT_LIMIT,                  // 快速充电电流限制
        POWER_PRECHARGE_CHARGING_CURRENT_LIMIT,             // 预充电电流限制
        POWER_TERMINATION_CHARGING_CURRENT_LIMIT,           // 终止充电电流限制
        POWER_TERMINATION_PRECHARGE_CHARGING_CURRENT_LIMIT, // 终止充电和预充电电流限制
        POWER_OTG_CURRENT_LIMIT,                            // OTG模式电流限制
        POWER_BAT_TO_SYS_DISCHARGE_CURRENT_LIMIT,           // BAT到SYS的放电电流限制
    };

protected:
};

class Arduino_IIC_Touch
{
public:
    enum Device_State
    {
        TOUCH_DEVICE_ON,  // 设备功能关
        TOUCH_DEVICE_OFF, // 设备功能开
    };
    enum Device_Mode
    {
        // 触摸中断模式
        TOUCH_DEVICE_INTERRUPT_TEST,     //  中断引脚测试，使能后自动周期性发出低脉冲
        TOUCH_DEVICE_INTERRUPT_PERIODIC, // 检测到触摸时，发出低脉冲
        TOUCH_DEVICE_INTERRUPT_CHANGE,   // 检测到触摸状态变化时，发出低脉冲
        TOUCH_DEVICE_INTERRUPT_MOTION,   //  检测到手势时，发出低脉冲
        TOUCH_DEVICE_INTERRUPT_ONCEWLP,  //  长按手势只发出一个低脉冲信号
    };
    enum Device
    {
        TOUCH_DEVICE_INTERRUPT_MODE, // 触摸中断模式
        TOUCH_DEVICE_SLEEP_MODE,     // 休眠模式
    };
    enum Device_Value
    {

    };
    enum Status_Information
    {
        TOUCH_GESTURE_ID, // 触摸手势ID
    };
    enum Value_Information
    {
        TOUCH_FINGER_NUMBER, // 触摸手指个数
        TOUCH_COORDINATE_X,  // 触摸坐标X
        TOUCH_COORDINATE_Y,  // 触摸坐标Y
    };
};