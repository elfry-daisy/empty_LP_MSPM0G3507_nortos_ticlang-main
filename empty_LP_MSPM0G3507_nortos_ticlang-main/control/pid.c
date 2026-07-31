/*
 * 通用离散 PID。
 *
 * error = setpoint - measurement
 * derivative = LPF((error - previousError) / dt)
 * output = Kp*error + Ki*integral + Kd*derivative
 *
 * 采用条件积分抗饱和：输出已在上限且误差仍推动其增大时，不继续累计积分；当误差
 * 有助于离开饱和区时允许积分更新。
 */
#include "control/pid.h"

static float clamp(float value, float minimum, float maximum)
{
    if (value > maximum) return maximum;
    if (value < minimum) return minimum;
    return value;
}

void PID_Init(PID_Controller *pid, float kp, float ki, float kd,
              float outputMin, float outputMax)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->outputMin = outputMin;
    pid->outputMax = outputMax;
    pid->integralMin = outputMin;
    pid->integralMax = outputMax;
    /* 默认保留 80% 上次微分，抑制传感器量化和噪声。 */
    pid->derivativeFilterAlpha = 0.80f;
    PID_Reset(pid);
}

void PID_SetTunings(PID_Controller *pid, float kp, float ki, float kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

void PID_SetIntegralLimits(PID_Controller *pid, float minimum, float maximum)
{
    pid->integralMin = minimum;
    pid->integralMax = maximum;
    pid->integral = clamp(pid->integral, minimum, maximum);
}

void PID_SetDerivativeFilter(PID_Controller *pid, float alpha)
{
    /* 0 表示不保留历史，1 表示完全保持历史微分。 */
    pid->derivativeFilterAlpha = clamp(alpha, 0.0f, 1.0f);
}

void PID_Reset(PID_Controller *pid)
{
    pid->integral = 0.0f;
    pid->previousError = 0.0f;
    pid->derivative = 0.0f;
    pid->output = 0.0f;
    pid->hasPrevious = false;
}

float PID_Update(PID_Controller *pid, float setpoint, float measurement,
                 float dtSeconds)
{
    float error;
    float rawDerivative = 0.0f;
    float candidateIntegral;
    float candidateOutput;
    bool allowIntegration;

    if (dtSeconds <= 0.0f) return pid->output;
    error = setpoint - measurement;

    /* 第一次更新没有可靠的 previousError，因此强制原始微分为 0。 */
    if (pid->hasPrevious) rawDerivative = (error - pid->previousError) / dtSeconds;
    else pid->hasPrevious = true;

    pid->derivative = pid->derivativeFilterAlpha * pid->derivative +
                      (1.0f - pid->derivativeFilterAlpha) * rawDerivative;

    /* 先计算候选积分和候选输出，再根据饱和方向决定是否接受积分。 */
    candidateIntegral = clamp(pid->integral + error * dtSeconds,
                              pid->integralMin, pid->integralMax);
    candidateOutput = pid->kp * error + pid->ki * candidateIntegral +
                      pid->kd * pid->derivative;

    allowIntegration = ((candidateOutput <= pid->outputMax) &&
                        (candidateOutput >= pid->outputMin)) ||
                       ((candidateOutput > pid->outputMax) && (error < 0.0f)) ||
                       ((candidateOutput < pid->outputMin) && (error > 0.0f));
    if (allowIntegration) pid->integral = candidateIntegral;

    /* 使用最终接受的积分重新计算并限制实际输出。 */
    pid->output = clamp(pid->kp * error + pid->ki * pid->integral +
                        pid->kd * pid->derivative,
                        pid->outputMin, pid->outputMax);
    pid->previousError = error;
    return pid->output;
}
