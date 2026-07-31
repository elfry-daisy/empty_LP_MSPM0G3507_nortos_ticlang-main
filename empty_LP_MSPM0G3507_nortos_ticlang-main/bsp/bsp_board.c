/*
 * 整板补充初始化 + 三按键。
 * 按键1 PA26: 切任务   按键2 PA27: 启动/停车   按键3 PA14: 标定
 * GPIO 初始化已由 SYSCFG_DL_GPIO_init() 完成。
 */
#include "bsp/bsp_board.h"
#include "ti_msp_dl_config.h"

static void btn_init(uint32_t iomux)
{
    DL_GPIO_initDigitalInputFeatures(iomux,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
}

void BSP_Board_Init(void)
{
    btn_init(IOMUX_PINCM59);  /* PA26: 按键1 切任务 */
    btn_init(IOMUX_PINCM60);  /* PA27: 按键2 启动/停车 */
    btn_init(IOMUX_PINCM36);  /* PA14: 按键3 标定 */
}
