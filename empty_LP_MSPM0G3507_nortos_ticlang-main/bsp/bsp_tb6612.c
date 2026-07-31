/*
 * TB6612 的 MSPM0 GPIO/PWM 适配层。
 * PWMA = TIMG12 CCP0 (PB13), PWMB = TIMG12 CCP1 (PA31)
 * AIN1/AIN2/BIN1/BIN2/STBY: 由 SysConfig 生成符号
 * PWM: 80 MHz / 4000 = 20 kHz, 0-1000 permille
 */
#include "bsp/bsp_tb6612.h"
#include "ti_msp_dl_config.h"

#define PWM_PERIOD  4000U

static uint16_t g_dutyA;
static uint16_t g_dutyB;

void BSP_TB6612_Init(void)
{
    g_dutyA = 0U;
    g_dutyB = 0U;
    BSP_TB6612_SetStandby(false);
    BSP_TB6612_SetInputs(BSP_TB6612_CHANNEL_A, false, false);
    BSP_TB6612_SetInputs(BSP_TB6612_CHANNEL_B, false, false);
}

void BSP_TB6612_SetStandby(bool standbyReleased)
{
    if (standbyReleased)
        DL_GPIO_setPins(STBY_PORT, STBY_PIN_4_PIN);
    else
        DL_GPIO_clearPins(STBY_PORT, STBY_PIN_4_PIN);
}

void BSP_TB6612_SetInputs(BSP_TB6612_Channel channel, bool in1, bool in2)
{
    GPIO_Regs *p1, *p2;
    uint32_t m1, m2;

    if (channel == BSP_TB6612_CHANNEL_A) {
        /* AIN1=PB0, AIN2=PB1 (硬编码) */
        p1 = GPIOB; m1 = DL_GPIO_PIN_0;
        p2 = GPIOB; m2 = DL_GPIO_PIN_1;
    } else {
        p1 = BIN1_PORT; m1 = BIN1_PIN_2_PIN;   /* PB12 */
        p2 = BIN2_PORT; m2 = BIN2_PIN_3_PIN;   /* PB20 */
    }
    if (in1) DL_GPIO_setPins(p1, m1); else DL_GPIO_clearPins(p1, m1);
    if (in2) DL_GPIO_setPins(p2, m2); else DL_GPIO_clearPins(p2, m2);
}

void BSP_TB6612_SetPwmPermille(BSP_TB6612_Channel channel, uint16_t duty)
{
    DL_TIMER_CC_INDEX idx;
    uint32_t cmp;

    if (duty > 1000U) duty = 1000U;

    if (channel == BSP_TB6612_CHANNEL_A) {
        g_dutyA = duty;
        idx = GPIO_MOTOR_PWM_C0_IDX;
    } else {
        g_dutyB = duty;
        idx = GPIO_MOTOR_PWM_C1_IDX;
    }

    cmp = PWM_PERIOD * (1000U - duty) / 1000U;
    DL_TimerG_setCaptureCompareValue(MOTOR_PWM_INST, cmp, idx);

    if ((g_dutyA == 0U) && (g_dutyB == 0U)) {
        DL_TimerG_stopCounter(MOTOR_PWM_INST);
    } else {
        DL_TimerG_startCounter(MOTOR_PWM_INST);
    }
}
