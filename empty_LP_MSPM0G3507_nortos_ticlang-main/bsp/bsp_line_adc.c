/*
 * 八路真实模拟循迹阵列的 ADC BSP。
 *
 * 设计目标：
 * 1. ADC0、ADC1 各运行一个软件触发的非重复序列；
 * 2. 两个序列的末尾 MEM 中断只置完成标志，不在 ISR 中做算法；
 * 3. 1 ms 主任务只有在两组都完成后才读取并提交完整八路帧；
 * 4. 提交后立即重新使能并触发下一帧，全程没有轮询等待或延时；
 * 5. 任一 ADC 长时间未完成时自动重启两个序列，避免半帧永久卡死。
 *
 * 默认映射（可以在 SysConfig 中改变“MEM 对应输入通道”，但数组顺序必须保持）：
 *   ADC0 MEM0..2 -> values[0..2]
 *   ADC1 MEM0..4 -> values[3..7]
 * values[0] 必须是车辆最左侧探头，values[7] 必须是最右侧探头。
 */
#include "bsp/bsp_line_adc.h"

#include <stddef.h>

#include "config/board_config.h"

#if CAR_LINE_ADC_READY
#include "ti_msp_dl_config.h"
#endif

_Static_assert(CAR_LINE_SENSOR_COUNT == 8U,
               "The line sensor driver requires exactly eight channels");
_Static_assert(CAR_LINE_ADC0_CHANNEL_COUNT > 0U,
               "ADC0 sequence must contain at least one channel");
_Static_assert(CAR_LINE_ADC1_CHANNEL_COUNT > 0U,
               "ADC1 sequence must contain at least one channel");
_Static_assert((CAR_LINE_ADC0_CHANNEL_COUNT + CAR_LINE_ADC1_CHANNEL_COUNT) ==
                   CAR_LINE_SENSOR_COUNT,
               "ADC0 and ADC1 channel counts must add up to eight");
_Static_assert(CAR_LINE_ADC0_CHANNEL_COUNT <= 12U,
               "ADC0 sequence exceeds the twelve ADC memory slots");
_Static_assert(CAR_LINE_ADC1_CHANNEL_COUNT <= 12U,
               "ADC1 sequence exceeds the twelve ADC memory slots");
_Static_assert(CAR_LINE_ADC_REVERSE_ORDER <= 1U,
               "CAR_LINE_ADC_REVERSE_ORDER must be 0 or 1");
_Static_assert(CAR_LINE_ADC_TIMEOUT_POLLS > 0U,
               "ADC timeout must be at least one poll");

static volatile bool g_adc0Done;
static volatile bool g_adc1Done;
static bool g_conversionActive;
static uint32_t g_waitPolls;
static uint32_t g_frameCount;
static uint32_t g_restartCount;
static volatile uint32_t g_unexpectedIrqCount;

#if CAR_LINE_ADC_READY
/** @brief 清除旧状态并尽可能同时启动两个 ADC 序列。 */
static void start_conversion_pair(void)
{
    g_adc0Done = false;
    g_adc1Done = false;
    g_waitPolls = 0U;

    DL_ADC12_clearInterruptStatus(CAR_LINE_ADC0_INST,
                                  CAR_LINE_ADC0_DONE_INTERRUPT);
    DL_ADC12_clearInterruptStatus(CAR_LINE_ADC1_INST,
                                  CAR_LINE_ADC1_DONE_INTERRUPT);

    /*
     * 两条语句之间只有很少的 CPU 周期差；相比 1 ms 控制周期，可以视为同一帧。
     * ADC0 和 ADC1 独立工作，因此不会发生八路逐个软件切换造成的长时间偏差。
     */
    DL_ADC12_startConversion(CAR_LINE_ADC0_INST);
    DL_ADC12_startConversion(CAR_LINE_ADC1_INST);
    g_conversionActive = true;
}

/** @brief 超时后重新武装两个非重复序列；不等待硬件完成。 */
static void restart_conversion_pair(void)
{
    DL_ADC12_disableConversions(CAR_LINE_ADC0_INST);
    DL_ADC12_disableConversions(CAR_LINE_ADC1_INST);
    DL_ADC12_enableConversions(CAR_LINE_ADC0_INST);
    DL_ADC12_enableConversions(CAR_LINE_ADC1_INST);
    g_restartCount++;
    start_conversion_pair();
}

/** @brief 把两个 ADC 的连续 MEM 结果映射到车辆左到右数组。 */
static void copy_completed_frame(uint16_t values[CAR_LINE_SENSOR_COUNT])
{
    uint32_t adcIndex;
    uint32_t outputIndex;
    uint16_t sample;

    for (adcIndex = 0U; adcIndex < CAR_LINE_ADC0_CHANNEL_COUNT; ++adcIndex) {
        outputIndex = adcIndex;
        if (CAR_LINE_ADC_REVERSE_ORDER) {
            outputIndex = (CAR_LINE_SENSOR_COUNT - 1U) - outputIndex;
        }
        sample = DL_ADC12_getMemResult(CAR_LINE_ADC0_INST,
                                      (DL_ADC12_MEM_IDX) adcIndex);
        values[outputIndex] = sample;
    }

    for (adcIndex = 0U; adcIndex < CAR_LINE_ADC1_CHANNEL_COUNT; ++adcIndex) {
        outputIndex = CAR_LINE_ADC0_CHANNEL_COUNT + adcIndex;
        if (CAR_LINE_ADC_REVERSE_ORDER) {
            outputIndex = (CAR_LINE_SENSOR_COUNT - 1U) - outputIndex;
        }
        sample = DL_ADC12_getMemResult(CAR_LINE_ADC1_INST,
                                      (DL_ADC12_MEM_IDX) adcIndex);
        values[outputIndex] = sample;
    }
}
#endif

void BSP_LineADC_Init(void)
{
    g_adc0Done = false;
    g_adc1Done = false;
    g_conversionActive = false;
    g_waitPolls = 0U;
    g_frameCount = 0U;
    g_restartCount = 0U;
    g_unexpectedIrqCount = 0U;

#if CAR_LINE_ADC_READY
    /* SYSCFG_DL_init() 已完成 ADC 时钟、序列、MEM、模拟引脚和中断源配置。 */
    NVIC_ClearPendingIRQ(CAR_LINE_ADC0_IRQN);
    NVIC_ClearPendingIRQ(CAR_LINE_ADC1_IRQN);
    NVIC_EnableIRQ(CAR_LINE_ADC0_IRQN);
    NVIC_EnableIRQ(CAR_LINE_ADC1_IRQN);
    start_conversion_pair();
#endif
}

bool BSP_LineADC_Read(uint16_t values[CAR_LINE_SENSOR_COUNT])
{
    if (values == NULL) return false;

#if CAR_LINE_ADC_READY
    if (!g_conversionActive) {
        start_conversion_pair();
        return false;
    }

    if (!(g_adc0Done && g_adc1Done)) {
        g_waitPolls++;
        if (g_waitPolls >= CAR_LINE_ADC_TIMEOUT_POLLS) {
            restart_conversion_pair();
        }
        return false;
    }

    /* 两组都完成后才读结果，保证上层收到的是完整八路帧。 */
    copy_completed_frame(values);
    g_frameCount++;
    g_conversionActive = false;

    /* 非重复序列完成后需要重新允许转换，再触发下一帧。 */
    DL_ADC12_enableConversions(CAR_LINE_ADC0_INST);
    DL_ADC12_enableConversions(CAR_LINE_ADC1_INST);
    start_conversion_pair();
    return true;
#else
    uint32_t index;

    for (index = 0U; index < CAR_LINE_SENSOR_COUNT; ++index) {
        values[index] = 0U;
    }
    return false;
#endif
}

bool BSP_LineADC_IsReady(void)
{
#if CAR_LINE_ADC_READY
    return true;
#else
    return false;
#endif
}

uint32_t BSP_LineADC_GetFrameCount(void) { return g_frameCount; }
uint32_t BSP_LineADC_GetRestartCount(void) { return g_restartCount; }
uint32_t BSP_LineADC_GetUnexpectedIrqCount(void)
{
    return g_unexpectedIrqCount;
}

#if CAR_LINE_ADC_READY
void CAR_LINE_ADC0_IRQ_HANDLER(void)
{
    const DL_ADC12_IIDX pending =
        DL_ADC12_getPendingInterrupt(CAR_LINE_ADC0_INST);

    if (pending == CAR_LINE_ADC0_DONE_IIDX) {
        g_adc0Done = true;
    } else if (pending != (DL_ADC12_IIDX) 0) {
        g_unexpectedIrqCount++;
    }
}

void CAR_LINE_ADC1_IRQ_HANDLER(void)
{
    const DL_ADC12_IIDX pending =
        DL_ADC12_getPendingInterrupt(CAR_LINE_ADC1_INST);

    if (pending == CAR_LINE_ADC1_DONE_IIDX) {
        g_adc1Done = true;
    } else if (pending != (DL_ADC12_IIDX) 0) {
        g_unexpectedIrqCount++;
    }
}
#endif
