/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "app/app_car.h"
#include "app/app_scheduler.h"

/*
 * ======================== 上电动作选择区 ========================
 *
 * App_Car_Init() 和 App_Scheduler_Init() 已经默认开启下列后台功能：
 *   - 1 ms：500 Hz IMU 接收、八路 ADC 采样和循迹位置计算；
 *   - 5 ms：左右编码器速度/里程更新；
 *   - 10 ms：车辆状态和丢线超时保护；
 *   - 20 ms：HC-05 命令与 CSV 遥测；
 *   - 100 ms：OLED 逐页诊断显示。
 *
 * 所以正常调试不需要在 while(1) 中重复调用这些函数。程序默认进入 IDLE，仅电机
 * 和循迹闭环没有自动起跑；可通过 HC-05 发送 R 启动、S 停止、X 紧急停止。
 *
 * 如果确实需要“上电后自动动作”，只取消下面对应一行的注释。两项只能选一个：
 *
 * 1. 自动进入标定：电机保持禁止，人工让八路探头扫过黑线与白底，完成后发送 E；
 * 2. 自动进入循迹：会释放 TB6612 STBY 并立即进入速度闭环，只能在传感器、
 *    编码器方向和电机方向都验证正确后使用，首次测试必须架空车轮。
 */
/* #define CAR_POWER_ON_START_CALIBRATION */
/* #define CAR_POWER_ON_START_TRACKING    */

#if defined(CAR_POWER_ON_START_CALIBRATION) && \
    defined(CAR_POWER_ON_START_TRACKING)
#error "Only one CAR_POWER_ON_START_* action can be enabled"
#endif

/*
 * 工程主入口。
 *
 * SYSCFG_DL_init() 负责 SysConfig 生成的静态外设初始化；App_Car_Init() 建立整车
 * 安全状态；随后主循环持续轮询协作调度器。这里故意没有 WFI、Sleep、Standby
 * 或任何阻塞延时，以符合电赛实时控制需求。
 */
int main(void)
{
    /* 等待外设上电稳定（JDY-31/OLED 模块比 MCU 慢启动） */
    DL_Common_delayCycles(3200000); /* ~100ms @ 32MHz (SYSOSC), 等外设就绪 */

    SYSCFG_DL_init();

    /* App_Car_Init 默认保持 TB6612 禁止，车辆不会在上电后自动运行。 */
    App_Car_Init();
    App_Scheduler_Init();

    /*
     * 可选上电动作统一放在所有驱动和调度器初始化完成之后。默认两个宏都未定义，
     * 因而这里不会释放 STBY，也不会改变原有的安全上电行为。
     */
#if defined(CAR_POWER_ON_START_CALIBRATION)
    App_Car_StartCalibration();
#elif defined(CAR_POWER_ON_START_TRACKING)
    App_Car_Start();
#endif
    while (1) {
        /* 所有周期任务都由 1 ms 软件时基触发；主循环本身始终运行。 */
        App_Scheduler_Run();
    }
}
