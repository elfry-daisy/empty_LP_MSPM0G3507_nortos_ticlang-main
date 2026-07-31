#ifndef BSP_BOARD_H
#define BSP_BOARD_H

/**
 * @file bsp_board.h
 * @brief SysConfig 初始化之后的整板补充初始化入口。
 */

/**
 * @brief 完成 SysConfig 没有自动启动的外设，并保证上电输出安全。
 *
 * 调用时机：SYSCFG_DL_init() 之后、应用调度器启动之前。
 */
void BSP_Board_Init(void);

#endif /* BSP_BOARD_H */
