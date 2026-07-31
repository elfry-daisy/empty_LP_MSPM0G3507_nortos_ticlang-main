#ifndef BSP_IMU_UART_H
#define BSP_IMU_UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file bsp_imu_uart.h
 * @brief 500 Hz 串口 IMU 专用的非阻塞 UART 边界。
 *
 * 调试串口和 IMU 串口用途不同，必须使用两个独立的 UART 实例。IMU 每 2 ms
 * 上报 9 字节数据，接收中断只负责把字节放入环形缓冲；协议解析和 CRC 校验在
 * 1 ms 传感器任务中完成，避免在中断里做浮点运算或长时间处理。
 */

/**
 * @brief 清空软件队列，并在硬件已配置时使能 IMU UART 中断。
 * @return true=CAR_IMU_UART_READY 已启用；false=仍是安全占位实现。
 */
bool BSP_IMU_UART_Init(void);

/**
 * @brief 非阻塞地把完整数据块放入 TX 队列。
 * @return true=已全部入队；false=硬件未配置、参数无效或队列空间不足。
 */
bool BSP_IMU_UART_TryWrite(const uint8_t *data, size_t length);

/** @brief 从 RX 环形缓冲取一个字节；无数据时立即返回 false。 */
bool BSP_IMU_UART_TryReadByte(uint8_t *value);

/**
 * @brief IMU UART 的公共中断服务函数。
 *
 * 当 CAR_IMU_UART_READY=1 时，本 BSP 会用 CAR_IMU_UART_IRQ_HANDLER 生成真正
 * 的中断入口，并在入口中调用本函数。若工程已有同名 IRQHandler，应只保留一个
 * 入口，并从已有入口调用这里。
 */
void BSP_IMU_UART_IRQHandler(void);

/** @return RX 环形缓冲因来不及取数而丢弃的字节总数。 */
uint32_t BSP_IMU_UART_GetRxOverflowCount(void);

#endif /* BSP_IMU_UART_H */
