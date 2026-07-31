#ifndef APP_DEBUG_H
#define APP_DEBUG_H

/**
 * @file app_debug.h
 * @brief UART 单字符命令、低频 CSV 遥测和 OLED 诊断显示应用层。
 *
 * 新增 IMU 命令：G=当前偏航角清零，I=开始静止角速度零偏标定。
 */

void App_Debug_Init(void);

/** @brief 高频非阻塞轮询 RX，把命令转换为整车状态机请求。 */
void App_Debug_PollCommands(void);

/** @brief 由调度器低频调用，尝试输出一行状态。 */
void App_Debug_Task(void);

/**
 * @brief 刷新 OLED 诊断画面的一行。
 *
 * 软件 I2C 为有界同步传输，因此每次只发送一个 8 像素页；调度器每 100 ms
 * 调用一次，八行约 0.8 秒轮换一遍。未配置或初始化 NACK 时立即返回。
 */
void App_Debug_OLEDTask(void);

#endif
