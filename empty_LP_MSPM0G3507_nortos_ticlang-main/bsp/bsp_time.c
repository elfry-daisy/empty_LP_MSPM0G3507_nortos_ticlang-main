/*
 * 1 ms 软件时基。
 *
 * Cortex-M0+ 对对齐的 32 位读写是原子的，因此主循环读取该计数不需要临界区。
 * uint32_t 回绕由调度器中的有符号差值比较自然处理。
 */
#include "bsp/bsp_time.h"

#include "config/board_config.h"

#if CAR_TIMEBASE_READY
#include "ti_msp_dl_config.h"
#endif

static volatile uint32_t g_timeMs;

void BSP_Time_Init(void)
{
    g_timeMs = 0U;

#if CAR_TIMEBASE_READY
    /*
     * SysConfig 把 TIMG0 配成 1 ms PERIODIC，但故意不自动启动。这里先停止并清除
     * 可能遗留的 ZERO 标志，再打开 NVIC 和 Counter，保证软件时间从确定的 0 开始。
     */
    DL_TimerG_stopCounter(CAR_TIMEBASE_INST);
    DL_TimerG_clearInterruptStatus(CAR_TIMEBASE_INST,
                                   DL_TIMERG_INTERRUPT_ZERO_EVENT);
    NVIC_ClearPendingIRQ(CAR_TIMEBASE_IRQN);
    NVIC_EnableIRQ(CAR_TIMEBASE_IRQN);
    DL_TimerG_startCounter(CAR_TIMEBASE_INST);
#endif
}

uint32_t BSP_Time_GetMs(void)
{
    return g_timeMs;
}

void BSP_Time_Tick1msFromISR(void)
{
    /* ISR 中只做常数时间自增，保持中断延迟可预测。 */
    ++g_timeMs;
}

#if CAR_TIMEBASE_READY
void CAR_TIMEBASE_IRQ_HANDLER(void)
{
    /* 读取 IIDX 会确认最高优先级待处理中断；当前只启用了 ZERO 事件。 */
    switch (DL_TimerG_getPendingInterrupt(CAR_TIMEBASE_INST)) {
        case DL_TIMER_IIDX_ZERO:
            BSP_Time_Tick1msFromISR();
            break;

        default:
            /* 未启用的来源不应推进系统时间，避免一次硬件异常产生额外 tick。 */
            break;
    }
}
#endif
