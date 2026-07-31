#ifndef BSP_TB6612_H
#define BSP_TB6612_H

#include <stdbool.h>
#include <stdint.h>

/** @file bsp_tb6612.h @brief TB6612 所需 GPIO/PWM 的 MSPM0 硬件边界。 */

/** A/B 与 TB6612 芯片丝印通道一致，不代表抽象的左右方向。 */
typedef enum {
    BSP_TB6612_CHANNEL_A = 0,
    BSP_TB6612_CHANNEL_B
} BSP_TB6612_Channel;

/** @brief 初始化安全输出：两个 PWM 为 0、方向脚低、STBY 低。 */
void BSP_TB6612_Init(void);

/**
 * @brief 控制 TB6612 的 STBY 引脚。
 * @param standbyReleased true=芯片工作，false=芯片高阻待机。
 * @note 这里的 standby 是电机芯片使能，不是 MCU 低功耗。
 */
void BSP_TB6612_SetStandby(bool standbyReleased);

/** @brief 写入指定通道的 IN1/IN2 逻辑电平。 */
void BSP_TB6612_SetInputs(BSP_TB6612_Channel channel, bool input1, bool input2);

/** @brief 设置 PWM 有效占空比，输入范围 0~1000 千分比。 */
void BSP_TB6612_SetPwmPermille(BSP_TB6612_Channel channel, uint16_t dutyPermille);

#endif /* BSP_TB6612_H */
