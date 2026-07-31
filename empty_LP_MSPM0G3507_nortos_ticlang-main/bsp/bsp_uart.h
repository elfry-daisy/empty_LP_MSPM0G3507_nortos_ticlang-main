#ifndef BSP_UART_H
#define BSP_UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file bsp_uart.h
 * @brief HC-05 蓝牙透传所用的非阻塞 UART 边界接口。
 *
 * 所有发送函数都只把数据复制进软件 TX 队列，然后立即返回；真正的硬件发送由
 * UART TX 中断继续完成。因此这里不会等待 FIFO、不会调用 delay，也不会进入
 * WFI/Sleep。接收方向由 UART ISR 把字节放进 RX 队列，主循环随时取走。
 */

/**
 * @brief 清空 TX/RX 软件队列，并在硬件已配置时打开 UART 中断。
 *
 * 必须在 SYSCFG_DL_init() 之后调用。CAR_HC05_UART_READY=0 时只清软件状态，
 * 保持安全占位实现。
 */
void BSP_UART_Init(void);

/** @brief 查询 HC-05 UART 的真实硬件分支是否已经启用。 */
bool BSP_UART_IsReady(void);

/**
 * @brief 尝试把一段数据放入发送队列，不等待物理发送完成。
 * @return true=全部接收；false=队列未配置或空间不足。
 */
bool BSP_UART_TryWrite(const uint8_t *data, size_t length);

/** @brief 非阻塞发送一个字节；队列已满或硬件未启用时返回 false。 */
bool BSP_UART_TryWriteByte(uint8_t value);

/**
 * @brief 非阻塞发送一个以 '\0' 结尾的字符串，结尾的 '\0' 本身不会发送。
 * @note 字符串必须短于 TX 缓冲区；需要发送二进制或带零字节的数据请用
 *       BSP_UART_TryWrite()。
 */
bool BSP_UART_TryWriteString(const char *text);

/** @brief 尝试从 RX 队列取一个字节；无数据时立即返回 false。 */
bool BSP_UART_TryReadByte(uint8_t *value);

/**
 * @brief 从 RX 队列尽可能多地读取数据，不等待新数据到达。
 * @return 本次实际写入 data 的字节数。
 */
size_t BSP_UART_Read(uint8_t *data, size_t capacity);

/** @brief 返回当前 RX 软件队列里等待处理的字节数。 */
size_t BSP_UART_GetRxAvailable(void);

/** @brief 返回 TX 软件队列当前还能完整接收的字节数。 */
size_t BSP_UART_GetTxFree(void);

/** @brief RX 软件队列装满后丢弃的新字节数。 */
uint32_t BSP_UART_GetRxOverflowCount(void);

/** @brief 硬件报告的 overrun/framing/parity/noise 错误事件总数。 */
uint32_t BSP_UART_GetHardwareErrorCount(void);

/** @brief 因参数非法、硬件未启用或 TX 空间不足而被拒绝的发送次数。 */
uint32_t BSP_UART_GetTxRejectedCount(void);

/**
 * @brief HC-05 UART 公共中断处理函数。
 * @note CAR_HC05_UART_READY=1 时，本 BSP 会生成 SysConfig 对应的真实 IRQHandler，
 *       应用代码不应再定义第二个同名中断入口。
 */
void BSP_UART_IRQHandler(void);

#endif /* BSP_UART_H */
