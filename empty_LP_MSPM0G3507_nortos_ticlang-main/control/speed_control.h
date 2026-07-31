#ifndef SPEED_CONTROL_H
#define SPEED_CONTROL_H

#include "control/pid.h"

/**
 * @file speed_control.h
 * @brief 左右轮独立速度闭环，输出有符号 PWM 千分比。
 */

/** 用于遥测和调参的最近一次速度环状态。 */
typedef struct {
    float leftTargetMmS;
    float rightTargetMmS;
    float leftMeasuredMmS;
    float rightMeasuredMmS;
    float leftDutyPermille;
    float rightDutyPermille;
} SpeedControl_Status;

/** @brief 初始化左右速度 PID，并把电机目标设为零。 */
void SpeedControl_Init(void);

/** @brief 清空两个 PID 和状态，不改变 TB6612 使能锁存。 */
void SpeedControl_Reset(void);

/**
 * @brief 执行左右轮一次闭环更新。
 * @param leftTargetMmS/rightTargetMmS 目标轮速，mm/s。
 * @param leftMeasuredMmS/rightMeasuredMmS 编码器实测轮速，mm/s。
 */
void SpeedControl_Update(float leftTargetMmS, float rightTargetMmS,
                         float leftMeasuredMmS, float rightMeasuredMmS,
                         float dtSeconds);
const SpeedControl_Status *SpeedControl_GetStatus(void);

/** @return 左右 PID 对象，供串口/菜单在线调参。 */
PID_Controller *SpeedControl_GetLeftPID(void);
PID_Controller *SpeedControl_GetRightPID(void);

#endif
