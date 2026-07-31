/*
 * 可选 I2C 总线适配层。
 *
 * IMU 和 OLED 可能共用该总线。实现时必须保证 OLED 的长传输不会长期阻塞 IMU
 * 或 5 ms 控制任务；可采用分段状态机、中断或带严格超时的短事务。
 */
#include "bsp/bsp_i2c.h"

void BSP_I2C_Init(void)
{
    /* TODO：启动 empty.syscfg 中配置的 400 kHz I2C Controller。 */
}

bool BSP_I2C_Write(uint8_t address, const uint8_t *data, size_t length)
{
    (void) address;
    (void) data;
    (void) length;
    /* TODO：执行有界/非阻塞写事务，并正确报告 NACK、仲裁丢失和超时。 */
    return false;
}

bool BSP_I2C_WriteRead(uint8_t address, const uint8_t *writeData,
                       size_t writeLength, uint8_t *readData,
                       size_t readLength)
{
    (void) address;
    (void) writeData;
    (void) writeLength;
    (void) readData;
    (void) readLength;
    /* TODO：为目标 IMU 实现 repeated-start 寄存器读。 */
    return false;
}
