/*
 * MSPM0G3507 软件 I2C (SSD1306 OLED). SCL=PA12, SDA=PA13.
 * 用输出使能模拟开漏：0=拉低+输出, 1=禁止输出(外部上拉).
 */
#include "bsp/bsp_oled_soft_i2c.h"
#include "ti_msp_dl_config.h"

#define DELAY 40U

static void set_pin(GPIO_Regs *p, uint32_t m, bool hi)
{
    if (hi) { DL_GPIO_disableOutput(p, m); }
    else    { DL_GPIO_clearPins(p, m); DL_GPIO_enableOutput(p, m); }
    DL_Common_delayCycles(DELAY);
}

void BSP_OLED_SoftI2C_Init(void)
{
    DL_GPIO_disableOutput(OLED_GPIO_PORT, OLED_GPIO_SCL_PIN);
    DL_GPIO_disableOutput(OLED_GPIO_PORT, OLED_GPIO_SDA_PIN);
    DL_GPIO_clearPins(OLED_GPIO_PORT, OLED_GPIO_SCL_PIN);
    DL_GPIO_clearPins(OLED_GPIO_PORT, OLED_GPIO_SDA_PIN);
    DL_Common_delayCycles(DELAY);
}

void BSP_OLED_SoftI2C_WriteSCL(bool hi) { set_pin(OLED_GPIO_PORT, OLED_GPIO_SCL_PIN, hi); }
void BSP_OLED_SoftI2C_WriteSDA(bool hi) { set_pin(OLED_GPIO_PORT, OLED_GPIO_SDA_PIN, hi); }
bool BSP_OLED_SoftI2C_ReadSDA(void)
{
    return (DL_GPIO_readPins(OLED_GPIO_PORT, OLED_GPIO_SDA_PIN) & OLED_GPIO_SDA_PIN) != 0U;
}
