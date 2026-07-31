#ifndef LINE_CONTROL_H
#define LINE_CONTROL_H

#include "control/pid.h"
#include "drivers/line_sensor.h"

/**
 * @file line_control.h
 * @brief 把循迹位置误差转换为前进速度和转向速度差。
 */

/** 方向环输出，两个字段单位都是 mm/s。 */
typedef struct {
    float forwardMmS;
    float steeringMmS;
} LineControl_Output;

/** @brief 使用 car_config.h 默认参数初始化方向 PID。 */
void LineControl_Init(void);

/** @brief 清除方向 PID 和最后有效循迹方向。 */
void LineControl_Reset(void);

/**
 * @brief 根据最新循迹结果计算本周期速度和转向。
 * @param baseSpeedMmS 直线上的目标速度，通常由任务档位或蓝牙调参决定。
 */
LineControl_Output LineControl_Update(const LineSensor_Data *line,
                                      float dtSeconds, float baseSpeedMmS);

/** @return 方向 PID 指针，供在线调参使用；不要在 ISR 中修改。 */
PID_Controller *LineControl_GetPID(void);

/** @return 滤波后的循迹位置（约 -1.0~+1.0），供 FSM 等上层使用。 */
float LineControl_GetFilteredPosition(void);

#endif
