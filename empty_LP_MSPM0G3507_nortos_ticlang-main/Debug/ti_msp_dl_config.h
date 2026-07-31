/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2500
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for MOTOR_PWM */
#define MOTOR_PWM_INST                                                    TIMG12
#define MOTOR_PWM_INST_IRQHandler                              TIMG12_IRQHandler
#define MOTOR_PWM_INST_INT_IRQN                                (TIMG12_INT_IRQn)
#define MOTOR_PWM_INST_CLK_FREQ                                         80000000
/* GPIO defines for channel 0 */
#define GPIO_MOTOR_PWM_C0_PORT                                             GPIOB
#define GPIO_MOTOR_PWM_C0_PIN                                     DL_GPIO_PIN_13
#define GPIO_MOTOR_PWM_C0_IOMUX                                  (IOMUX_PINCM30)
#define GPIO_MOTOR_PWM_C0_IOMUX_FUNC                IOMUX_PINCM30_PF_TIMG12_CCP0
#define GPIO_MOTOR_PWM_C0_IDX                                DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_MOTOR_PWM_C1_PORT                                             GPIOA
#define GPIO_MOTOR_PWM_C1_PIN                                     DL_GPIO_PIN_31
#define GPIO_MOTOR_PWM_C1_IOMUX                                   (IOMUX_PINCM6)
#define GPIO_MOTOR_PWM_C1_IOMUX_FUNC                 IOMUX_PINCM6_PF_TIMG12_CCP1
#define GPIO_MOTOR_PWM_C1_IDX                                DL_TIMER_CC_1_INDEX




/* Defines for LEFT_ENCODER_QEI */
#define LEFT_ENCODER_QEI_INST                                              TIMG8
#define LEFT_ENCODER_QEI_INST_IRQHandler                        TIMG8_IRQHandler
#define LEFT_ENCODER_QEI_INST_INT_IRQN                          (TIMG8_INT_IRQn)
/* Pin configuration defines for LEFT_ENCODER_QEI PHA Pin */
#define GPIO_LEFT_ENCODER_QEI_PHA_PORT                                     GPIOB
#define GPIO_LEFT_ENCODER_QEI_PHA_PIN                              DL_GPIO_PIN_6
#define GPIO_LEFT_ENCODER_QEI_PHA_IOMUX                          (IOMUX_PINCM23)
#define GPIO_LEFT_ENCODER_QEI_PHA_IOMUX_FUNC             IOMUX_PINCM23_PF_TIMG8_CCP0
/* Pin configuration defines for LEFT_ENCODER_QEI PHB Pin */
#define GPIO_LEFT_ENCODER_QEI_PHB_PORT                                     GPIOB
#define GPIO_LEFT_ENCODER_QEI_PHB_PIN                              DL_GPIO_PIN_7
#define GPIO_LEFT_ENCODER_QEI_PHB_IOMUX                          (IOMUX_PINCM24)
#define GPIO_LEFT_ENCODER_QEI_PHB_IOMUX_FUNC             IOMUX_PINCM24_PF_TIMG8_CCP1


/* Defines for TICK_TIMER */
#define TICK_TIMER_INST                                                  (TIMG0)
#define TICK_TIMER_INST_IRQHandler                              TIMG0_IRQHandler
#define TICK_TIMER_INST_INT_IRQN                                (TIMG0_INT_IRQn)
#define TICK_TIMER_INST_LOAD_VALUE                                      (39999U)



/* Defines for IMU_UART */
#define IMU_UART_INST                                                      UART0
#define IMU_UART_INST_FREQUENCY                                         40000000
#define IMU_UART_INST_IRQHandler                                UART0_IRQHandler
#define IMU_UART_INST_INT_IRQN                                    UART0_INT_IRQn
#define GPIO_IMU_UART_RX_PORT                                              GPIOA
#define GPIO_IMU_UART_TX_PORT                                              GPIOA
#define GPIO_IMU_UART_RX_PIN                                      DL_GPIO_PIN_11
#define GPIO_IMU_UART_TX_PIN                                      DL_GPIO_PIN_10
#define GPIO_IMU_UART_IOMUX_RX                                   (IOMUX_PINCM22)
#define GPIO_IMU_UART_IOMUX_TX                                   (IOMUX_PINCM21)
#define GPIO_IMU_UART_IOMUX_RX_FUNC                    IOMUX_PINCM22_PF_UART0_RX
#define GPIO_IMU_UART_IOMUX_TX_FUNC                    IOMUX_PINCM21_PF_UART0_TX
#define IMU_UART_BAUD_RATE                                                (9600)
#define IMU_UART_IBRD_40_MHZ_9600_BAUD                                     (260)
#define IMU_UART_FBRD_40_MHZ_9600_BAUD                                      (27)
/* Defines for HC05_UART */
#define HC05_UART_INST                                                     UART1
#define HC05_UART_INST_FREQUENCY                                        40000000
#define HC05_UART_INST_IRQHandler                               UART1_IRQHandler
#define HC05_UART_INST_INT_IRQN                                   UART1_INT_IRQn
#define GPIO_HC05_UART_RX_PORT                                             GPIOB
#define GPIO_HC05_UART_TX_PORT                                             GPIOB
#define GPIO_HC05_UART_RX_PIN                                      DL_GPIO_PIN_5
#define GPIO_HC05_UART_TX_PIN                                      DL_GPIO_PIN_4
#define GPIO_HC05_UART_IOMUX_RX                                  (IOMUX_PINCM18)
#define GPIO_HC05_UART_IOMUX_TX                                  (IOMUX_PINCM17)
#define GPIO_HC05_UART_IOMUX_RX_FUNC                   IOMUX_PINCM18_PF_UART1_RX
#define GPIO_HC05_UART_IOMUX_TX_FUNC                   IOMUX_PINCM17_PF_UART1_TX
#define HC05_UART_BAUD_RATE                                               (9600)
#define HC05_UART_IBRD_40_MHZ_9600_BAUD                                    (260)
#define HC05_UART_FBRD_40_MHZ_9600_BAUD                                     (27)





/* Defines for LINE_ADC0 */
#define LINE_ADC0_INST                                                      ADC0
#define LINE_ADC0_INST_IRQHandler                                ADC0_IRQHandler
#define LINE_ADC0_INST_INT_IRQN                                  (ADC0_INT_IRQn)
#define LINE_ADC0_ADCMEM_0                                    DL_ADC12_MEM_IDX_0
#define LINE_ADC0_ADCMEM_0_REF                   DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define LINE_ADC0_ADCMEM_0_REF_VOLTAGE_V                                     3.3
#define LINE_ADC0_ADCMEM_1                                    DL_ADC12_MEM_IDX_1
#define LINE_ADC0_ADCMEM_1_REF                   DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define LINE_ADC0_ADCMEM_1_REF_VOLTAGE_V                                     3.3
#define LINE_ADC0_ADCMEM_2                                    DL_ADC12_MEM_IDX_2
#define LINE_ADC0_ADCMEM_2_REF                   DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define LINE_ADC0_ADCMEM_2_REF_VOLTAGE_V                                     3.3
#define GPIO_LINE_ADC0_C2_PORT                                             GPIOA
#define GPIO_LINE_ADC0_C2_PIN                                     DL_GPIO_PIN_25
#define GPIO_LINE_ADC0_IOMUX_C2                                  (IOMUX_PINCM55)
#define GPIO_LINE_ADC0_IOMUX_C2_FUNC              (IOMUX_PINCM55_PF_UNCONNECTED)
#define GPIO_LINE_ADC0_C3_PORT                                             GPIOA
#define GPIO_LINE_ADC0_C3_PIN                                     DL_GPIO_PIN_24
#define GPIO_LINE_ADC0_IOMUX_C3                                  (IOMUX_PINCM54)
#define GPIO_LINE_ADC0_IOMUX_C3_FUNC              (IOMUX_PINCM54_PF_UNCONNECTED)
#define GPIO_LINE_ADC0_C7_PORT                                             GPIOA
#define GPIO_LINE_ADC0_C7_PIN                                     DL_GPIO_PIN_22
#define GPIO_LINE_ADC0_IOMUX_C7                                  (IOMUX_PINCM47)
#define GPIO_LINE_ADC0_IOMUX_C7_FUNC              (IOMUX_PINCM47_PF_UNCONNECTED)

/* Defines for LINE_ADC1 */
#define LINE_ADC1_INST                                                      ADC1
#define LINE_ADC1_INST_IRQHandler                                ADC1_IRQHandler
#define LINE_ADC1_INST_INT_IRQN                                  (ADC1_INT_IRQn)
#define LINE_ADC1_ADCMEM_0                                    DL_ADC12_MEM_IDX_0
#define LINE_ADC1_ADCMEM_0_REF                   DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define LINE_ADC1_ADCMEM_0_REF_VOLTAGE_V                                     3.3
#define LINE_ADC1_ADCMEM_1                                    DL_ADC12_MEM_IDX_1
#define LINE_ADC1_ADCMEM_1_REF                   DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define LINE_ADC1_ADCMEM_1_REF_VOLTAGE_V                                     3.3
#define LINE_ADC1_ADCMEM_2                                    DL_ADC12_MEM_IDX_2
#define LINE_ADC1_ADCMEM_2_REF                   DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define LINE_ADC1_ADCMEM_2_REF_VOLTAGE_V                                     3.3
#define LINE_ADC1_ADCMEM_3                                    DL_ADC12_MEM_IDX_3
#define LINE_ADC1_ADCMEM_3_REF                   DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define LINE_ADC1_ADCMEM_3_REF_VOLTAGE_V                                     3.3
#define LINE_ADC1_ADCMEM_4                                    DL_ADC12_MEM_IDX_4
#define LINE_ADC1_ADCMEM_4_REF                   DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define LINE_ADC1_ADCMEM_4_REF_VOLTAGE_V                                     3.3
#define GPIO_LINE_ADC1_C0_PORT                                             GPIOA
#define GPIO_LINE_ADC1_C0_PIN                                     DL_GPIO_PIN_15
#define GPIO_LINE_ADC1_IOMUX_C0                                  (IOMUX_PINCM37)
#define GPIO_LINE_ADC1_IOMUX_C0_FUNC              (IOMUX_PINCM37_PF_UNCONNECTED)
#define GPIO_LINE_ADC1_C2_PORT                                             GPIOA
#define GPIO_LINE_ADC1_C2_PIN                                     DL_GPIO_PIN_17
#define GPIO_LINE_ADC1_IOMUX_C2                                  (IOMUX_PINCM39)
#define GPIO_LINE_ADC1_IOMUX_C2_FUNC              (IOMUX_PINCM39_PF_UNCONNECTED)
#define GPIO_LINE_ADC1_C4_PORT                                             GPIOB
#define GPIO_LINE_ADC1_C4_PIN                                     DL_GPIO_PIN_17
#define GPIO_LINE_ADC1_IOMUX_C4                                  (IOMUX_PINCM43)
#define GPIO_LINE_ADC1_IOMUX_C4_FUNC              (IOMUX_PINCM43_PF_UNCONNECTED)
#define GPIO_LINE_ADC1_C5_PORT                                             GPIOB
#define GPIO_LINE_ADC1_C5_PIN                                     DL_GPIO_PIN_18
#define GPIO_LINE_ADC1_IOMUX_C5                                  (IOMUX_PINCM44)
#define GPIO_LINE_ADC1_IOMUX_C5_FUNC              (IOMUX_PINCM44_PF_UNCONNECTED)
#define GPIO_LINE_ADC1_C6_PORT                                             GPIOB
#define GPIO_LINE_ADC1_C6_PIN                                     DL_GPIO_PIN_19
#define GPIO_LINE_ADC1_IOMUX_C6                                  (IOMUX_PINCM45)
#define GPIO_LINE_ADC1_IOMUX_C6_FUNC              (IOMUX_PINCM45_PF_UNCONNECTED)



/* Port definition for Pin Group AIN1 */
#define AIN1_PORT                                                        (GPIOB)

/* Defines for PIN_0: GPIOB.0 with pinCMx 12 on package pin 47 */
#define AIN1_PIN_0_PIN                                           (DL_GPIO_PIN_0)
#define AIN1_PIN_0_IOMUX                                         (IOMUX_PINCM12)
/* Port definition for Pin Group AIN2 */
#define AIN2_PORT                                                        (GPIOB)

/* Defines for PIN_1: GPIOB.1 with pinCMx 13 on package pin 48 */
#define AIN2_PIN_1_PIN                                           (DL_GPIO_PIN_1)
#define AIN2_PIN_1_IOMUX                                         (IOMUX_PINCM13)
/* Port definition for Pin Group BIN1 */
#define BIN1_PORT                                                        (GPIOB)

/* Defines for PIN_2: GPIOB.12 with pinCMx 29 on package pin 64 */
#define BIN1_PIN_2_PIN                                          (DL_GPIO_PIN_12)
#define BIN1_PIN_2_IOMUX                                         (IOMUX_PINCM29)
/* Port definition for Pin Group BIN2 */
#define BIN2_PORT                                                        (GPIOB)

/* Defines for PIN_3: GPIOB.20 with pinCMx 48 on package pin 19 */
#define BIN2_PIN_3_PIN                                          (DL_GPIO_PIN_20)
#define BIN2_PIN_3_IOMUX                                         (IOMUX_PINCM48)
/* Port definition for Pin Group STBY */
#define STBY_PORT                                                        (GPIOA)

/* Defines for PIN_4: GPIOA.28 with pinCMx 3 on package pin 35 */
#define STBY_PIN_4_PIN                                          (DL_GPIO_PIN_28)
#define STBY_PIN_4_IOMUX                                          (IOMUX_PINCM3)
/* Port definition for Pin Group OLED_GPIO */
#define OLED_GPIO_PORT                                                   (GPIOA)

/* Defines for SCL: GPIOA.12 with pinCMx 34 on package pin 5 */
#define OLED_GPIO_SCL_PIN                                       (DL_GPIO_PIN_12)
#define OLED_GPIO_SCL_IOMUX                                      (IOMUX_PINCM34)
/* Defines for SDA: GPIOA.13 with pinCMx 35 on package pin 6 */
#define OLED_GPIO_SDA_PIN                                       (DL_GPIO_PIN_13)
#define OLED_GPIO_SDA_IOMUX                                      (IOMUX_PINCM35)
/* Port definition for Pin Group RIGHT_ENCODER_GPIO */
#define RIGHT_ENCODER_GPIO_PORT                                          (GPIOB)

/* Defines for A: GPIOB.8 with pinCMx 25 on package pin 60 */
// pins affected by this interrupt request:["A","B"]
#define RIGHT_ENCODER_GPIO_INT_IRQN                             (GPIOB_INT_IRQn)
#define RIGHT_ENCODER_GPIO_INT_IIDX             (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define RIGHT_ENCODER_GPIO_A_IIDX                            (DL_GPIO_IIDX_DIO8)
#define RIGHT_ENCODER_GPIO_A_PIN                                 (DL_GPIO_PIN_8)
#define RIGHT_ENCODER_GPIO_A_IOMUX                               (IOMUX_PINCM25)
/* Defines for B: GPIOB.9 with pinCMx 26 on package pin 61 */
#define RIGHT_ENCODER_GPIO_B_IIDX                            (DL_GPIO_IIDX_DIO9)
#define RIGHT_ENCODER_GPIO_B_PIN                                 (DL_GPIO_PIN_9)
#define RIGHT_ENCODER_GPIO_B_IOMUX                               (IOMUX_PINCM26)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_SYSCTL_CLK_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_MOTOR_PWM_init(void);
void SYSCFG_DL_LEFT_ENCODER_QEI_init(void);
void SYSCFG_DL_TICK_TIMER_init(void);
void SYSCFG_DL_IMU_UART_init(void);
void SYSCFG_DL_HC05_UART_init(void);
void SYSCFG_DL_LINE_ADC0_init(void);
void SYSCFG_DL_LINE_ADC1_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
