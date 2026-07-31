#ifndef BSP_I2C_H
#define BSP_I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @file bsp_i2c.h @brief 可选 IMU/OLED 共用 I2C 控制器接口。 */

void BSP_I2C_Init(void);

/** @brief 写事务；实现时必须具有超时或非阻塞完成机制。 */
bool BSP_I2C_Write(uint8_t address, const uint8_t *data, size_t length);

/** @brief repeated-start 寄存器读，适合常见 IMU。 */
bool BSP_I2C_WriteRead(uint8_t address, const uint8_t *writeData,
                       size_t writeLength, uint8_t *readData,
                       size_t readLength);

#endif
