#ifndef BSP_OLED_SOFT_I2C_H
#define BSP_OLED_SOFT_I2C_H

#include <stdbool.h>

/**
 * @file bsp_oled_soft_i2c.h
 * @brief SSD1306 OLED 专用的软件 I2C GPIO 边界。
 *
 * 写高电平表示“释放总线”，写低电平表示主动下拉，符合 I2C 开漏要求。
 * 微秒级位间隔是软件 I2C 时序，不是 MCU 休眠或低功耗入口。
 */

void BSP_OLED_SoftI2C_Init(void);
void BSP_OLED_SoftI2C_WriteSCL(bool high);
void BSP_OLED_SoftI2C_WriteSDA(bool high);
bool BSP_OLED_SoftI2C_ReadSDA(void);

#endif /* BSP_OLED_SOFT_I2C_H */
