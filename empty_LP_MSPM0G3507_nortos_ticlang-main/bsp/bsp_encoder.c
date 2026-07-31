/*
 * 左右 QEI 硬件适配层。
 *
 * 左轮: TIMG8 硬件 QEI (PB6=PHA, PB7=PHB), 16-bit + 32-bit 软件扩展
 * 右轮: GPIOB 双沿中断软件解码 (PB8=PHA, PB9=PHB)
 *
 * IOMUX/QEI/中断配置已由 SYSCFG_DL_init() 完成。
 */
#include "bsp/bsp_encoder.h"
#include "ti_msp_dl_config.h"

/* ---------- Left encoder (TIMG8 QEI) ---------- */
static int32_t  g_leftExt;
static uint16_t g_leftLast;

/* ---------- Right encoder (software 4-state decode) ---------- */
static const int8_t g_qt[4][4] = {
    /* curr: 00  01  10  11  */
    /* 00 */ { 0, +1, -1,  0},
    /* 01 */ {-1,  0,  0, +1},
    /* 10 */ {+1,  0,  0, -1},
    /* 11 */ { 0, -1, +1,  0},
};
static volatile int32_t g_rightCnt;
static volatile uint8_t  g_rightPrev;
static volatile bool     g_rightOk;

void BSP_Encoder_Init(void)
{
    g_leftExt  = 0;
    /* QEI position counts live in the CTR counter; rebase instead of writing HW. */
    DL_TimerG_setLoadValue(LEFT_ENCODER_QEI_INST, 65535U);
    DL_TimerG_startCounter(LEFT_ENCODER_QEI_INST);
    g_leftLast = (uint16_t)DL_TimerG_getTimerCount(LEFT_ENCODER_QEI_INST);

    g_rightCnt  = 0;
    g_rightPrev = 0U;
    g_rightOk   = false;
    NVIC_ClearPendingIRQ(RIGHT_ENCODER_GPIO_INT_IRQN);
    NVIC_EnableIRQ(RIGHT_ENCODER_GPIO_INT_IRQN);
    {
        uint32_t p = DL_GPIO_readPins(RIGHT_ENCODER_GPIO_PORT,
            RIGHT_ENCODER_GPIO_A_PIN | RIGHT_ENCODER_GPIO_B_PIN);
        g_rightPrev = (uint8_t)(
            ((p & RIGHT_ENCODER_GPIO_B_PIN) ? 2U : 0U) |
            ((p & RIGHT_ENCODER_GPIO_A_PIN) ? 1U : 0U));
        g_rightOk = true;
    }
}

int32_t BSP_Encoder_GetLeftCount(void)
{
    /* In QEI mode the position counter is CTR, not the capture/compare reg. */
    uint16_t hw = DL_TimerG_getTimerCount(LEFT_ENCODER_QEI_INST);
    int32_t d = (int32_t)(hw - g_leftLast);
    if (d > 32767) d -= 65536; else if (d < -32767) d += 65536;
    g_leftExt  += d;
    g_leftLast  = hw;
    return g_leftExt;
}

int32_t BSP_Encoder_GetRightCount(void) { return g_rightCnt; }

void BSP_Encoder_GetDiagnostics(BSP_Encoder_Diagnostics *d)
{
    if (d) {
        d->edgeInterrupts      = 0;
        d->invalidTransitions  = 0;
        d->state               = g_rightPrev;
    }
}

void BSP_Encoder_ResetCounts(void)
{
    /* Rebase on the current CTR to avoid writing the counter while running. */
    g_leftExt  = 0;
    g_leftLast = (uint16_t)DL_TimerG_getTimerCount(LEFT_ENCODER_QEI_INST);
    uint32_t p = DL_GPIO_readPins(RIGHT_ENCODER_GPIO_PORT,
        RIGHT_ENCODER_GPIO_A_PIN | RIGHT_ENCODER_GPIO_B_PIN);
    g_rightCnt  = 0;
    g_rightPrev = (uint8_t)(
        ((p & RIGHT_ENCODER_GPIO_B_PIN) ? 2U : 0U) |
        ((p & RIGHT_ENCODER_GPIO_A_PIN) ? 1U : 0U));
    g_rightOk = true;
}

void GROUP1_IRQHandler(void)
{
    if ((CPUSS->INT_GROUP[1].IIDX & CPUSS_INT_GROUP_IIDX_STAT_MASK)
        != RIGHT_ENCODER_GPIO_INT_IIDX) return;

    uint32_t pen = DL_GPIO_getRawInterruptStatus(RIGHT_ENCODER_GPIO_PORT,
        RIGHT_ENCODER_GPIO_A_PIN | RIGHT_ENCODER_GPIO_B_PIN);
    if (!pen) return;

    uint32_t p = DL_GPIO_readPins(RIGHT_ENCODER_GPIO_PORT,
        RIGHT_ENCODER_GPIO_A_PIN | RIGHT_ENCODER_GPIO_B_PIN);
    uint8_t s = (uint8_t)(
        ((p & RIGHT_ENCODER_GPIO_B_PIN) ? 2U : 0U) |
        ((p & RIGHT_ENCODER_GPIO_A_PIN) ? 1U : 0U));

    DL_GPIO_clearInterruptStatus(RIGHT_ENCODER_GPIO_PORT,
        RIGHT_ENCODER_GPIO_A_PIN | RIGHT_ENCODER_GPIO_B_PIN);

    if (g_rightOk) g_rightCnt += g_qt[g_rightPrev][s];
    g_rightPrev = s;
    g_rightOk = true;
}
