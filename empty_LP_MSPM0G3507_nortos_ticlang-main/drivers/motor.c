/*
 * 整车电机管理层。
 *
 * 上层使用“逻辑前进为正”的左右命令，本层通过 CAR_MOTOR_*_POLARITY 适配实际
 * 电机接线。target 是期望值，applied 是经过斜坡限制后真正发送给 TB6612 的值。
 */
#include "drivers/motor.h"
#include "config/car_config.h"

static Motor_Status g_status;
static TB6612_StopMode g_stopMode = TB6612_STOP_COAST;

static int16_t clamp_duty(int32_t value)
{
    /* 32 位中间量允许先乘极性，再安全限制到配置范围。 */
    if (value > CAR_MOTOR_DUTY_LIMIT_PERMILLE) value = CAR_MOTOR_DUTY_LIMIT_PERMILLE;
    if (value < -CAR_MOTOR_DUTY_LIMIT_PERMILLE) value = -CAR_MOTOR_DUTY_LIMIT_PERMILLE;
    return (int16_t) value;
}

static int16_t slew(int16_t current, int16_t target)
{
    /* 限制单周期变化，减小起步电流冲击和机械打滑。 */
    int32_t difference = (int32_t) target - current;
    if (difference > CAR_MOTOR_SLEW_PER_5MS) difference = CAR_MOTOR_SLEW_PER_5MS;
    if (difference < -CAR_MOTOR_SLEW_PER_5MS) difference = -CAR_MOTOR_SLEW_PER_5MS;
    return (int16_t) ((int32_t) current + difference);
}

static int16_t add_deadband(int16_t duty)
{
    /* 非零小指令提升到克服静摩擦所需的最小占空比。 */
    if ((duty > 0) && (duty < CAR_MOTOR_DEADBAND_PERMILLE)) return CAR_MOTOR_DEADBAND_PERMILLE;
    if ((duty < 0) && (duty > -CAR_MOTOR_DEADBAND_PERMILLE)) return -CAR_MOTOR_DEADBAND_PERMILLE;
    return duty;
}

void Motor_Init(void)
{
    g_status = (Motor_Status) {0};
    g_stopMode = TB6612_STOP_COAST;
    TB6612_Init();
}

void Motor_Enable(bool enable)
{
    /* 紧急停止具有更高优先级，必须先显式清除锁存。 */
    if (g_status.emergencyStopped) enable = false;
    if (!enable) {
        g_status.leftTargetPermille = 0;
        g_status.rightTargetPermille = 0;
        g_status.leftAppliedPermille = 0;
        g_status.rightAppliedPermille = 0;
        TB6612_StopAll(g_stopMode);
    }
    g_status.enabled = enable;
    TB6612_Enable(enable);
}

void Motor_SetTargetPermille(int16_t left, int16_t right)
{
    /* 极性只在一个位置应用，控制层无需散落负号。 */
    g_status.leftTargetPermille = clamp_duty((int32_t) left * CAR_MOTOR_LEFT_POLARITY);
    g_status.rightTargetPermille = clamp_duty((int32_t) right * CAR_MOTOR_RIGHT_POLARITY);
}

void Motor_SetStopMode(TB6612_StopMode mode) { g_stopMode = mode; }

void Motor_Update(void)
{
    if (!g_status.enabled || g_status.emergencyStopped) {
        TB6612_StopAll(g_stopMode);
        return;
    }
    /* 先推进内部斜坡，再添加死区补偿并提交给芯片级驱动。 */
    g_status.leftAppliedPermille = slew(g_status.leftAppliedPermille, g_status.leftTargetPermille);
    g_status.rightAppliedPermille = slew(g_status.rightAppliedPermille, g_status.rightTargetPermille);
    TB6612_SetSignedDuty(TB6612_MOTOR_LEFT, add_deadband(g_status.leftAppliedPermille), g_stopMode);
    TB6612_SetSignedDuty(TB6612_MOTOR_RIGHT, add_deadband(g_status.rightAppliedPermille), g_stopMode);
}

void Motor_Stop(void) { Motor_SetTargetPermille(0, 0); }

void Motor_EmergencyStop(void)
{
    /* 先清状态并请求停止，随后拉低 STBY，保证最终 H 桥处于高阻无输出。 */
    g_status.emergencyStopped = true;
    g_status.enabled = false;
    g_status.leftTargetPermille = 0;
    g_status.rightTargetPermille = 0;
    g_status.leftAppliedPermille = 0;
    g_status.rightAppliedPermille = 0;
    TB6612_StopAll(TB6612_STOP_BRAKE);
    TB6612_Enable(false);
}

void Motor_ClearEmergencyStop(void) { g_status.emergencyStopped = false; }
const Motor_Status *Motor_GetStatus(void) { return &g_status; }
