/*
 * HC-05 蓝牙透传 UART 非阻塞适配层。
 *
 * HC-05 数据模式只是透明串口，所以这里同时承担“蓝牙发送驱动”和“蓝牙接收驱动”：
 * - TX：主循环把数据一次性复制到环形队列，ISR 持续填充硬件 FIFO；
 * - RX：ISR 排空硬件 FIFO 并写入环形队列，主循环随时读取；
 * - 队列空间不足时立即返回失败，绝不忙等 UART，也绝不拖慢循迹控制环。
 *
 * 缓冲长度必须是 2 的整数次幂，环形索引才能用掩码高效回绕。TX 设为 512 字节，
 * 可以完整容纳 app_debug.c 的一行 CSV，并吸收手机蓝牙链路的短时抖动。
 */
#include "bsp/bsp_uart.h"

#include <string.h>

#include "config/board_config.h"

#if CAR_HC05_UART_READY
#include "ti_msp_dl_config.h"
#endif

#define UART_RX_BUFFER_SIZE (256U)
#define UART_TX_BUFFER_SIZE (512U)
#define UART_RX_MASK        (UART_RX_BUFFER_SIZE - 1U)
#define UART_TX_MASK        (UART_TX_BUFFER_SIZE - 1U)

static uint8_t g_rxBuffer[UART_RX_BUFFER_SIZE];
#if CAR_HC05_UART_READY
static uint8_t g_txBuffer[UART_TX_BUFFER_SIZE];
#endif

/*
 * RX 是 ISR 单生产者、主循环单消费者；TX 是主循环单生产者、ISR 单消费者。
 * Cortex-M0+ 对自然对齐的 16/32 位访问是原子的，volatile 防止编译器缓存索引。
 */
static volatile uint16_t g_rxHead;
static volatile uint16_t g_rxTail;
static volatile uint16_t g_txHead;
static volatile uint16_t g_txTail;
static volatile uint32_t g_rxOverflowCount;
static volatile uint32_t g_hardwareErrorCount;
static volatile uint32_t g_txRejectedCount;

#if CAR_HC05_UART_READY
/**
 * @brief 尽量把 TX 软件队列装入硬件 FIFO。
 *
 * 软件队列排空后必须关闭 TX 中断，否则“FIFO 有空位”会不断触发空中断。
 */
static void service_tx_fifo(void)
{
    while ((g_txTail != g_txHead) &&
           !DL_UART_Main_isTXFIFOFull(CAR_HC05_UART_INST)) {
        DL_UART_Main_transmitData(CAR_HC05_UART_INST,
                                  g_txBuffer[g_txTail]);
        g_txTail = (uint16_t) ((g_txTail + 1U) & UART_TX_MASK);
    }

    if (g_txTail == g_txHead) {
        DL_UART_Main_disableInterrupt(CAR_HC05_UART_INST,
                                      DL_UART_MAIN_INTERRUPT_TX);
    } else {
        DL_UART_Main_enableInterrupt(CAR_HC05_UART_INST,
                                     DL_UART_MAIN_INTERRUPT_TX);
    }
}

/** @brief 一次排空硬件 RX FIFO，尽量减少连续蓝牙数据造成的中断次数。 */
static void service_rx_fifo(void)
{
    while (!DL_UART_Main_isRXFIFOEmpty(CAR_HC05_UART_INST)) {
        uint8_t value = DL_UART_Main_receiveData(CAR_HC05_UART_INST);
        uint16_t next = (uint16_t) ((g_rxHead + 1U) & UART_RX_MASK);

        if (next == g_rxTail) {
            /* 保留旧数据并丢弃新字节，让上层有机会处理已经收到的完整命令。 */
            g_rxOverflowCount++;
        } else {
            g_rxBuffer[g_rxHead] = value;
            g_rxHead = next;
        }
    }
}
#endif

void BSP_UART_Init(void)
{
    g_rxHead = 0U;
    g_rxTail = 0U;
    g_txHead = 0U;
    g_txTail = 0U;
    g_rxOverflowCount = 0U;
    g_hardwareErrorCount = 0U;
    g_txRejectedCount = 0U;

#if CAR_HC05_UART_READY
    /* 时钟、引脚复用、FIFO、波特率和 8N1 必须已经由 SYSCFG_DL_init() 完成。 */
    DL_UART_Main_disableInterrupt(CAR_HC05_UART_INST,
                                  DL_UART_MAIN_INTERRUPT_TX);
    DL_UART_Main_enableInterrupt(CAR_HC05_UART_INST,
                                 DL_UART_MAIN_INTERRUPT_RX |
                                 DL_UART_MAIN_INTERRUPT_OVERRUN_ERROR |
                                 DL_UART_MAIN_INTERRUPT_FRAMING_ERROR |
                                 DL_UART_MAIN_INTERRUPT_PARITY_ERROR |
                                 DL_UART_MAIN_INTERRUPT_NOISE_ERROR);
    NVIC_ClearPendingIRQ(CAR_HC05_UART_IRQN);
    NVIC_EnableIRQ(CAR_HC05_UART_IRQN);
#endif
}

bool BSP_UART_IsReady(void)
{
#if CAR_HC05_UART_READY
    return true;
#else
    return false;
#endif
}

size_t BSP_UART_GetTxFree(void)
{
#if CAR_HC05_UART_READY
    uint16_t head = g_txHead;
    uint16_t tail = g_txTail;
    return (size_t) ((tail - head - 1U) & UART_TX_MASK);
#else
    return 0U;
#endif
}

bool BSP_UART_TryWrite(const uint8_t *data, size_t length)
{
#if CAR_HC05_UART_READY
    uint16_t head;
    size_t i;

    if ((data == NULL) || (length == 0U) ||
        (length >= UART_TX_BUFFER_SIZE) ||
        (length > BSP_UART_GetTxFree())) {
        g_txRejectedCount++;
        return false;
    }

    /*
     * 暂停 TX 空 FIFO 中断后再更新生产者索引，避免 ISR 看到“只入队了一半”的包。
     * 这里仅执行内存复制，不会等待物理串口；RX 中断和其他中断仍保持启用。
     */
    DL_UART_Main_disableInterrupt(CAR_HC05_UART_INST,
                                  DL_UART_MAIN_INTERRUPT_TX);
    head = g_txHead;
    for (i = 0U; i < length; ++i) {
        g_txBuffer[head] = data[i];
        head = (uint16_t) ((head + 1U) & UART_TX_MASK);
    }
    g_txHead = head;

    /* 立即装填硬件 FIFO；剩余数据由后续 TX 中断接力发送。 */
    service_tx_fifo();
    return true;
#else
    (void) data;
    (void) length;
    g_txRejectedCount++;
    return false;
#endif
}

bool BSP_UART_TryWriteByte(uint8_t value)
{
    return BSP_UART_TryWrite(&value, 1U);
}

bool BSP_UART_TryWriteString(const char *text)
{
    size_t length;

    if (text == NULL) {
        g_txRejectedCount++;
        return false;
    }

    length = strlen(text);
    if (length == 0U) return true;
    return BSP_UART_TryWrite((const uint8_t *) text, length);
}

bool BSP_UART_TryReadByte(uint8_t *value)
{
    if ((value == NULL) || (g_rxTail == g_rxHead)) return false;

    *value = g_rxBuffer[g_rxTail];
    g_rxTail = (uint16_t) ((g_rxTail + 1U) & UART_RX_MASK);
    return true;
}

size_t BSP_UART_Read(uint8_t *data, size_t capacity)
{
    size_t count = 0U;

    if (data == NULL) return 0U;
    while ((count < capacity) && BSP_UART_TryReadByte(&data[count])) {
        count++;
    }
    return count;
}

size_t BSP_UART_GetRxAvailable(void)
{
    uint16_t head = g_rxHead;
    uint16_t tail = g_rxTail;
    return (size_t) ((head - tail) & UART_RX_MASK);
}

void BSP_UART_IRQHandler(void)
{
#if CAR_HC05_UART_READY
    switch (DL_UART_Main_getPendingInterrupt(CAR_HC05_UART_INST)) {
    case DL_UART_MAIN_IIDX_RX:
        service_rx_fifo();
        break;
    case DL_UART_MAIN_IIDX_TX:
        service_tx_fifo();
        break;
    case DL_UART_MAIN_IIDX_OVERRUN_ERROR:
    case DL_UART_MAIN_IIDX_FRAMING_ERROR:
    case DL_UART_MAIN_IIDX_PARITY_ERROR:
    case DL_UART_MAIN_IIDX_NOISE_ERROR:
        g_hardwareErrorCount++;
        /* 即使出现错误，也尽量保留同一时刻 FIFO 中仍可读取的有效字节。 */
        service_rx_fifo();
        break;
    default:
        break;
    }
#endif
}

uint32_t BSP_UART_GetRxOverflowCount(void)
{
    return g_rxOverflowCount;
}

uint32_t BSP_UART_GetHardwareErrorCount(void)
{
    return g_hardwareErrorCount;
}

uint32_t BSP_UART_GetTxRejectedCount(void)
{
    return g_txRejectedCount;
}

#if CAR_HC05_UART_READY
/* 用 SysConfig 生成的真实中断名建立入口，不要在其他文件再次定义同名 IRQHandler。 */
void CAR_HC05_UART_IRQ_HANDLER(void)
{
    BSP_UART_IRQHandler();
}
#endif
