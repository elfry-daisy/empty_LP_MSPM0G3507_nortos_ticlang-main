#ifndef BSP_LINE_ADC_H
#define BSP_LINE_ADC_H

#include <stdbool.h>
#include <stdint.h>

#include "config/car_config.h"

/**
 * @file bsp_line_adc.h
 * @brief 八路真实模拟循迹阵列的双 ADC 非阻塞采样边界。
 *
 * 默认用 ADC0 的三通道序列和 ADC1 的五通道序列组成一帧。两个末尾 MEM 中断
 * 只置标志，主任务在两组均完成后一次提交八个 12 位结果。
 */

/**
 * @brief 清空诊断状态、开启两个 ADC 中断并触发第一帧。
 * @note 必须在 SYSCFG_DL_init() 之后调用；不包含阻塞等待或延时。
 */
void BSP_LineADC_Init(void);

/**
 * @brief 尝试取得一帧八路模拟量，并立即触发下一帧。
 * @param values 输出数组，索引 0~7 必须对应车辆物理最左到最右。
 * @return true=本次复制了完整新帧；false=转换尚未完成、正在重启或硬件未配置。
 *
 * 应由当前 1 ms 传感器任务周期调用；禁止用 while 紧循环等待 true。
 */
bool BSP_LineADC_Read(uint16_t values[CAR_LINE_SENSOR_COUNT]);

/** @brief 查询 CAR_LINE_ADC_READY 是否已启用真实 ADC 分支。 */
bool BSP_LineADC_IsReady(void);

/** @brief 自初始化以来成功提交的完整八路帧数量。 */
uint32_t BSP_LineADC_GetFrameCount(void);

/** @brief 两个 ADC 未在超时窗口内共同完成而被重启的次数。 */
uint32_t BSP_LineADC_GetRestartCount(void);

/** @brief ADC IRQ 收到非预期 IIDX 的累计次数。 */
uint32_t BSP_LineADC_GetUnexpectedIrqCount(void);

#endif /* BSP_LINE_ADC_H */
