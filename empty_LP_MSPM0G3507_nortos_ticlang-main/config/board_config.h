#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/*
 * @file board_config.h
 * @brief 硬件接线记录表；所有宏现在已用 SysConfig 选择的实际值填充。
 *
 * 这些宏用于集中记录接线和外设分配，避免接线信息散落在业务代码中。
 * 端口编号：0=GPIOA，1=GPIOB。该编号只用于文档，不应直接强转成 GPIO_Regs 指针。
 *
 * 使用规则：
 * - BSP 层通过 ti_msp_dl_config.h 生成的符号访问 DriverLib API；
 * - CAR_TB6612_*_GPIO_PORT/PIN 宏现在已映射到生成的硬件值；
 * - 未使用的功能仍保持 CAR_PIN_UNASSIGNED。
 */
#define CAR_PIN_UNASSIGNED                  (0xFFFFFFFFUL)

/* =========================================================================
 * TB6612：A 通道驱动左轮，B 通道驱动右轮
 * =========================================================================
 * SysConfig 实例:
 *   TB6612_PWMA = TIMG12 CCP0 (PB13), TB6612_PWMB = TIMG12 CCP1 (PA31)
 *   TB6612_AIN1 = PB0,  TB6612_AIN2 = PB1
 *   TB6612_BIN1 = PB12, TB6612_BIN2 = PB20
 *   TB6612_STBY = PA28
 */
/* TB6612 使用 SysConfig 生成的符号 */
#define CAR_TB6612_PWM_INST                 (MOTOR_PWM_INST)
#define CAR_TB6612_PWMA_CC_INDEX            (GPIO_MOTOR_PWM_C0_IDX)
#define CAR_TB6612_PWMB_CC_INDEX            (GPIO_MOTOR_PWM_C1_IDX)
#define CAR_TB6612_AIN1_GPIO_PORT           (AIN1_PORT)
#define CAR_TB6612_AIN1_GPIO_PIN            (AIN1_PIN_0_PIN)
#define CAR_TB6612_AIN2_GPIO_PORT           (AIN2_PORT)
#define CAR_TB6612_AIN2_GPIO_PIN            (AIN2_PIN_1_PIN)
#define CAR_TB6612_BIN1_GPIO_PORT           (BIN1_PORT)
#define CAR_TB6612_BIN1_GPIO_PIN            (BIN1_PIN_2_PIN)
#define CAR_TB6612_BIN2_GPIO_PORT           (BIN2_PORT)
#define CAR_TB6612_BIN2_GPIO_PIN            (BIN2_PIN_3_PIN)
#define CAR_TB6612_STBY_GPIO_PORT           (STBY_PORT)
#define CAR_TB6612_STBY_GPIO_PIN            (STBY_PIN_4_PIN)
#define CAR_TB6612_PWM_FREQUENCY_HZ         (20000UL)

/* =========================================================================
 * 正交编码器 - 左轮: TIMG8 硬件 QEI (PB6=PHA, PB7=PHB)
 *               右轮: GPIO 双沿中断软件解码 (PB8=PHA, PB9=PHB)
 * ========================================================================= */
#define CAR_ENCODER_READY                   (1U)

#define CAR_ENCODER_LEFT_TIMER_INDEX        (LEFT_ENCODER_QEI_INST)
#define CAR_ENCODER_LEFT_A_GPIO_PORT        (GPIOB)
#define CAR_ENCODER_LEFT_A_GPIO_PIN         (DL_GPIO_PIN_6)
#define CAR_ENCODER_LEFT_B_GPIO_PORT        (GPIOB)
#define CAR_ENCODER_LEFT_B_GPIO_PIN         (DL_GPIO_PIN_7)

#define CAR_ENCODER_RIGHT_TIMER_INDEX       (CAR_PIN_UNASSIGNED)  /* software decode, no timer */
#define CAR_ENCODER_RIGHT_A_GPIO_PORT       (RIGHT_ENCODER_GPIO_PORT)
#define CAR_ENCODER_RIGHT_A_GPIO_PIN        (RIGHT_ENCODER_GPIO_A_PIN)
#define CAR_ENCODER_RIGHT_B_GPIO_PORT       (RIGHT_ENCODER_GPIO_PORT)
#define CAR_ENCODER_RIGHT_B_GPIO_PIN        (RIGHT_ENCODER_GPIO_B_PIN)

/*
 * 八路模拟循迹阵列：使用 ADC0 和 ADC1 的两个非重复序列采集真实 12 位模拟量。
 * 实例名为 LINE_ADC0 和 LINE_ADC1；ADC0/MEM0~2 三路、ADC1/MEM0~4 五路。
 * 两个序列都由软件触发，最后一个 MEM 完成时产生中断。
 */
#ifndef CAR_LINE_ADC_READY
#define CAR_LINE_ADC_READY                  (1U)
#endif

#if CAR_LINE_ADC_READY
#ifndef CAR_LINE_ADC0_INST
#define CAR_LINE_ADC0_INST                  (LINE_ADC0_INST)
#endif
#ifndef CAR_LINE_ADC0_IRQN
#define CAR_LINE_ADC0_IRQN                  (LINE_ADC0_INST_INT_IRQN)
#endif
#ifndef CAR_LINE_ADC0_IRQ_HANDLER
#define CAR_LINE_ADC0_IRQ_HANDLER           LINE_ADC0_INST_IRQHandler
#endif

#ifndef CAR_LINE_ADC1_INST
#define CAR_LINE_ADC1_INST                  (LINE_ADC1_INST)
#endif
#ifndef CAR_LINE_ADC1_IRQN
#define CAR_LINE_ADC1_IRQN                  (LINE_ADC1_INST_INT_IRQN)
#endif
#ifndef CAR_LINE_ADC1_IRQ_HANDLER
#define CAR_LINE_ADC1_IRQ_HANDLER           LINE_ADC1_INST_IRQHandler
#endif

#ifndef CAR_LINE_ADC0_DONE_IIDX
#define CAR_LINE_ADC0_DONE_IIDX             DL_ADC12_IIDX_MEM2_RESULT_LOADED
#endif
#ifndef CAR_LINE_ADC0_DONE_INTERRUPT
#define CAR_LINE_ADC0_DONE_INTERRUPT        DL_ADC12_INTERRUPT_MEM2_RESULT_LOADED
#endif
#ifndef CAR_LINE_ADC1_DONE_IIDX
#define CAR_LINE_ADC1_DONE_IIDX             DL_ADC12_IIDX_MEM4_RESULT_LOADED
#endif
#ifndef CAR_LINE_ADC1_DONE_INTERRUPT
#define CAR_LINE_ADC1_DONE_INTERRUPT        DL_ADC12_INTERRUPT_MEM4_RESULT_LOADED
#endif
#endif

/*
 * NoRTOS 1 ms 单调时基：TICK_TIMER 使用 TIMG0，BUSCLK/1、周期 1 ms、ZERO 中断。
 */
#ifndef CAR_TIMEBASE_READY
#define CAR_TIMEBASE_READY                  (1U)
#endif

#if CAR_TIMEBASE_READY
#ifndef CAR_TIMEBASE_INST
#define CAR_TIMEBASE_INST                   (TICK_TIMER_INST)
#endif
#ifndef CAR_TIMEBASE_IRQN
#define CAR_TIMEBASE_IRQN                   (TICK_TIMER_INST_INT_IRQN)
#endif
#ifndef CAR_TIMEBASE_IRQ_HANDLER
#define CAR_TIMEBASE_IRQ_HANDLER            TICK_TIMER_INST_IRQHandler
#endif
#endif

/*
 * 硬件 I2C1：PB2=SCL、PB3=SDA（预留，SysConfig 尚未实例化）。
 * SCL/SDA 必须外接 4.7k 上拉到 3.3 V。
 */
#define CAR_I2C_INSTANCE_INDEX               (1U)
#define CAR_I2C_SCL_GPIO_PORT                (1U)  /* 1 = GPIOB */
#define CAR_I2C_SCL_GPIO_PIN                 (2U)  /* PB2 */
#define CAR_I2C_SDA_GPIO_PORT                (1U)  /* 1 = GPIOB */
#define CAR_I2C_SDA_GPIO_PIN                 (3U)  /* PB3 */
#define CAR_I2C_BITRATE_HZ                   (400000UL)

/*
 * 预留 UART2：PB15=TX、PB16=RX（SysConfig 尚未实例化）。
 */
#define CAR_SPARE_UART_INSTANCE_INDEX        (2U)
#define CAR_SPARE_UART_TX_GPIO_PORT          (1U)  /* GPIOB */
#define CAR_SPARE_UART_TX_GPIO_PIN           (15U) /* PB15 */
#define CAR_SPARE_UART_RX_GPIO_PORT          (1U)  /* GPIOB */
#define CAR_SPARE_UART_RX_GPIO_PIN           (16U) /* PB16 */

/*
 * 100 Hz 串口 IMU：9600、8N1 (JY61P 默认)。
 * SysConfig 实例名 IMU_UART，使用 UART0、PA10 TX、PA11 RX。
 */
#ifndef CAR_IMU_UART_READY
#define CAR_IMU_UART_READY                  (1U)
#endif
#define CAR_IMU_UART_BAUD_RATE              (9600UL)

#if CAR_IMU_UART_READY
#ifndef CAR_IMU_UART_INST
#define CAR_IMU_UART_INST                   (IMU_UART_INST)
#endif
#ifndef CAR_IMU_UART_IRQN
#define CAR_IMU_UART_IRQN                   (IMU_UART_INST_INT_IRQN)
#endif
#ifndef CAR_IMU_UART_IRQ_HANDLER
#define CAR_IMU_UART_IRQ_HANDLER            IMU_UART_INST_IRQHandler
#endif
#endif

/*
 * OLED 软件 I2C：SCL=PA12, SDA=PA13。
 * 两个引脚在 SysConfig 中配置为普通数字 GPIO，总线上需外接 4.7kΩ 上拉到 3.3V。
 * BSP 通过"输出低/切换为输入高阻"模拟开漏，不会主动推挽输出高电平。
 */
#ifndef CAR_OLED_SOFT_I2C_READY
#define CAR_OLED_SOFT_I2C_READY              (1U)
#endif
#ifndef CAR_OLED_I2C_ADDRESS_7BIT
#define CAR_OLED_I2C_ADDRESS_7BIT            (0x3CU)
#endif
#ifndef CAR_OLED_SOFT_I2C_DELAY_CYCLES
#define CAR_OLED_SOFT_I2C_DELAY_CYCLES       (40U)
#endif

#if CAR_OLED_SOFT_I2C_READY
#ifndef CAR_OLED_SCL_PORT
#define CAR_OLED_SCL_PORT                    (OLED_GPIO_PORT)
#endif
#ifndef CAR_OLED_SCL_PIN
#define CAR_OLED_SCL_PIN                     (OLED_GPIO_SCL_PIN)
#endif
#ifndef CAR_OLED_SDA_PORT
#define CAR_OLED_SDA_PORT                    (OLED_GPIO_PORT)
#endif
#ifndef CAR_OLED_SDA_PIN
#define CAR_OLED_SDA_PIN                     (OLED_GPIO_SDA_PIN)
#endif
#endif

/*
 * HC-05 蓝牙串口透传：SysConfig 实例 HC05_UART，UART1、PB4 TX、PB5 RX。
 */
#ifndef CAR_HC05_UART_READY
#define CAR_HC05_UART_READY                 (1U)
#endif
#define CAR_HC05_UART_BAUD_RATE             (9600UL)

#if CAR_HC05_UART_READY
#ifndef CAR_HC05_UART_INST
#define CAR_HC05_UART_INST                  (HC05_UART_INST)
#endif
#ifndef CAR_HC05_UART_IRQN
#define CAR_HC05_UART_IRQN                  (HC05_UART_INST_INT_IRQN)
#endif
#ifndef CAR_HC05_UART_IRQ_HANDLER
#define CAR_HC05_UART_IRQ_HANDLER           HC05_UART_INST_IRQHandler
#endif
#endif

/* 可选启停按键与电池电压 ADC；驱动尚未实现，暂保留为 UNASSIGNED。 */
#define CAR_KEY_GPIO_PORT                    CAR_PIN_UNASSIGNED
#define CAR_KEY_GPIO_PIN                     CAR_PIN_UNASSIGNED
#define CAR_BATTERY_ADC_CHANNEL              CAR_PIN_UNASSIGNED

#endif /* BOARD_CONFIG_H */
