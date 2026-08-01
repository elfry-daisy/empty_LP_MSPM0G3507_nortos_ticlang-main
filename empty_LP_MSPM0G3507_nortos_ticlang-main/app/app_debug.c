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
static uint8_t g_oledPage;

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

static void render_oled(void)
{
    uint32_t elapsed = App_Car_GetElapsedMs();
    float dist       = App_Car_GetTraveledMm();
    Car_State s      = App_Car_GetState();
    float spd        = App_Car_GetSpeed();

    OLED_Clear();
    OLED_Printf(0,  0, OLED_6X8, "%s", task_name(App_Car_GetTask()));
    OLED_Printf(0, 16, OLED_6X8, "T:%2lu.%02lu  %s",
        (unsigned long)(elapsed/1000U), (unsigned int)((elapsed%1000U)/10U),
        (s==CAR_STATE_RUNNING?"RUN":s==CAR_STATE_STOPPED?"DONE":
         s==CAR_STATE_LINE_LOST?"LOST":s==CAR_STATE_CALIBRATING?"CAL":"IDLE"));
    OLED_Printf(0, 32, OLED_6X8, "Acc:%.0f Max:%.0f",
        (double)App_Car_GetStraightAccel(), (double)App_Car_GetMaxSpeed());
    OLED_Printf(0, 40, OLED_6X8, "S:%3.0f D:%4lu",
        (double)spd, (unsigned long)dist);
    OLED_Printf(0, 52, OLED_6X8, "KP:%.0f KI:%.1f KD:%.0f",
        (double)LineControl_GetPID()->kp, (double)LineControl_GetPID()->ki,
        (double)LineControl_GetPID()->kd);
}
#endif

/* ---- 三按键 ---- */
#define B1_PIN  DL_GPIO_PIN_26   /* PA26: 切任务 */
#define B2_PIN  DL_GPIO_PIN_27   /* PA27: 启动 */
#define B3_PIN  DL_GPIO_PIN_14   /* PA14: 标定 */
#define DBNC    3U

typedef struct { bool dn; uint8_t db; } Btn;

static Btn g_b1, g_b2, g_b3;

static bool btn_edge(Btn *bt, bool pressed, uint32_t now)
{
    (void)now;
    if (pressed == bt->dn) {
        bt->db = DBNC;
        return false;
    }
    if (bt->db > 0U) { bt->db--; return false; }

    bool up = !pressed && bt->dn;
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

    /* PA27 短按=启动（无长按停车功能） */
    if (up2 && can_start(s))
        App_Car_Start();

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
    /* 按键输入初始化：内部上拉，避免悬空电平抖动导致按键失灵 */
    DL_GPIO_initDigitalInputFeatures(GPIOA,
        B1_PIN | B2_PIN | B3_PIN,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableInput(GPIOA, B1_PIN | B2_PIN | B3_PIN);
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
        (void)BSP_UART_TryWriteByte(c);   /* echo: verify bluetooth link */
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

    /* OLED 上电时序较慢时，每 1s 重试初始化直到就绪 */
    if (!g_oledOk) {
        if ((uint32_t)(now - retryMs) < 1000U) return;
        retryMs = now;
        OLED_Init();
        g_oledOk = OLED_IsConnected();
        if (!g_oledOk) return;
    }

    /* 每次只上传一行（约 6ms），避免整屏刷新（约 50ms）阻塞 5ms 控制环。
     * 运行中轮换关键四行，实现实时显示；静止时刷满 8 页。 */
    render_oled();
    OLED_UpdateArea(0, (int16_t)(g_oledPage * 8), 128, 8);
    {
        /* 运行中轮换前 6 页（任务/时间/Acc/Max/速度/距离），静止时刷满 8 页 */
        uint8_t pageCount = (App_Car_GetState() == CAR_STATE_RUNNING) ? 6U : 8U;
        g_oledPage = (uint8_t)((g_oledPage + 1U) % pageCount);
    }
    g_oledOk = OLED_IsConnected();
#endif
}
