/*
 * 循迹方向环。
 *
 * 方向 PID 的输出单位是 mm/s 速度差，不是 PWM。这样电池电压和电机左右差异由
 * 下面的轮速闭环处理。线偏得越多，基础前进速度越低，给车辆留下更大转弯余量。
 */
#include "control/line_control.h"
#include "config/car_config.h"

static PID_Controller g_linePid;
static float g_lastPosition;
static float g_filtPosition;   /* 低通滤波后的位置 */
static float g_lastSteering;
static float g_steeringApplied; /* 经过速率限制后实际下发的转向 */
static uint32_t g_lostCycles;

static float absolute(float value) { return (value < 0.0f) ? -value : value; }
static float maximum(float left, float right) { return (left > right) ? left : right; }

void LineControl_Init(void)
{
    PID_Init(&g_linePid, CAR_LINE_KP, CAR_LINE_KI, CAR_LINE_KD,
             -CAR_LINE_STEERING_LIMIT_MM_S, CAR_LINE_STEERING_LIMIT_MM_S);
    /* 方向环默认 Ki=0；仍限制积分，避免在线打开 Ki 时瞬间积累过大。 */
    PID_SetIntegralLimits(&g_linePid, -1.0f, 1.0f);
    PID_SetDerivativeFilter(&g_linePid, 0.85f);
    g_lastPosition  = 0.0f;
    g_filtPosition  = 0.0f;
    g_lastSteering  = 0.0f;
    g_steeringApplied = 0.0f;
    g_lostCycles    = 0U;
}

void LineControl_Reset(void)
{
    PID_Reset(&g_linePid);
    g_lastPosition = 0.0f;
    g_filtPosition = 0.0f;
    g_lastSteering = 0.0f;
    g_steeringApplied = 0.0f;
    g_lostCycles   = 0U;
}

LineControl_Output LineControl_Update(const LineSensor_Data *line,
                                      float dtSeconds, float baseSpeedMmS)
{
    LineControl_Output output;
    float slowdown;

    if (line->lineDetected) {
        g_lastPosition = line->position;
        /* 传感器位置低通滤波，抑制噪声和量化台阶 */
        g_filtPosition += CAR_LINE_POSITION_FILTER_ALPHA *
                         (line->position - g_filtPosition);

        output.steeringMmS = CAR_LINE_STEERING_POLARITY *
            PID_Update(&g_linePid, 0.0f, -g_filtPosition, dtSeconds);

        /* 死区：微小转向置零，抑制直线晃动 */
        if (absolute(output.steeringMmS) < CAR_LINE_STEERING_DEADBAND_MM_S)
            output.steeringMmS = 0.0f;

        /* 转向速率限制：小误差时慢修（丝滑），大误差时快跟（弯道力度） */
        {
            float slewLimit = (float) CAR_LINE_STEERING_SLEW_BASE_PER_5MS +
                              CAR_LINE_STEERING_SLEW_GAIN *
                              absolute(g_filtPosition);
            float diff = output.steeringMmS - g_steeringApplied;
            if (diff > slewLimit) diff = slewLimit;
            else if (diff < -slewLimit) diff = -slewLimit;
            output.steeringMmS = g_steeringApplied + diff;
            g_steeringApplied = output.steeringMmS;
        }

        /* 使用传入的可变基准速度代替原来的 CAR_BASE_SPEED_MM_S */
        slowdown = CAR_CURVE_SLOWDOWN_GAIN * absolute(line->position);
        output.forwardMmS = maximum(CAR_MIN_CURVE_SPEED_MM_S,
                                    baseSpeedMmS - slowdown);
        g_lastSteering = output.steeringMmS;
        g_lostCycles   = 0U;
    } else {
        /* 丢线：短暂保持最后转向防止弯道直冲，超时后低速直走找回 */
        output.forwardMmS = CAR_MIN_CURVE_SPEED_MM_S;
        if (g_lostCycles < CAR_LINE_LOST_STEER_HOLD_CYCLES) {
            output.steeringMmS = g_lastSteering;
            g_lostCycles++;
        } else {
            output.steeringMmS = 0.0f;
        }
    }
    return output;
}

PID_Controller *LineControl_GetPID(void) { return &g_linePid; }

/** @return 滤波后的循迹位置，供 FSM 使用，避免速度随原始位置噪声抖动。 */
float LineControl_GetFilteredPosition(void) { return g_filtPosition; }
