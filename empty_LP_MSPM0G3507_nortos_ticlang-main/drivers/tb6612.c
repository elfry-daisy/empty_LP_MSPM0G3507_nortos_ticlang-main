/*
 * TB6612 芯片级驱动。
 *
 * 方向真值表：
 *   IN1=1, IN2=0：正转；IN1=0, IN2=1：反转；
 *   IN1=0, IN2=0：滑行；IN1=1, IN2=1 且 PWM 有效：短刹车。
 * PWMA/PWMB 只接收无符号幅值，正负号在本层转换为方向脚。
 */
#include "drivers/tb6612.h"
#include "bsp/bsp_tb6612.h"

static uint16_t magnitude(int16_t value)
{
    /* 先提升到 32 位，避免对 INT16_MIN 直接取负溢出。 */
    int32_t wide = value;
    if (wide < 0) wide = -wide;
    if (wide > 1000) wide = 1000;
    return (uint16_t) wide;
}

static BSP_TB6612_Channel to_channel(TB6612_Motor motor)
{
    return (motor == TB6612_MOTOR_LEFT) ? BSP_TB6612_CHANNEL_A
                                        : BSP_TB6612_CHANNEL_B;
}

void TB6612_Init(void)
{
    /* 初始化顺序确保 STBY 释放前，PWM 和方向脚已经处于安全值。 */
    BSP_TB6612_Init();
    BSP_TB6612_SetPwmPermille(BSP_TB6612_CHANNEL_A, 0U);
    BSP_TB6612_SetPwmPermille(BSP_TB6612_CHANNEL_B, 0U);
    BSP_TB6612_SetInputs(BSP_TB6612_CHANNEL_A, false, false);
    BSP_TB6612_SetInputs(BSP_TB6612_CHANNEL_B, false, false);
    BSP_TB6612_SetStandby(false);
}

void TB6612_Enable(bool enable)
{
    /* 禁用前先请求滑行，随后 STBY 低会让 H 桥进入高阻。 */
    if (!enable) TB6612_StopAll(TB6612_STOP_COAST);
    BSP_TB6612_SetStandby(enable);
}

void TB6612_SetSignedDuty(TB6612_Motor motor, int16_t dutyPermille,
                          TB6612_StopMode zeroMode)
{
    BSP_TB6612_Channel channel = to_channel(motor);

    /* 将有符号命令翻译为 TB6612 的方向真值表。 */
    if (dutyPermille > 0) {
        BSP_TB6612_SetInputs(channel, true, false);
        BSP_TB6612_SetPwmPermille(channel, magnitude(dutyPermille));
    } else if (dutyPermille < 0) {
        BSP_TB6612_SetInputs(channel, false, true);
        BSP_TB6612_SetPwmPermille(channel, magnitude(dutyPermille));
    } else if (zeroMode == TB6612_STOP_BRAKE) {
        /* H/H + 100% PWM 对应短刹车；具体波形极性由 BSP 核对。 */
        BSP_TB6612_SetInputs(channel, true, true);
        BSP_TB6612_SetPwmPermille(channel, 1000U);
    } else {
        BSP_TB6612_SetPwmPermille(channel, 0U);
        BSP_TB6612_SetInputs(channel, false, false);
    }
}

void TB6612_StopAll(TB6612_StopMode mode)
{
    TB6612_SetSignedDuty(TB6612_MOTOR_LEFT, 0, mode);
    TB6612_SetSignedDuty(TB6612_MOTOR_RIGHT, 0, mode);
}
