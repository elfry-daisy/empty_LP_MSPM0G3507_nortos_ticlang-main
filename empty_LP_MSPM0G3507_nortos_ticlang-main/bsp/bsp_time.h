#ifndef BSP_TIME_H
#define BSP_TIME_H

#include <stdint.h>

/** @file bsp_time.h @brief 无休眠 NoRTOS 调度器使用的 1 ms 单调时基。 */

/**
 * @brief 清零软件毫秒计数，并在 SysConfig 已就绪时启动 1 ms 硬件时基。
 * @note 必须在 SYSCFG_DL_init() 之后调用；不包含延时、忙等或低功耗等待。
 */
void BSP_Time_Init(void);

/** @return 自初始化以来经过的毫秒数；允许 uint32_t 自然回绕。 */
uint32_t BSP_Time_GetMs(void);

/**
 * @brief 把软件单调时钟推进 1 ms。
 * @note 真实 Timer ISR 已在 bsp_time.c 中实现；其他代码不应直接调用。
 */
void BSP_Time_Tick1msFromISR(void);

#endif /* BSP_TIME_H */
