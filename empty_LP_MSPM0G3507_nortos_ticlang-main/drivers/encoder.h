#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

/**
 * @file encoder.h
 * @brief 把左右 QEI 累计计数转换为增量、轮速、RPM 和累计距离。
 */

/** 编码器派生数据；速度单位 mm/s，距离 mm。 */
typedef struct {
    int32_t count;
    int32_t deltaCount;
    float speedRpm;
    float speedMmS;
    float distanceMm;
} Encoder_Data;

/** @brief 初始化硬件边界并清零软件里程。 */
void Encoder_Init(void);

/** @brief 清零硬件/软件计数、速度滤波状态和累计距离。 */
void Encoder_Reset(void);

/** @brief 按实际时间间隔更新左右数据；正常由 5 ms 控制任务调用。 */
void Encoder_Update(float dtSeconds);

/** @return 左/右编码器只读快照指针。 */
const Encoder_Data *Encoder_GetLeft(void);
const Encoder_Data *Encoder_GetRight(void);

#endif
