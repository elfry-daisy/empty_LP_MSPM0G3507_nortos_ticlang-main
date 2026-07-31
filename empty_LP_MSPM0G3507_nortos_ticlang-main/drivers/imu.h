#ifndef IMU_H
#define IMU_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file imu.h
 * @brief 串口陀螺仪协议驱动 (帧头 0A 03 04, CRC-16, 9600 baud)。
 *
 * 角度帧 (0x53): 0x55 0x53 RollL RollH PitchL PitchH YawL YawH TL TH Sum
 * 角速度帧 (0x52): 0x55 0x52 wxL wxH wyL wyH wzL wzH TL TH Sum
 * 数据为有符号 16 位小端，角度 /32768*180(°)，角速度 /32768*2000(°/s)。
 * 校验和 Sum = (全部前 10 字节累加) & 0xFF。
 */

typedef struct {
    /** 已扣除静止零偏的 Z 轴角速度，单位 degrees/second。 */
    float gyroZDps;
    /** 相对 IMU_ResetYaw() 的连续偏航角，自动处理 16 位原始角度回绕。 */
    float yawDegrees;
    /** 模块当前直接上报的角度，范围由有符号 16 位原始值决定。 */
    float sensorAngleDegrees;
    /** 最近一次完成非阻塞静止标定得到的角速度零偏。 */
    float gyroBiasDps;
    /** 当前协议没有温度字段，因此保持 0，temperatureValid=false。 */
    float temperatureC;
    bool temperatureValid;
    /** 最近一帧是否未超时且 CRC 正确。 */
    bool valid;
    /** 本次 IMU_Update() 是否消费了一个新样本。 */
    bool newSample;
} IMU_Data;

typedef struct {
    uint32_t validFrames;
    uint32_t crcErrors;
    uint32_t formatErrors;
    uint32_t droppedFrames;
    uint32_t uartRxOverflows;
    bool configCommandQueued;
    bool calibrating;
    uint16_t calibrationSamples;
} IMU_Diagnostics;

/**
 * @brief 复位协议状态并初始化专用 UART。
 * @return true=UART 已由 SysConfig 配置；false=CAR_IMU_UART_READY 仍为 0。
 * @note 数据要等收到第一帧后才 valid；配置命令会在非阻塞上电延迟后发送。
 */
bool IMU_Init(void);

/**
 * @brief 从 UART 队列取数、解析帧、检查超时并更新物理量。
 * @param dtSeconds 距上次调用经过的秒数；推荐在 1 ms 任务中传 0.001f。
 * @return true=本次得到新有效帧；false=没有新帧或仅处理了无效帧。
 */
bool IMU_Update(float dtSeconds);

/**
 * @brief 向协议状态机输入一个字节。
 * @return true=该字节恰好完成一帧 CRC 正确的数据。
 * @note 常规工程由 IMU_Update() 从 BSP 队列调用；也可在自定义 RX ISR 中直接调用。
 */
bool IMU_PushRxByte(uint8_t byte);

/** @brief 开始收集静止角速度样本；分时完成，不阻塞主循环。 */
void IMU_StartGyroCalibration(void);

/** @brief 把当前连续偏航角定义为 0，不修改模块内部角度。 */
void IMU_ResetYaw(void);

const IMU_Data *IMU_GetData(void);
const IMU_Diagnostics *IMU_GetDiagnostics(void);

#endif /* IMU_H */
