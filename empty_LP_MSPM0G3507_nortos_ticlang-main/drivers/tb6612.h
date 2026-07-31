#ifndef TB6612_H
#define TB6612_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file tb6612.h
 * @brief TB6612FNG 方向真值表和双通道抽象。
 *
 * 该层不依赖 MSPM0 寄存器；具体 GPIO/PWM 写入由 bsp_tb6612 完成。
 */

/** 逻辑左右轮到 TB6612 A/B 通道的映射。 */
typedef enum { TB6612_MOTOR_LEFT = 0, TB6612_MOTOR_RIGHT } TB6612_Motor;

/** 零指令时使用输出高阻滑行，或 H/H 短接电机两端制动。 */
typedef enum { TB6612_STOP_COAST = 0, TB6612_STOP_BRAKE } TB6612_StopMode;

/** @brief 初始化为 PWM=0、方向脚低、STBY 低的安全状态。 */
void TB6612_Init(void);

/** @brief 释放或拉低 TB6612 STBY；false 会先请求滑行停止。 */
void TB6612_Enable(bool enable);

/**
 * @brief 设置一个通道的有符号占空比。
 * @param dutyPermille -1000~+1000；正负号决定 IN1/IN2 方向。
 * @param zeroMode duty=0 时的停止方式。
 */
void TB6612_SetSignedDuty(TB6612_Motor motor, int16_t dutyPermille,
                          TB6612_StopMode zeroMode);

/** @brief 用相同停止方式同时停止两个通道。 */
void TB6612_StopAll(TB6612_StopMode mode);

#endif
