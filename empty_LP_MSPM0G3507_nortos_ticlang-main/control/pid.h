#ifndef PID_H
#define PID_H

#include <stdbool.h>

/**
 * @file pid.h
 * @brief 可实例化的离散 PID，包含限幅、条件积分和微分低通。
 */

/**
 * PID 运行状态。
 * integral 保存的是误差积分（error*s），不是已经乘 Ki 的输出；
 * derivativeFilterAlpha 越接近 1，微分滤波越强、响应越慢。
 */
typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float previousError;
    float derivative;
    float output;
    float outputMin;
    float outputMax;
    float integralMin;
    float integralMax;
    float derivativeFilterAlpha;
    bool hasPrevious;
} PID_Controller;

/** @brief 设置增益和输出范围，并把运行状态复位。 */
void PID_Init(PID_Controller *pid, float kp, float ki, float kd,
              float outputMin, float outputMax);

/** @brief 在线修改增益；保留已有积分和滤波状态。 */
void PID_SetTunings(PID_Controller *pid, float kp, float ki, float kd);

/** @brief 设置误差积分本身的上下限。 */
void PID_SetIntegralLimits(PID_Controller *pid, float minimum, float maximum);

/** @brief 设置 0~1 的微分一阶滤波系数。 */
void PID_SetDerivativeFilter(PID_Controller *pid, float alpha);

/** @brief 清空积分、上次误差、微分滤波和输出。 */
void PID_Reset(PID_Controller *pid);

/**
 * @brief 执行一次离散 PID。
 * @param dtSeconds 本次实际时间间隔，必须大于 0；否则保持上次输出。
 */
float PID_Update(PID_Controller *pid, float setpoint, float measurement,
                 float dtSeconds);

#endif
