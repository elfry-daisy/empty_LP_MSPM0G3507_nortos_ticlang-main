/*
 * 极简版：纯循迹，无停车逻辑，无 junction 检测。
 *
 * 数据通路: LineSensor → LineControl → VehicleMixer → SpeedControl → Motor → TB6612
 */
#include "app/app_car.h"

#include "bsp/bsp_board.h"
#include "bsp/bsp_time.h"
#include "config/car_config.h"
#include "control/line_control.h"
#include "control/speed_control.h"
#include "control/vehicle_mixer.h"
#include "drivers/encoder.h"
#include "drivers/imu.h"
#include "drivers/line_sensor.h"
#include "drivers/motor.h"

static Car_State  g_state;
static uint32_t   g_lastLineSeenMs;
static Task_Mode  g_task;
static float      g_taskSpeed;
static uint32_t   g_startTimeMs;
static uint32_t   g_stopTimeMs;
static float      g_avgDistMm;
static float      g_prevDistMm;

/* FSM */
static float      g_fsmSpeed = 350.0f;
static float      g_fsmDist;
static float      g_straightAccel = FSM_ACC_NORMAL;
static float      g_maxSpeed      = FSM_SPEED_MAX;
static float      g_curveMaxSpeed = FSM_SPEED_CURVE_MAX;
static uint32_t   g_stopDistMm    = CAR_STOP_DISTANCE_MM;
typedef enum { FSM_STRAIGHT, FSM_PRE_BRAKE, FSM_CURVE } FSM_State;
static FSM_State  g_fsmState = FSM_STRAIGHT;
static uint16_t   g_curveHoldCnt;

/* 任务速度剖面：直道巡航 / 弯道上限 / 停车距离(mm)。
 * 时间目标：T2 ≤20s→约17s，T4 A→B ≤8s→约7s，T5/T6 ≤30s→约28s。 */
typedef struct {
    float    straightMmS;
    float    curveMmS;
    uint32_t stopDistMm;
} Task_Profile;

static const Task_Profile g_taskProfiles[TASK_COUNT] = {
    /* TASK_NONE */           { 300.0f, 220.0f, 6140U },
    /* TASK_2_LAP_FAST */     { 500.0f, 300.0f, 6140U },
    /* TASK_4_AB_BALL */      { 350.0f, 250.0f, CAR_AB_DISTANCE_MM },
    /* TASK_5_LAP_BALL */     { 280.0f, 210.0f, 6140U },
    /* TASK_6_LAP_BALL_POS */ { 280.0f, 210.0f, 6140U },
};

static void apply_task_profile(Task_Mode t)
{
    const Task_Profile *p = &g_taskProfiles[t];
    g_maxSpeed      = p->straightMmS;
    g_curveMaxSpeed = p->curveMmS;
    g_stopDistMm    = p->stopDistMm;
}

static float task_default_speed(Task_Mode t)
{
    switch (t) {
    default:
    case TASK_2_LAP_FAST:   return TASK2_BASE_SPEED_MM_S;
    case TASK_4_AB_BALL:    return TASK4_BASE_SPEED_MM_S;
    case TASK_5_LAP_BALL:   return TASK5_BASE_SPEED_MM_S;
    case TASK_6_LAP_BALL_POS: return TASK6_BASE_SPEED_MM_S;
    }
}

void App_Car_NextTask(void)
{
    g_task = (Task_Mode)((uint8_t)g_task + 1U);
    if (g_task >= TASK_COUNT) g_task = (Task_Mode)1U;
    g_taskSpeed = task_default_speed(g_task);
    apply_task_profile(g_task);
}

Task_Mode App_Car_GetTask(void) { return g_task; }

void App_Car_SetTask(Task_Mode t)
{
    g_task = t;
    g_taskSpeed = task_default_speed(t);
    apply_task_profile(t);
}

void App_Car_Init(void)
{
    BSP_Time_Init();
    BSP_Board_Init();
    Motor_Init();
    Encoder_Init();
    LineSensor_Init();
    (void)IMU_Init();
    LineControl_Init();
    SpeedControl_Init();
    g_lastLineSeenMs = BSP_Time_GetMs();
    g_task   = TASK_2_LAP_FAST;
    g_taskSpeed = task_default_speed(g_task);
    apply_task_profile(g_task);
    g_state  = CAR_STATE_IDLE;
    g_avgDistMm = 0.0f;
}

void App_Car_SensorTask1ms(void)
{
    (void)IMU_Update(0.001f);
    if (LineSensor_Sample() && LineSensor_GetData()->lineDetected)
        g_lastLineSeenMs = BSP_Time_GetMs();
}

void App_Car_ControlTask5ms(void)
{
    const Encoder_Data *left;
    const Encoder_Data *right;
    LineControl_Output lineOutput;
    WheelSpeed_Targets targets;

    Encoder_Update(CAR_CONTROL_PERIOD_S);
    left  = Encoder_GetLeft();
    right = Encoder_GetRight();
    g_avgDistMm = (left->distanceMm + right->distanceMm) * 0.5f;

    if (g_state == CAR_STATE_RUNNING) {
        /* Mileage stop: precise stop at the task stop distance */
        const float targetStop = (float)g_stopDistMm;
        if (g_avgDistMm >= targetStop) {
            Motor_SetStopMode(TB6612_STOP_BRAKE);
            Motor_Enable(false);
            SpeedControl_Reset();
            g_stopTimeMs = BSP_Time_GetMs();
            g_state = CAR_STATE_STOPPED;
            return;
        }

        lineOutput = LineControl_Update(LineSensor_GetData(),
                                        CAR_CONTROL_PERIOD_S, g_taskSpeed);

        /* ==== FSM segment accel/decel ==== */
        {
            /* 使用滤波后位置，避免速度/状态随传感器量化噪声抖动 */
            float filtPos = LineControl_GetFilteredPosition();
            float absPos = (filtPos > 0.0f) ? filtPos : -filtPos;
            float distDelta = g_avgDistMm - g_prevDistMm;
            g_prevDistMm = g_avgDistMm;

            if (g_fsmState == FSM_CURVE) {
                /* Hysteresis: only leave the curve after N consecutive low-error
                 * cycles, so mid-curve dips do not re-accelerate the car. */
                if (absPos < FSM_CURVE_THRESHOLD) {
                    g_curveHoldCnt++;
                    if (g_curveHoldCnt >= FSM_CURVE_EXIT_CYCLES) {
                        g_fsmState     = FSM_PRE_BRAKE;
                        g_fsmDist      = 0.0f;
                        g_curveHoldCnt = 0U;
                    }
                } else {
                    g_curveHoldCnt = 0U;
                }
            } else {
                g_curveHoldCnt = 0U;
                if (absPos < FSM_STRAIGHT_THRESHOLD) {
                    if (g_fsmState != FSM_STRAIGHT) {
                        g_fsmState = FSM_STRAIGHT;
                        g_fsmDist  = 0.0f;
                    }
                    /* Accumulate first, then brake before the next curve. */
                    g_fsmDist += distDelta;
                    if (g_fsmDist >= (float)FSM_PRE_BRAKE_DIST_MM)
                        g_fsmState = FSM_PRE_BRAKE;
                } else if (absPos < FSM_CURVE_THRESHOLD) {
                    /* Transition band: final buffer before the curve /
                     * slow recovery after the curve. */
                    if (g_fsmState != FSM_PRE_BRAKE) g_fsmDist = 0.0f;
                    g_fsmState = FSM_PRE_BRAKE;
                } else {
                    g_fsmState = FSM_CURVE;
                    g_fsmDist  = 0.0f;
                }
            }

            /* Speed integrator: straight cruise / pre-brake / curve / stop.
             * Stop zone is adaptive: brake from v^2/(2a)+margin, then crawl
             * at low speed until the mileage stop distance is reached. */
            float target, accel;
            const float crawlMmS   = (float)CAR_STOP_CRAWL_MM_S;
            const float distToStop = targetStop - g_avgDistMm;
            const float brakeDist  = ((g_fsmSpeed * g_fsmSpeed) /
                                      (2.0f * CAR_STOP_DECEL_MM_S2)) +
                                     (float)CAR_STOP_DECEL_MARGIN_MM + 60.0f;
            bool stopping = (distToStop <= brakeDist);

            if (stopping) {
                target = crawlMmS;
                accel  = (g_fsmSpeed > crawlMmS) ?
                         -CAR_STOP_DECEL_MM_S2 : 0.0f;
            } else if (g_fsmState == FSM_CURVE) {
                target = g_curveMaxSpeed - (absPos * FSM_CURVE_SLOWDOWN);
                if (target < FSM_SPEED_MIN) target = FSM_SPEED_MIN;
                if (target > g_curveMaxSpeed) target = g_curveMaxSpeed;
            } else if (g_fsmState == FSM_PRE_BRAKE) {
                target = g_curveMaxSpeed;  /* slow down before the curve */
            } else {
                target = g_maxSpeed;
            }

            if (!stopping && (g_fsmSpeed < target))
                accel = g_straightAccel;
            else if (!stopping && (g_fsmState == FSM_PRE_BRAKE))
                accel = -FSM_DEC_PRE_BRAKE;
            else if (!stopping && (g_fsmState == FSM_CURVE))
                accel = -FSM_DEC_CURVE;
            else if (!stopping)
                accel = -FSM_DEC_SMOOTH;

            g_fsmSpeed += accel * CAR_CONTROL_PERIOD_S;
            if (g_fsmSpeed > g_maxSpeed) g_fsmSpeed = g_maxSpeed;
            if (stopping && (g_fsmSpeed < crawlMmS))
                g_fsmSpeed = crawlMmS;
            else if (!stopping && (g_fsmSpeed < FSM_SPEED_MIN))
                g_fsmSpeed = FSM_SPEED_MIN;
            if (g_fsmSpeed < 0.0f) g_fsmSpeed = 0.0f;

            lineOutput.forwardMmS = g_fsmSpeed;
        }

        /* 保底：steering 不能超过前进速度，防止慢轮反转 */
        float maxSteer = lineOutput.forwardMmS;
        if (lineOutput.steeringMmS > maxSteer)
            lineOutput.steeringMmS = maxSteer;
        else if (lineOutput.steeringMmS < -maxSteer)
            lineOutput.steeringMmS = -maxSteer;

        targets = VehicleMixer_Mix(lineOutput.forwardMmS,
            lineOutput.steeringMmS, CAR_MAX_WHEEL_SPEED_MM_S);
        SpeedControl_Update(targets.leftMmS, targets.rightMmS,
                            left->speedMmS, right->speedMmS,
                            CAR_CONTROL_PERIOD_S);
    } else {
        SpeedControl_Reset();
    }
    Motor_Update();
}

void App_Car_StateTask10ms(void)
{
    if (g_state == CAR_STATE_RUNNING &&
        (BSP_Time_GetMs() - g_lastLineSeenMs) >= CAR_LINE_LOST_STOP_MS) {
        Motor_SetStopMode(TB6612_STOP_BRAKE);
        Motor_Enable(false);
        SpeedControl_Reset();
        g_state = CAR_STATE_LINE_LOST;
    }
}

void App_Car_Start(void)
{
    if ((g_state == CAR_STATE_FAULT) || LineSensor_IsCalibrating()) return;

    Encoder_Reset();
    g_avgDistMm = 0.0f;
    g_prevDistMm = 0.0f;
    g_fsmSpeed = 350.0f;
    g_fsmDist = 0.0f;
    g_fsmState = FSM_STRAIGHT;
    g_curveHoldCnt = 0U;
    LineControl_Reset();
    SpeedControl_Reset();
    Motor_ClearEmergencyStop();
    Motor_SetStopMode(TB6612_STOP_COAST);
    Motor_Enable(true);
    g_startTimeMs  = BSP_Time_GetMs();
    g_lastLineSeenMs = BSP_Time_GetMs();
    g_state = CAR_STATE_RUNNING;
}

void App_Car_Stop(void)
{
    Motor_SetStopMode(TB6612_STOP_BRAKE);
    Motor_Enable(false);
    SpeedControl_Reset();
    g_stopTimeMs = BSP_Time_GetMs();
    g_state = CAR_STATE_STOPPED;
}

void App_Car_EmergencyStop(void)
{
    Motor_EmergencyStop();
    SpeedControl_Reset();
    g_state = CAR_STATE_FAULT;
}

void App_Car_StartCalibration(void)
{
    Motor_Enable(false);
    LineSensor_StartCalibration();
    g_state = CAR_STATE_CALIBRATING;
}

void App_Car_FinishCalibration(void)
{
    LineSensor_FinishCalibration();
    g_state = CAR_STATE_IDLE;
}

Car_State App_Car_GetState(void) { return g_state; }

uint32_t App_Car_GetElapsedMs(void)
{
    if (g_state == CAR_STATE_RUNNING)
        return BSP_Time_GetMs() - g_startTimeMs;
    if (g_state == CAR_STATE_STOPPED || g_state == CAR_STATE_LINE_LOST)
        return g_stopTimeMs - g_startTimeMs;
    return 0U;
}

float App_Car_GetTraveledMm(void) { return g_avgDistMm; }
uint32_t App_Car_GetTargetMm(void) { return CAR_LAP_DISTANCE_MM; }
void App_Car_SetSpeed(float s) { g_taskSpeed = s; }
float App_Car_GetSpeed(void) { return g_taskSpeed; }
void App_Car_SetStraightAccel(float a) { g_straightAccel = a; }
float App_Car_GetStraightAccel(void)   { return g_straightAccel; }
void App_Car_SetMaxSpeed(float s)      { g_maxSpeed = s; }
float App_Car_GetMaxSpeed(void)        { return g_maxSpeed; }
