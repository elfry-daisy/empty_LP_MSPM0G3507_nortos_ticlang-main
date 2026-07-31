/*
 * 500 Hz 串口 IMU 的非阻塞 UART 适配层。
 *
 * RX 和 TX 都使用单生产者/单消费者环形缓冲：
 * - RX：UART ISR 写入，1 ms 任务读取；
 * - TX：主循环写入，UART ISR 读取。
 *
 * 所有缓冲长度均为 2 的整数次幂，索引用掩码回绕。代码不调用阻塞式 UART API，
 * 不包含 delay、WFI 或低功耗等待。
 */
#include "bsp/bsp_imu_uart.h"

#include "config/board_config.h"

#if CAR_IMU_UART_READY
#include "ti_msp_dl_config.h"
#endif

#define IMU_UART_RX_BUFFER_SIZE (64U)
#define IMU_UART_TX_BUFFER_SIZE (32U)
#define IMU_UART_RX_MASK        (IMU_UART_RX_BUFFER_SIZE - 1U)
#define IMU_UART_TX_MASK        (IMU_UART_TX_BUFFER_SIZE - 1U)

static uint8_t g_rxBuffer[IMU_UART_RX_BUFFER_SIZE];
#if CAR_IMU_UART_READY
static uint8_t g_txBuffer[IMU_UART_TX_BUFFER_SIZE];
#endif
static volatile uint16_t g_rxHead;
static volatile uint16_t g_rxTail;
static volatile uint16_t g_txHead;
static volatile uint16_t g_txTail;
static volatile uint32_t g_rxOverflowCount;

#if CAR_IMU_UART_READY
/** 尽可能填满硬件 TX FIFO；队列清空后关闭 TX 中断，避免空中断风暴。 */
static void service_tx_fifo(void)
{
    while ((g_txTail != g_txHead) &&
           !DL_UART_Main_isTXFIFOFull(CAR_IMU_UART_INST)) {
        DL_UART_Main_transmitData(CAR_IMU_UART_INST, g_txBuffer[g_txTail]);
        g_txTail = (uint16_t) ((g_txTail + 1U) & IMU_UART_TX_MASK);
    }

    if (g_txTail == g_txHead) {
        DL_UART_Main_disableInterrupt(CAR_IMU_UART_INST,
                                      DL_UART_MAIN_INTERRUPT_TX);
    } else {
        DL_UART_Main_enableInterrupt(CAR_IMU_UART_INST,
                                     DL_UART_MAIN_INTERRUPT_TX);
    }
}

/** 把硬件 RX FIFO 一次排空，减少 500 Hz 连续数据造成的中断次数。 */
static void service_rx_fifo(void)
{
    while (!DL_UART_Main_isRXFIFOEmpty(CAR_IMU_UART_INST)) {
        uint8_t value = DL_UART_Main_receiveData(CAR_IMU_UART_INST);
        uint16_t next = (uint16_t) ((g_rxHead + 1U) & IMU_UART_RX_MASK);

        if (next == g_rxTail) {
            /* 缓冲已满：丢弃新字节并保留旧数据，交给协议层重新寻找帧头。 */
            g_rxOverflowCount++;
        } else {
            g_rxBuffer[g_rxHead] = value;
            g_rxHead = next;
        }
    }
}
#endif

bool BSP_IMU_UART_Init(void)
{
    g_rxHead = 0U;
    g_rxTail = 0U;
    g_txHead = 0U;
    g_txTail = 0U;
    g_rxOverflowCount = 0U;

#if CAR_IMU_UART_READY
    /* UART 时钟、波特率、8N1 和引脚复用必须先由 SYSCFG_DL_init() 完成。 */
    DL_UART_Main_disableInterrupt(CAR_IMU_UART_INST,
                                  DL_UART_MAIN_INTERRUPT_TX);
    DL_UART_Main_enableInterrupt(CAR_IMU_UART_INST,
                                 DL_UART_MAIN_INTERRUPT_RX |
                                 DL_UART_MAIN_INTERRUPT_OVERRUN_ERROR |
                                 DL_UART_MAIN_INTERRUPT_FRAMING_ERROR |
                                 DL_UART_MAIN_INTERRUPT_PARITY_ERROR |
                                 DL_UART_MAIN_INTERRUPT_NOISE_ERROR);
    NVIC_ClearPendingIRQ(CAR_IMU_UART_IRQN);
    NVIC_EnableIRQ(CAR_IMU_UART_IRQN);
    return true;
#else
    return false;
#endif
}

bool BSP_IMU_UART_TryWrite(const uint8_t *data, size_t length)
{
#if CAR_IMU_UART_READY
    uint16_t head;
    uint16_t tail;
    size_t freeBytes;
    size_t i;

    if ((data == NULL) || (length == 0U) ||
        (length >= IMU_UART_TX_BUFFER_SIZE)) {
        return false;
    }

    head = g_txHead;
    tail = g_txTail;
    freeBytes = (size_t) ((tail - head - 1U) & IMU_UART_TX_MASK);
    if (length > freeBytes) return false;

    /*
     * 先关闭 TX 中断再修改生产者索引。这里只复制很短的配置命令，不会阻塞等待
     * UART；RX 中断仍保持开启。
     */
    DL_UART_Main_disableInterrupt(CAR_IMU_UART_INST,
                                  DL_UART_MAIN_INTERRUPT_TX);
    for (i = 0U; i < length; ++i) {
        g_txBuffer[head] = data[i];
        head = (uint16_t) ((head + 1U) & IMU_UART_TX_MASK);
    }
    g_txHead = head;
    service_tx_fifo();
    return true;
#else
    (void) data;
    (void) length;
    return false;
#endif
}

bool BSP_IMU_UART_TryReadByte(uint8_t *value)
{
    if ((value == NULL) || (g_rxTail == g_rxHead)) return false;

    *value = g_rxBuffer[g_rxTail];
    g_rxTail = (uint16_t) ((g_rxTail + 1U) & IMU_UART_RX_MASK);
    return true;
}

void BSP_IMU_UART_IRQHandler(void)
{
#if CAR_IMU_UART_READY
    switch (DL_UART_Main_getPendingInterrupt(CAR_IMU_UART_INST)) {
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
        /* 统计为接收错误；下一次 RX 中断会继续排空仍然有效的字节。 */
        g_rxOverflowCount++;
        break;
    default:
        break;
    }
#endif
}

uint32_t BSP_IMU_UART_GetRxOverflowCount(void)
{
    return g_rxOverflowCount;
}

#if CAR_IMU_UART_READY
/* SysConfig 生成的真正中断名通过宏展开；不要再定义第二个同名 IRQHandler。 */
void CAR_IMU_UART_IRQ_HANDLER(void)
{
    BSP_IMU_UART_IRQHandler();
}
#endif
