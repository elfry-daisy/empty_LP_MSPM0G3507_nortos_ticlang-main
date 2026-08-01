/*
 * 调试模块：三按键 + 蓝牙命令 + OLED任务显示 + CSV遥测。
 *
 * PA26短按=切换任务  PA27短按=启动  PA27长按=停止  PA14=标定切换
 * OLED显示：任务、计时、速度、距离、PID
 */
#include "app/app_debug.h"

#include <stdio.h>
#include <stdint.h>
#include "ti_msp_dl_config.h"
#include "app/app_car.h"
#include "bsp/bsp_time.h"
#include "bsp/bsp_uart.h"
#include "config/board_config.h"
#include "drivers/encoder.h"
#include "drivers/imu.h"
#include "drivers/line_sensor.h"
#include "drivers/motor.h"
#include "drivers/oled.h"
#include "control/pid.h"
#include "control/line_control.h"

/* ---- OLED ---- */
#if CAR_OLED_SOFT_I2C_READY
static bool g_oledOk;

static const char* task_name(Task_Mode t)
{
    switch (t) {
    case TASK_2_LAP_FAST:   return "T2:1Lap<=20s";
    case TASK_4_AB_BALL:    return "T4:AB <=8s";
    case TASK_5_LAP_BALL:   return "T5:1Lap<=30s";
    case TASK_6_LAP_BALL_POS: return "T6:1LapPos";
    default:                 return "---";
    }
}

static const char* state_name(Car_State s)
{
    switch (s) {
    case CAR_STATE_RUNNING:     return "RUN";
    case CAR_STATE_STOPPED:     return "DONE";
    case CAR_STATE_LINE_LOST:   return "LOST";
    case CAR_STATE_CALIBRATING: return "CAL";
    case CAR_STATE_FAULT:       return "FAULT";
    default:                    return "IDLE";
    }
}

static void render_oled(void)
{
    uint32_t elapsed = App_Car_GetElapsedMs();
    float dist       = App_Car_GetTraveledMm();
    Car_State s      = App_Car_GetState();
    const Encoder_Data *le = Encoder_GetLeft();
    const Encoder_Data *re = Encoder_GetRight();
    float spd        = (le->speedMmS + re->speedMmS) * 0.5f;

    OLED_Clear();

    if (s == CAR_STATE_CALIBRATING) {
        OLED_Printf(0,  0, OLED_6X8, "== CAL SWEEP ==");
        OLED_Printf(0,  8, OLED_6X8, "OK:%lu/%lu  B3=done",
            (unsigned long)LineSensor_GetCalOkCount(),
            (unsigned long)CAR_LINE_SENSOR_COUNT);
        OLED_Printf(0, 16, OLED_6X8, "Sweep B/W lines,");
        OLED_Printf(0, 24, OLED_6X8, "then B3 or BT e");
        return;
    }

    OLED_Printf(0,  0, OLED_6X8, "%s %s", task_name(App_Car_GetTask()),
                state_name(s));
    OLED_Printf(0,  8, OLED_6X8, "T:%2lu.%02lu",
        (unsigned long)(elapsed/1000U), (unsigned int)((elapsed%1000U)/10U));
    OLED_Printf(0, 16, OLED_6X8, "S:%3.0f D:%4lu",
        (double)spd, (unsigned long)dist);
    OLED_Printf(0, 24, OLED_6X8, "Tgt:%3.0f Stop:%4lu",
        (double)App_Car_GetSpeed(), (unsigned long)App_Car_GetTargetMm());
    OLED_Printf(0, 32, OLED_6X8, "Acc:%.0f Max:%.0f",
        (double)App_Car_GetStraightAccel(), (double)App_Car_GetMaxSpeed());
    OLED_Printf(0, 40, OLED_6X8, "KP:%.0f KD:%.0f",
        (double)LineControl_GetPID()->kp, (double)LineControl_GetPID()->kd);
    OLED_Printf(0, 48, OLED_6X8, "CAL:%s",
        LineSensor_IsCalibrated() ? "OK" : "NO");
}
#endif

/* ---- 三按键 ---- */
#define B1_PIN  DL_GPIO_PIN_26   /* PA26: 切任务 */
#define B2_PIN  DL_GPIO_PIN_27   /* PA27: 启动/停车 */
#define B3_PIN  DL_GPIO_PIN_14   /* PA14: 标定 */
#define LONG_MS 2000U
#define DBNC    3U

typedef struct { uint32_t ms; bool dn, ld; uint8_t db; } Btn;

static Btn g_b1, g_b2, g_b3;

static bool btn_edge(Btn *bt, bool pressed, uint32_t now)
{
    if (pressed == bt->dn) { bt->db = DBNC; return false; }
    if (bt->db > 0U)       { bt->db--;       return false; }

    bool up = !pressed && bt->dn;
    if (pressed && !bt->dn) { bt->ms = now; bt->ld = false; }
    if (pressed && !bt->ld && ((now - bt->ms) >= LONG_MS))
        bt->ld = true;
    bt->dn = pressed;
    return up;
}

static bool can_start(Car_State s)
{
    return (s == CAR_STATE_IDLE || s == CAR_STATE_STOPPED
            || s == CAR_STATE_LINE_LOST);
}

static void poll_buttons(void)
{
    uint32_t now = BSP_Time_GetMs();
    Car_State s = App_Car_GetState();

    bool up1 = btn_edge(&g_b1, DL_GPIO_readPins(GPIOA, B1_PIN) == 0U, now);
    bool up2 = btn_edge(&g_b2, DL_GPIO_readPins(GPIOA, B2_PIN) == 0U, now);
    bool up3 = btn_edge(&g_b3, DL_GPIO_readPins(GPIOA, B3_PIN) == 0U, now);

    /* PA26 短按=切任务（静止时才安全） */
    if (up1 && can_start(s))
        App_Car_NextTask();

    /* PA27 短按=启动 / 长按2秒=停车 */
    if (up2 && can_start(s))
        App_Car_Start();
    if (g_b2.ld && g_b2.dn && (s == CAR_STATE_RUNNING))
        App_Car_Stop();

    /* PA14 短按=标定切换 */
    if (up3) {
        if (LineSensor_IsCalibrating()) App_Car_FinishCalibration();
        else if (s == CAR_STATE_IDLE)   App_Car_StartCalibration();
    }
}

/* ---- 公开API ---- */
void App_Debug_Init(void)
{
    BSP_UART_Init();
#if CAR_OLED_SOFT_I2C_READY
    OLED_Init();
    g_oledOk = OLED_IsConnected();
#endif
}

void App_Debug_PollCommands(void)
{
    uint8_t c;
    Car_State s = App_Car_GetState();
    bool isStatic = (s == CAR_STATE_IDLE || s == CAR_STATE_STOPPED);

    poll_buttons();
    while (BSP_UART_TryReadByte(&c)) {
        PID_Controller *pid = LineControl_GetPID();
        switch (c) {
        case 'r': case 'R': App_Car_Start(); break;
        case 's': case 'S': App_Car_Stop(); break;
        case 'x': case 'X': App_Car_EmergencyStop(); break;
        case 'c': case 'C': App_Car_StartCalibration(); break;
        case 'e': case 'E': App_Car_FinishCalibration(); break;
        case 'g': case 'G': IMU_ResetYaw(); break;
        case 'i': case 'I': IMU_StartGyroCalibration(); break;
        case '2': App_Car_SetTask(TASK_2_LAP_FAST); break;
        case '4': App_Car_SetTask(TASK_4_AB_BALL); break;
        case '5': App_Car_SetTask(TASK_5_LAP_BALL); break;
        case '6': App_Car_SetTask(TASK_6_LAP_BALL_POS); break;
        /* 在线调参 —— PID 运行中也可调 */
        case 'p': PID_SetTunings(pid, pid->kp+50.f, pid->ki, pid->kd); break;
        case 'P': PID_SetTunings(pid, pid->kp-50.f, pid->ki, pid->kd); break;
        case 'k': PID_SetTunings(pid, pid->kp, pid->ki+0.1f, pid->kd); break;
        case 'K': PID_SetTunings(pid, pid->kp, pid->ki-0.1f, pid->kd); break;
        case 'd': PID_SetTunings(pid, pid->kp, pid->ki, pid->kd+2.f); break;
        case 'D': PID_SetTunings(pid, pid->kp, pid->ki, pid->kd-2.f); break;
        /* 速度调节 —— 仅静止时生效，作为下次启动的默认速度 */
        case 'a':
            if (isStatic) App_Car_SetStraightAccel(App_Car_GetStraightAccel() + 20.f);
            break;
        case 'A':
            if (isStatic) App_Car_SetStraightAccel(App_Car_GetStraightAccel() - 20.f);
            break;
        case 'm':
            if (isStatic) App_Car_SetMaxSpeed(App_Car_GetMaxSpeed() + 25.f);
            break;
        case 'M':
            if (isStatic) App_Car_SetMaxSpeed(App_Car_GetMaxSpeed() - 25.f);
            break;
        case 'v':
            if (isStatic) App_Car_SetSpeed(App_Car_GetSpeed() + 50.f);
            break;
        case 'V':
            if (isStatic) App_Car_SetSpeed(App_Car_GetSpeed() - 50.f);
            break;
        default: break;
        }
    }
}

void App_Debug_Task(void)
{
    char buf[160];
    const LineSensor_Data *ln = LineSensor_GetData();
    const Encoder_Data *le = Encoder_GetLeft();
    const Encoder_Data *re = Encoder_GetRight();
    const Motor_Status *mt = Motor_GetStatus();
    int len = snprintf(buf, sizeof(buf),
        "%lu,%u,%u,%lu,%lu,%ld,%ld,%ld,%d,%d,%u\n",
        (unsigned long)BSP_Time_GetMs(),
        (unsigned int)App_Car_GetState(),
        (unsigned int)App_Car_GetTask(),
        (unsigned long)App_Car_GetElapsedMs(),
        (unsigned long)App_Car_GetTraveledMm(),
        (long)(ln->position*1000.0f),
        (long)le->speedMmS, (long)re->speedMmS,
        (int)mt->leftAppliedPermille, (int)mt->rightAppliedPermille,
        ln->lineDetected?1U:0U);
    if (len > 0) {
        size_t sl = (size_t)len;
        if (sl >= sizeof(buf)) sl = sizeof(buf)-1U;
        (void)BSP_UART_TryWrite((const uint8_t*)buf, sl);
    }
}

void App_Debug_OLEDTask(void)
{
#if CAR_OLED_SOFT_I2C_READY
    static uint32_t retryMs;
    uint32_t now = BSP_Time_GetMs();
    Car_State s   = App_Car_GetState();

    /* 运行中绝不触碰 OLED：软件 I2C 阻塞传输（一页约 4~8 ms），
     * 会打乱 200 Hz 控制环，造成速度抖动/按键偶发失灵。
     * 停车/待机时才整屏刷新，此时车不动，阻塞无影响。 */
    if (s == CAR_STATE_RUNNING) return;

    /* 未连接时每 1 s 重试初始化，直到 OLED 应答为止。 */
    if (!g_oledOk) {
        if ((uint32_t)(now - retryMs) < 1000U) return;
        retryMs = now;
        OLED_Init();
        g_oledOk = OLED_IsConnected();
        if (!g_oledOk) return;
    }

    render_oled();
    OLED_Update();
    g_oledOk = OLED_IsConnected();
#endif
}
