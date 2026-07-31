#ifndef APP_CAR_H
#define APP_CAR_H

#include <stdint.h>

typedef enum {
    CAR_STATE_IDLE = 0,
    CAR_STATE_CALIBRATING,
    CAR_STATE_RUNNING,
    CAR_STATE_LINE_LOST,
    CAR_STATE_STOPPED,
    CAR_STATE_FAULT
} Car_State;

typedef enum {
    TASK_NONE = 0,
    TASK_2_LAP_FAST,
    TASK_4_AB_BALL,
    TASK_5_LAP_BALL,
    TASK_6_LAP_BALL_POS,
    TASK_COUNT
} Task_Mode;

void App_Car_Init(void);
void App_Car_NextTask(void);
Task_Mode App_Car_GetTask(void);
void App_Car_SetTask(Task_Mode t);
void App_Car_SensorTask1ms(void);
void App_Car_ControlTask5ms(void);
void App_Car_StateTask10ms(void);
void App_Car_Start(void);
void App_Car_Stop(void);
void App_Car_EmergencyStop(void);
void App_Car_StartCalibration(void);
void App_Car_FinishCalibration(void);
Car_State App_Car_GetState(void);
uint32_t App_Car_GetElapsedMs(void);
float    App_Car_GetTraveledMm(void);
uint32_t App_Car_GetTargetMm(void);
void App_Car_SetSpeed(float mmps);
float App_Car_GetSpeed(void);
void App_Car_SetStraightAccel(float a);
float App_Car_GetStraightAccel(void);
void App_Car_SetMaxSpeed(float s);
float App_Car_GetMaxSpeed(void);

#endif
