/*
 * NoRTOS 协作调度器。
 *
 * 使用 (int32_t)(now-deadline)>=0 判断到期，对 uint32_t 毫秒计数回绕安全。若主循环
 * 因调试等原因迟到，不连续追赶执行多次，而是从当前 now 重新安排下一周期，避免
 * 瞬间堆积控制任务；因此所有任务本身仍必须足够短且非阻塞。
 */
#include "app/app_scheduler.h"

#include <stdbool.h>
#include <stdint.h>
#include "app/app_car.h"
#include "app/app_debug.h"
#include "bsp/bsp_time.h"
#include "config/car_config.h"

static uint32_t g_nextSensorMs;
static uint32_t g_nextControlMs;
static uint32_t g_nextStateMs;
static uint32_t g_nextDebugMs;
static uint32_t g_nextOledMs;

static bool due(uint32_t now, uint32_t deadline)
{
    /* 只要单次调度跨度小于 2^31 ms，该比较在回绕前后都成立。 */
    return ((int32_t) (now - deadline) >= 0);
}

void App_Scheduler_Init(void)
{
    uint32_t now = BSP_Time_GetMs();
    App_Debug_Init();
    /* 初始化不立即执行任务，让所有驱动先稳定到安全状态。 */
    g_nextSensorMs = now + 1U;
    g_nextControlMs = now + CAR_CONTROL_PERIOD_MS;
    g_nextStateMs = now + 10U;
    g_nextDebugMs = now + CAR_DEBUG_PERIOD_MS;
    g_nextOledMs = now + CAR_OLED_PERIOD_MS;
}

void App_Scheduler_Run(void)
{
    uint32_t now = BSP_Time_GetMs();
    /* RX 轮询必须非阻塞，保证停止/紧急命令尽快被处理。 */
    App_Debug_PollCommands();

    if (due(now, g_nextSensorMs)) {
        /* 用 now+period 跳过错过的周期，避免迟到后突发追赶。 */
        g_nextSensorMs = now + 1U;
        App_Car_SensorTask1ms();
    }
    if (due(now, g_nextControlMs)) {
        g_nextControlMs = now + CAR_CONTROL_PERIOD_MS;
        App_Car_ControlTask5ms();
    }
    if (due(now, g_nextStateMs)) {
        g_nextStateMs = now + 10U;
        App_Car_StateTask10ms();
    }
    if (due(now, g_nextDebugMs)) {
        g_nextDebugMs = now + CAR_DEBUG_PERIOD_MS;
        App_Debug_Task();
    }
    if (due(now, g_nextOledMs)) {
        /* OLED 软件 I2C 放在所有实时性更高的任务之后，并且每次只刷新一行。 */
        g_nextOledMs = now + CAR_OLED_PERIOD_MS;
        App_Debug_OLEDTask();
    }
}
