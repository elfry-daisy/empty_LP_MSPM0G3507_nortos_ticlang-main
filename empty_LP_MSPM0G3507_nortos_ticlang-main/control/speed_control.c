/*
 * 左右轮速度闭环。
 *
 * 两侧必须使用独立 PID 状态，因为积分、上次误差和机械特性各不相同。PID 输出直接
 * 使用电机层约定的 -1000~+1000 千分比，然后由 Motor 层继续做斜坡和极性处理。
 */
#include "control/speed_control.h"
#include "config/car_config.h"
#include "drivers/motor.h"

static PID_Controller g_leftPid;
static PID_Controller g_rightPid;
static SpeedControl_Status g_status;

void SpeedControl_Init(void)
{
    /* 左右增益分开配置，方便补偿电机、减速箱和负载差异。 */
    PID_Init(&g_leftPid, CAR_SPEED_LEFT_KP, CAR_SPEED_LEFT_KI,
             CAR_SPEED_LEFT_KD, 100,
             CAR_MOTOR_DUTY_LIMIT_PERMILLE);
    PID_Init(&g_rightPid, CAR_SPEED_RIGHT_KP, CAR_SPEED_RIGHT_KI,
             CAR_SPEED_RIGHT_KD, 100,
             CAR_MOTOR_DUTY_LIMIT_PERMILLE);
    SpeedControl_Reset();
}

void SpeedControl_Reset(void)
{
    /* 停车/重新起跑时清积分，防止旧饱和状态造成突然冲击。 */
    PID_Reset(&g_leftPid);
    PID_Reset(&g_rightPid);
    g_status = (SpeedControl_Status) {0};
    Motor_SetTargetPermille(0, 0);
}

void SpeedControl_Update(float leftTargetMmS, float rightTargetMmS,
                         float leftMeasuredMmS, float rightMeasuredMmS,
                         float dtSeconds)
{
    g_status.leftTargetMmS = leftTargetMmS;
    g_status.rightTargetMmS = rightTargetMmS;
    g_status.leftMeasuredMmS = leftMeasuredMmS;
    g_status.rightMeasuredMmS = rightMeasuredMmS;

    /* 分别计算两侧输出；不得复用同一个 PID_Controller。 */
    g_status.leftDutyPermille = PID_Update(&g_leftPid, leftTargetMmS,
                                           leftMeasuredMmS, dtSeconds);
    g_status.rightDutyPermille = PID_Update(&g_rightPid, rightTargetMmS,
                                            rightMeasuredMmS, dtSeconds);
    Motor_SetTargetPermille((int16_t) g_status.leftDutyPermille,
                            (int16_t) g_status.rightDutyPermille);
}

const SpeedControl_Status *SpeedControl_GetStatus(void) { return &g_status; }
PID_Controller *SpeedControl_GetLeftPID(void) { return &g_leftPid; }
PID_Controller *SpeedControl_GetRightPID(void) { return &g_rightPid; }
