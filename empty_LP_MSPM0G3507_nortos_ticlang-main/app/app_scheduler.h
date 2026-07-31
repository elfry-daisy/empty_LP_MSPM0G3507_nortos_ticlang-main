#ifndef APP_SCHEDULER_H
#define APP_SCHEDULER_H

/**
 * @file app_scheduler.h
 * @brief 基于 1 ms 计数器的无休眠、非阻塞协作调度器。
 */

/** @brief 设置所有任务的首次截止时间。 */
void App_Scheduler_Init(void);

/** @brief 主循环持续调用；只执行已到期任务，从不主动休眠。 */
void App_Scheduler_Run(void);

#endif
