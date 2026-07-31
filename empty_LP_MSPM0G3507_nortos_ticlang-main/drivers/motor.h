#ifndef MOTOR_H
#define MOTOR_H

#include <stdbool.h>
#include <stdint.h>
#include "drivers/tb6612.h"

/**
 * @file motor.h
 * @brief 面向整车的左右电机管理，统一处理极性、限幅、斜坡和安全停机。
 */

/** 电机调试快照；所有占空比字段单位都是千分比。 */
typedef struct {
    int16_t leftTargetPermille;
    int16_t rightTargetPermille;
    int16_t leftAppliedPermille;
    int16_t rightAppliedPermille;
    bool enabled;
    bool emergencyStopped;
} Motor_Status;

/** @brief 初始化软件状态和 TB6612，默认电机禁止。 */
void Motor_Init(void);

/** @brief 允许或禁止电机；紧急停止锁存期间不能重新允许。 */
void Motor_Enable(bool enable);

/** @brief 设置逻辑左右轮目标，占空比范围会自动限制并应用极性宏。 */
void Motor_SetTargetPermille(int16_t left, int16_t right);

/** @brief 选择零指令时滑行或短刹车。 */
void Motor_SetStopMode(TB6612_StopMode mode);

/** @brief 每个 5 ms 控制周期调用一次，推进斜坡并写 TB6612。 */
void Motor_Update(void);

/** @brief 把目标设为零，实际输出按斜坡逐步归零。 */
void Motor_Stop(void);

/** @brief 立即清零、锁存故障并拉低 TB6612 STBY。 */
void Motor_EmergencyStop(void);

/** @brief 清除软件紧急停止锁存；不会自动启动车辆。 */
void Motor_ClearEmergencyStop(void);

/** @return 只读状态指针，调用者不能修改其内容。 */
const Motor_Status *Motor_GetStatus(void);

#endif
