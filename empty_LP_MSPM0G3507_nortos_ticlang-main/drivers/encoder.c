/*
 * 编码器物理量换算。
 *
 * distancePerCount = π × 轮径 / 每车轮一圈计数
 * speed = 本周期计数增量 × distancePerCount / dt
 *
 * 每圈计数必须是减速箱输出轴/车轮侧最终 QEI 计数，而不是电机裸轴线数。
 */
#include "drivers/encoder.h"

#include <stdint.h>
#include "bsp/bsp_encoder.h"
#include "config/car_config.h"

#define ENCODER_PI                         (3.14159265358979323846f)
#define ENCODER_SPEED_FILTER_ALPHA         (0.35f)

static Encoder_Data g_left;
static Encoder_Data g_right;
static int32_t g_previousLeft;
static int32_t g_previousRight;

static int32_t wrapped_delta(int32_t current, int32_t previous)
{
    /* 先做无符号减法，让 32 位计数自然回绕，再解释成有符号短时增量。 */
    return (int32_t) ((uint32_t) current - (uint32_t) previous);
}

static void update_one(Encoder_Data *data, int32_t rawCount, int32_t *previous,
                       int32_t polarity, float dtSeconds)
{
    const float distancePerCount = (ENCODER_PI * CAR_WHEEL_DIAMETER_MM) /
                                   CAR_ENCODER_COUNTS_PER_WHEEL_REV;
    const int32_t rawDelta = wrapped_delta(rawCount, *previous);
    float instantaneousSpeed;

    /* 极性保证“车辆前进”为正；累计 count 从复位时刻开始。 */
    *previous = rawCount;
    data->deltaCount = rawDelta * polarity;
    data->count += data->deltaCount;
    data->distanceMm = data->count * distancePerCount;
    if (dtSeconds > 0.0f) {
        instantaneousSpeed = data->deltaCount * distancePerCount / dtSeconds;
        /* 一阶低通抑制低速量化噪声；alpha 越大，响应越快、滤波越弱。 */
        data->speedMmS += ENCODER_SPEED_FILTER_ALPHA *
                          (instantaneousSpeed - data->speedMmS);
        data->speedRpm = data->speedMmS * 60.0f /
                         (ENCODER_PI * CAR_WHEEL_DIAMETER_MM);
    }
}

void Encoder_Init(void)
{
    BSP_Encoder_Init();
    Encoder_Reset();
}

void Encoder_Reset(void)
{
    BSP_Encoder_ResetCounts();
    g_left = (Encoder_Data) {0};
    g_right = (Encoder_Data) {0};
    g_previousLeft = BSP_Encoder_GetLeftCount();
    g_previousRight = BSP_Encoder_GetRightCount();
}

void Encoder_Update(float dtSeconds)
{
    update_one(&g_left, BSP_Encoder_GetLeftCount(), &g_previousLeft,
               CAR_ENCODER_LEFT_POLARITY, dtSeconds);
    update_one(&g_right, BSP_Encoder_GetRightCount(), &g_previousRight,
               CAR_ENCODER_RIGHT_POLARITY, dtSeconds);
}

const Encoder_Data *Encoder_GetLeft(void) { return &g_left; }
const Encoder_Data *Encoder_GetRight(void) { return &g_right; }
