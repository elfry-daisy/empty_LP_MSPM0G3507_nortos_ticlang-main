/*
 * 串口 IMU 驱动 (9600 baud, UART0 PA10/PA11).
 *
 * 帧格式 (9 字节):
 *   0x0A 0x03 0x04 D0 D1 D2 D3 CRC_L CRC_H
 *   D0=D1: 角度 (int16, big-endian)
 *   D2=D3: 角速度 (int16, big-endian)
 *   CRC: CRC-16/MODBUS 标准
 *
 * 缩放: 角度 = raw / 32768 * 180°, 角速度 = raw / 32768 * 2000°/s
 */
#include "drivers/imu.h"

#include <stddef.h>
#include "bsp/bsp_imu_uart.h"
#include "config/car_config.h"

#define IMU_FRAME_SIZE  9U
#define IMU_HDR0        0x0AU
#define IMU_HDR1        0x03U
#define IMU_HDR2        0x04U

/* CRC-16/MODBUS 查找表 */
static const uint16_t g_crc16Table[256] = {
    0x0000,0xC0C1,0xC181,0x0140,0xC301,0x03C0,0x0280,0xC241,
    0xC601,0x06C0,0x0780,0xC741,0x0500,0xC5C1,0xC481,0x0440,
    0xCC01,0x0CC0,0x0D80,0xCD41,0x0F00,0xCFC1,0xCE81,0x0E40,
    0x0A00,0xCAC1,0xCB81,0x0B40,0xC901,0x09C0,0x0880,0xC841,
    0xD801,0x18C0,0x1980,0xD941,0x1B00,0xDBC1,0xDA81,0x1A40,
    0x1E00,0xDEC1,0xDF81,0x1F40,0xDD01,0x1DC0,0x1C80,0xDC41,
    0x1400,0xD4C1,0xD581,0x1540,0xD701,0x17C0,0x1680,0xD641,
    0xD201,0x12C0,0x1380,0xD341,0x1100,0xD1C1,0xD081,0x1040,
    0xF001,0x30C0,0x3180,0xF141,0x3300,0xF3C1,0xF281,0x3240,
    0x3600,0xF6C1,0xF781,0x3740,0xF501,0x35C0,0x3480,0xF441,
    0x3C00,0xFCC1,0xFD81,0x3D40,0xFF01,0x3FC0,0x3E80,0xFE41,
    0xFA01,0x3AC0,0x3B80,0xFB41,0x3900,0xF9C1,0xF881,0x3840,
    0x2800,0xE8C1,0xE981,0x2940,0xEB01,0x2BC0,0x2A80,0xEA41,
    0xEE01,0x2EC0,0x2F80,0xEF41,0x2D00,0xEDC1,0xEC81,0x2C40,
    0xE401,0x24C0,0x2580,0xE541,0x2700,0xE7C1,0xE681,0x2640,
    0x2200,0xE2C1,0xE381,0x2340,0xE101,0x21C0,0x2080,0xE041,
    0xA001,0x60C0,0x6180,0xA141,0x6300,0xA3C1,0xA281,0x6240,
    0x6600,0xA6C1,0xA781,0x6740,0xA501,0x65C0,0x6480,0xA441,
    0x6C00,0xACC1,0xAD81,0x6D40,0xAF01,0x6FC0,0x6E80,0xAE41,
    0xAA01,0x6AC0,0x6B80,0xAB41,0x6900,0xA9C1,0xA881,0x6840,
    0x7800,0xB8C1,0xB981,0x7940,0xBB01,0x7BC0,0x7A80,0xBA41,
    0xBE01,0x7EC0,0x7F80,0xBF41,0x7D00,0xBDC1,0xBC81,0x7C40,
    0xB401,0x74C0,0x7580,0xB541,0x7700,0xB7C1,0xB681,0x7640,
    0x7200,0xB2C1,0xB381,0x7340,0xB101,0x71C0,0x7080,0xB041,
    0x5000,0x90C1,0x9181,0x5140,0x9301,0x53C0,0x5280,0x9241,
    0x9601,0x56C0,0x5780,0x9741,0x5500,0x95C1,0x9481,0x5440,
    0x9C01,0x5CC0,0x5D80,0x9D41,0x5F00,0x9FC1,0x9E81,0x5E40,
    0x5A00,0x9AC1,0x9B81,0x5B40,0x9901,0x59C0,0x5880,0x9841,
    0x8801,0x48C0,0x4980,0x8941,0x4B00,0x8BC1,0x8A81,0x4A40,
    0x4E00,0x8EC1,0x8F81,0x4F40,0x8D01,0x4DC0,0x4C80,0x8C41,
    0x4400,0x84C1,0x8581,0x4540,0x8701,0x47C0,0x4680,0x8641,
    0x8201,0x42C0,0x4380,0x8341,0x4100,0x81C1,0x8081,0x4040
};

static uint16_t CRC16(const uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    uint8_t i;
    for (i = 0U; i < len; i++)
        crc = (crc >> 8) ^ g_crc16Table[(crc ^ data[i]) & 0xFF];
    return crc;
}

/* 解析器 */
typedef struct {
    uint8_t  bytes[IMU_FRAME_SIZE];
    uint8_t  index;
    uint8_t  state;
} IMU_Parser;

/* 发布区：无锁读取 */
static volatile int16_t  g_pubAngleRaw;
static volatile int16_t  g_pubDpsRaw;
static volatile uint32_t g_pubSeq;

static IMU_Parser       g_parser;
static IMU_Data         g_data;
static IMU_Diagnostics  g_diag;
static uint32_t         g_consSeq;
static int16_t          g_lastAngleRaw;
static bool             g_haveLastAngle;
static float            g_ageSec;
static bool             g_uartOk;
static float            g_calSumDps;

/* ---- 帧解析 ---- */
bool IMU_PushRxByte(uint8_t b)
{
    switch (g_parser.state) {
    case 0:
        if (b == IMU_HDR0) { g_parser.bytes[0] = b; g_parser.state = 1; }
        return false;
    case 1:
        if (b == IMU_HDR1) { g_parser.bytes[1] = b; g_parser.state = 2; }
        else { g_parser.state = 0; }
        return false;
    case 2:
        if (b == IMU_HDR2) { g_parser.bytes[2] = b; g_parser.index = 3; g_parser.state = 3; }
        else { g_parser.state = 0; }
        return false;
    case 3:
        g_parser.bytes[g_parser.index++] = b;
        if (g_parser.index >= IMU_FRAME_SIZE) {
            g_parser.state = 0;
            /* 校验 CRC */
            uint16_t calc = CRC16(g_parser.bytes, 7);
            uint16_t recv = (uint16_t)g_parser.bytes[7] |
                           ((uint16_t)g_parser.bytes[8] << 8);
            if (calc == recv) {
                /* 角度: int16 big-endian */
                g_pubAngleRaw = (int16_t)(((uint16_t)g_parser.bytes[3] << 8) |
                                            g_parser.bytes[4]);
                /* 角速度: int16 big-endian */
                g_pubDpsRaw = (int16_t)(((uint16_t)g_parser.bytes[5] << 8) |
                                          g_parser.bytes[6]);
                g_diag.validFrames++;
                g_pubSeq++;
                return true;
            } else {
                g_diag.crcErrors++;
            }
        }
        return false;
    default:
        g_parser.state = 0;
        return false;
    }
}

/* ---- 无锁读取 ---- */
static bool copy_latest(int16_t *a, int16_t *d, uint32_t *s)
{
    uint32_t b, af;
    uint8_t n;
    for (n = 0U; n < 3U; n++) {
        b  = g_pubSeq;
        *a = g_pubAngleRaw;
        *d = g_pubDpsRaw;
        af = g_pubSeq;
        if (b == af) { *s = af; return true; }
    }
    return false;
}

static bool consume_latest(void)
{
    int16_t  a, d;
    uint32_t s;

    if (!copy_latest(&a, &d, &s) || (s == g_consSeq)) return false;

    if ((g_consSeq != 0U) && ((uint32_t)(s - g_consSeq) > 1U))
        g_diag.droppedFrames += (uint32_t)(s - g_consSeq - 1U);
    g_consSeq = s;

    g_data.sensorAngleDegrees = (float)a / 32768.0f * 180.0f
                                * CAR_IMU_YAW_POLARITY;

    if (!g_haveLastAngle) {
        g_lastAngleRaw   = a;
        g_haveLastAngle  = true;
    } else {
        int16_t delta = (int16_t)((uint16_t)a - (uint16_t)g_lastAngleRaw);
        g_data.yawDegrees += (float)delta / 32768.0f * 180.0f
                             * CAR_IMU_YAW_POLARITY;
        g_lastAngleRaw = a;
    }

    float dps = (float)d / 32768.0f * 2000.0f * CAR_IMU_YAW_POLARITY;
    if (g_diag.calibrating) {
        g_calSumDps += dps;
        g_diag.calibrationSamples++;
        if (g_diag.calibrationSamples >= CAR_IMU_CALIBRATION_SAMPLES) {
            g_data.gyroBiasDps = g_calSumDps
                               / (float)g_diag.calibrationSamples;
            g_diag.calibrating = false;
        }
    }
    g_data.gyroZDps   = dps - g_data.gyroBiasDps;
    g_data.valid      = true;
    g_data.newSample  = true;
    g_ageSec          = 0.0f;
    return true;
}

/* ---- 公开 API ---- */
bool IMU_Init(void)
{
    g_parser = (IMU_Parser){0};
    g_data   = (IMU_Data){0};
    g_diag   = (IMU_Diagnostics){0};
    g_pubAngleRaw = 0;
    g_pubDpsRaw   = 0;
    g_pubSeq      = 0U;
    g_consSeq     = 0U;
    g_lastAngleRaw    = 0;
    g_haveLastAngle   = false;
    g_ageSec          = 0.0f;
    g_calSumDps       = 0.0f;
    g_uartOk = BSP_IMU_UART_Init();
    /* 发送配置命令：AA 06 01 01 01 AD 00，使能输出 */
    {
        static const uint8_t cfg[] = {0xAA,0x06,0x01,0x01,0x01,0xAD,0x00};
        if (g_uartOk) {
            g_diag.configCommandQueued = BSP_IMU_UART_TryWrite(cfg, sizeof(cfg));
        }
    }
    return g_uartOk;
}

bool IMU_Update(float dt)
{
    uint8_t  b;
    uint16_t n = 0U;

    if (dt < 0.0f) dt = 0.0f;
    g_data.newSample = false;

    /* 若配置命令未发出，重试 */
    if (!g_diag.configCommandQueued) {
        static const uint8_t cfg[] = {0xAA,0x06,0x01,0x01,0x01,0xAD,0x00};
        g_diag.configCommandQueued = BSP_IMU_UART_TryWrite(cfg, sizeof(cfg));
    }

    while ((n < CAR_IMU_MAX_BYTES_PER_UPDATE)
           && BSP_IMU_UART_TryReadByte(&b)) {
        (void)IMU_PushRxByte(b);
        n++;
    }

    bool ok = consume_latest();
    if (!ok) {
        g_ageSec += dt;
        if (g_ageSec > ((float)CAR_IMU_DATA_TIMEOUT_MS * 0.001f))
            g_data.valid = false;
    }
    g_diag.uartRxOverflows = BSP_IMU_UART_GetRxOverflowCount();
    return ok;
}

void IMU_StartGyroCalibration(void)
{
    g_calSumDps = 0.0f;
    g_diag.calibrationSamples = 0U;
    g_diag.calibrating = true;
}

void IMU_ResetYaw(void) { g_data.yawDegrees = 0.0f; }
const IMU_Data *IMU_GetData(void) { return &g_data; }
const IMU_Diagnostics *IMU_GetDiagnostics(void) { return &g_diag; }
