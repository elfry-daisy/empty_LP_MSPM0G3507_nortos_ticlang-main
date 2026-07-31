#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include <stdint.h>

/** @file bsp_encoder.h @brief 左硬件 QEI、右 GPIO 软件 QEI 的硬件抽象接口。 */

/** 右轮软件 AB 解码诊断；用于 OLED/串口低频观察，不能在 ISR 中格式化输出。 */
typedef struct {
    uint32_t edgeInterrupts;
    uint32_t invalidTransitions;
    uint8_t state;
} BSP_Encoder_Diagnostics;

/** @brief 启动左右 QEI；具体引脚和计数方式由 SysConfig 决定。 */
void BSP_Encoder_Init(void);

/** @return 左编码器当前有符号累计计数。 */
int32_t BSP_Encoder_GetLeftCount(void);

/** @return 右编码器当前有符号累计计数。 */
int32_t BSP_Encoder_GetRightCount(void);

/** @brief 同时清零硬件计数器和可能存在的软件溢出扩展。 */
void BSP_Encoder_ResetCounts(void);

/** @brief 取得右编码器中断数、非法跳变数和当前 AB 状态的一致快照。 */
void BSP_Encoder_GetDiagnostics(BSP_Encoder_Diagnostics *diagnostics);

#endif /* BSP_ENCODER_H */
