/*
 * 循迹阵列算法。
 *
 * 处理流程：底层原始值 → 每通道 0~1000 归一化 → 左右位置权重 → 加权质心。
 * 归一化后“目标线”强度大、背景强度小。当前 BSP 提交八路真实 12 位 ADC 结果，
 * 因此每路黑白跨度、噪声和器件差异都由独立标定值消除。
 */
#include "drivers/line_sensor.h"
#include "bsp/bsp_line_adc.h"

static LineSensor_Data g_data;
static uint16_t g_minimum[CAR_LINE_SENSOR_COUNT];
static uint16_t g_maximum[CAR_LINE_SENSOR_COUNT];
static bool g_calibrating;
static bool g_calibrated;
static float g_lastPosition;

static uint16_t normalize(uint16_t raw, uint16_t minimum, uint16_t maximum)
{
    uint32_t scaled;
    /* 没有形成有效标定跨度时返回 0，避免除零和伪造强信号。 */
    if (maximum <= minimum) return 0U;
    if (raw <= minimum) scaled = 0U;
    else if (raw >= maximum) scaled = 1000U;
    else scaled = ((uint32_t) (raw - minimum) * 1000U) /
                  (uint32_t) (maximum - minimum);
#if CAR_LINE_ACTIVE_DARK && CAR_LINE_BLACK_IS_LOW_RAW
    /* 黑线为目标但原始 ADC 黑线更低：反相后让黑线接近 1000。 */
    scaled = 1000U - scaled;
#elif !CAR_LINE_ACTIVE_DARK && !CAR_LINE_BLACK_IS_LOW_RAW
    scaled = 1000U - scaled;
#endif
    return (uint16_t) scaled;
}

void LineSensor_Init(void)
{
    uint32_t index;
    BSP_LineADC_Init();
    g_data = (LineSensor_Data) {0};
    g_calibrating = false;
    g_calibrated = false;
    g_lastPosition = 0.0f;
    for (index = 0U; index < CAR_LINE_SENSOR_COUNT; ++index) {
        /* 未标定前使用 12 位满量程；实际使用前仍建议执行一次动态黑白标定。 */
        g_minimum[index] = 0U;
        g_maximum[index] = 4095U;
    }
}

bool LineSensor_Sample(void)
{
    uint32_t index;
    uint32_t sum = 0U;
    uint32_t activeCount = 0U;
    int64_t weightedSum = 0;

    g_data.fresh = BSP_LineADC_Read(g_data.raw);
    if (!g_data.fresh) return false;
    if (g_calibrating) LineSensor_UpdateCalibration();

    for (index = 0U; index < CAR_LINE_SENSOR_COUNT; ++index) {
        /*
         * 等间距权重覆盖 -1000~+1000，不把传感器数量写死在算法中。
         * 最终再除以 1000，使公开 position 约为 -1.0~+1.0。
         */
        const int32_t weight = ((int32_t) (2U * index) -
                                (int32_t) (CAR_LINE_SENSOR_COUNT - 1U)) * 1000 /
                               (int32_t) (CAR_LINE_SENSOR_COUNT - 1U);
        const uint16_t value = normalize(g_data.raw[index], g_minimum[index],
                                         g_maximum[index]);
        g_data.normalized[index] = value;
        sum += value;
        weightedSum += (int64_t) value * weight;
        if (value >= CAR_LINE_ELEMENT_THRESHOLD) ++activeCount;
    }

    /* activeCount 用于非常初步的全黑/全白/宽线判断，赛题元素仍需状态机确认。 */
    g_data.lineDetected = (sum >= CAR_LINE_DETECT_SUM_MIN);
    g_data.allBlack = (activeCount == CAR_LINE_SENSOR_COUNT);
    g_data.allWhite = (activeCount == 0U);
    g_data.junction = (activeCount >= (CAR_LINE_SENSOR_COUNT - 1U));
    g_data.confidence = (float) sum / (float) (CAR_LINE_SENSOR_COUNT * 1000U);
    if (g_data.lineDetected && (sum != 0U)) {
        /* 标准加权质心；只在线有效时更新最后方向。 */
        g_data.position = (float) weightedSum / ((float) sum * 1000.0f);
        g_lastPosition = g_data.position;
    } else {
        /* 丢线时保留最后位置，供上层决定向哪一侧搜索。 */
        g_data.position = g_lastPosition;
    }
    return true;
}

void LineSensor_StartCalibration(void)
{
    uint32_t index;
    for (index = 0U; index < CAR_LINE_SENSOR_COUNT; ++index) {
        g_minimum[index] = UINT16_MAX;
        g_maximum[index] = 0U;
    }
    g_calibrating = true;
    g_calibrated = false;
}

void LineSensor_UpdateCalibration(void)
{
    uint32_t index;
    if (!g_calibrating) return;
    for (index = 0U; index < CAR_LINE_SENSOR_COUNT; ++index) {
        if (g_data.raw[index] < g_minimum[index]) g_minimum[index] = g_data.raw[index];
        if (g_data.raw[index] > g_maximum[index]) g_maximum[index] = g_data.raw[index];
    }
}

void LineSensor_FinishCalibration(void)
{
    g_calibrating = false;
    g_calibrated = (LineSensor_GetCalOkCount() == CAR_LINE_SENSOR_COUNT);
}
bool LineSensor_IsCalibrating(void) { return g_calibrating; }
bool LineSensor_IsCalibrated(void) { return g_calibrated; }
uint8_t LineSensor_GetCalOkCount(void)
{
    uint8_t ok = 0U;
    uint32_t index;
    for (index = 0U; index < CAR_LINE_SENSOR_COUNT; ++index) {
        if ((uint32_t)(g_maximum[index] - g_minimum[index]) >=
            (uint32_t)CAR_LINE_CAL_MIN_SPAN) {
            ok++;
        }
    }
    return ok;
}
const LineSensor_Data *LineSensor_GetData(void) { return &g_data; }
