#ifndef CAR_CONFIG_H
#define CAR_CONFIG_H

/*
 * @file car_config.h
 * @brief 整车几何、控制周期、限幅和初始控制参数。
 *
 * 本文件只放"与车辆行为有关"的参数。具体 GPIO、定时器和 ADC 通道应记录在
 * board_config.h，并最终通过 SysConfig 生成的符号接入 BSP。
 *
 * 重要单位约定：
 * - 电机指令：有符号千分比，-1000~+1000；
 * - 速度：mm/s；
 * - 距离和轮径：mm；
 * - 控制周期：秒或毫秒，宏名中明确标出；
 * - 循迹位置：约 -1.0~+1.0。
 */

/* 200 Hz 主控制环；MS 与 S 必须保持一致。 */
#define CAR_CONTROL_PERIOD_MS              (5U)
#define CAR_CONTROL_PERIOD_S               (0.005f)

/* 电机限制。SLEW 表示每个 5 ms 控制周期最多变化多少千分比。 */
#define CAR_MOTOR_DUTY_LIMIT_PERMILLE      (950)
#define CAR_MOTOR_BRAKE_DUTY_PERMILLE      (100)
#define CAR_MOTOR_SLEW_PER_5MS             (35)
#define CAR_MOTOR_LEFT_POLARITY            (+1)
#define CAR_MOTOR_RIGHT_POLARITY           (+1)
#define CAR_MOTOR_DEADBAND_PERMILLE        (0)

/*
 * 编码器参数。
 * COUNTS_PER_WHEEL_REV 已按铭牌参数计算为理论 1560 count/车轮圈。
 * 该值包含编码器 PPR、AB 相四倍频和减速比；厂家对 PPR 的定义可能不同，
 * 上板后仍必须手转车轮一整圈，用 QEI 实际增量确认后再高速闭环运行。
 */
/*
 * MG513-30 减速电机铭牌参数：12 V、额定约 360 mA、堵转约 2.8 A、减速比
 * 1:30、减速后空载约 365±26 RPM，霍尔 AB 相编码器标称 13 PPR。
 *
 * 若 QEI 对 AB 两相都做上升/下降沿四倍频，则理论值：13×30×4=1560 count/轮圈。
 * 厂家对 PPR 的定义偶有差异，仍必须手转车轮一圈实测确认。
 */
#define CAR_MOTOR_RATED_VOLTAGE_V          (12.0f)
#define CAR_MOTOR_RATED_CURRENT_MA         (360U)
#define CAR_MOTOR_STALL_CURRENT_MA         (2800U)
#define CAR_MOTOR_NO_LOAD_OUTPUT_RPM       (365.0f)
#define CAR_MOTOR_GEAR_RATIO               (30.0f)
#define CAR_MOTOR_ENCODER_PPR              (13.0f)
#define CAR_ENCODER_QUADRATURE_FACTOR      (4.0f)
#define CAR_WHEEL_DIAMETER_MM              (65.0f)
#define CAR_ENCODER_COUNTS_PER_WHEEL_REV   (CAR_MOTOR_ENCODER_PPR * \
                                             CAR_MOTOR_GEAR_RATIO * \
                                             CAR_ENCODER_QUADRATURE_FACTOR)
#define CAR_ENCODER_LEFT_POLARITY          (-1)
#define CAR_ENCODER_RIGHT_POLARITY         (+1)

/* 八路真实模拟循迹阵列；公开数组始终按车辆物理最左侧到最右侧排列。 */
#define CAR_LINE_SENSOR_COUNT              (8U)
#define CAR_LINE_ACTIVE_DARK               (1U)
#define CAR_LINE_BLACK_IS_LOW_RAW          (1U)
#define CAR_LINE_ADC0_CHANNEL_COUNT        (3U)
#define CAR_LINE_ADC1_CHANNEL_COUNT        (5U)
#define CAR_LINE_ADC_MAX_VALUE             (4095U)
#define CAR_LINE_ADC_TIMEOUT_POLLS         (5U)
#define CAR_LINE_ADC_REVERSE_ORDER         (0U)
#define CAR_LINE_DETECT_SUM_MIN            (600U)
#define CAR_LINE_ELEMENT_THRESHOLD         (650U)
#define CAR_LINE_LOST_STOP_MS              (1000U)
#define CAR_LINE_LOST_STEER_HOLD_CYCLES    (30U)

/* 方向环生成速度差；速度目标必须≤电机实际可达值（编码器半值，实际约350） */
#define CAR_BASE_SPEED_MM_S                (500.0f)
#define CAR_MIN_CURVE_SPEED_MM_S           (200.0f)
#define CAR_MAX_WHEEL_SPEED_MM_S           (700.0f)
#define CAR_CURVE_SLOWDOWN_GAIN            (150.0f)

/* 传感器位置低通滤波，0=不过滤 1=完全保持旧值 */
#define CAR_LINE_POSITION_FILTER_ALPHA      (0.40f)

/* 方向死区：|steering| 小于此值时置零，抑制直线微振 */
#define CAR_LINE_STEERING_DEADBAND_MM_S     (10.0f)

/*
 * 转向速率限制：每个 5ms 周期 steering 最大变化量（mm/s）。
 * 基础项保证直道丝滑；增益项随 |位置误差| 增大而加大，保证弯道转向力度。
 */
#define CAR_LINE_STEERING_SLEW_BASE_PER_5MS (10)
#define CAR_LINE_STEERING_SLEW_GAIN         (60.0f)

/* FSM 分段加减速 */
#define FSM_STRAIGHT_THRESHOLD             (0.08f)
#define FSM_CURVE_THRESHOLD                (0.20f)
#define FSM_PRE_BRAKE_DIST_MM              (1000U)
#define FSM_CURVE_EXIT_CYCLES              (20U)
#define FSM_ACC_NORMAL                     (300.0f)
#define FSM_DEC_SMOOTH                     (100.0f)
#define FSM_DEC_HARD                       (400.0f)
#define FSM_DEC_PRE_BRAKE                  (300.0f)
#define FSM_DEC_CURVE                      (250.0f)
#define FSM_SPEED_MIN                      (200.0f)
#define FSM_SPEED_MAX                      (650.0f)
#define FSM_SPEED_CURVE_MAX                (400.0f)
#define FSM_CURVE_SLOWDOWN                 (500.0f)

/* 循迹方向环 PD（基于实车测试） */
#define CAR_LINE_STEERING_POLARITY         (-1.0f)
#define CAR_LINE_KP                        (800.0f)
#define CAR_LINE_KI                        (0.0f)
#define CAR_LINE_KD                        (6.0f)
#define CAR_LINE_STEERING_LIMIT_MM_S       (350.0f)

/* 终点停车：编码器校准后按实际赛道调整；减速段保证停车平稳不甩球 */
#define CAR_STOP_DISTANCE_MM               (6140U)
#define CAR_STOP_DECEL_MARGIN_MM           (30U)
#define CAR_STOP_DECEL_MM_S2               (450.0f)
#define CAR_STOP_SPEED_MM_S                (20.0f)
#define CAR_STOP_CRAWL_MM_S                (60.0f)

/* 速度环 PI：KI 消稳态误差、KP 处理瞬态差速 */
#define CAR_SPEED_LEFT_KP                  (1.5f)
#define CAR_SPEED_LEFT_KI                  (0.3f)
#define CAR_SPEED_LEFT_KD                  (0.0f)
#define CAR_SPEED_RIGHT_KP                 (1.5f)
#define CAR_SPEED_RIGHT_KI                 (0.3f)
#define CAR_SPEED_RIGHT_KD                 (0.0f)

/*
 * 串口 IMU 参数。
 * 参考驱动中角度和角速度原始值都乘 0.1；帧频由用户确认为 100 Hz。
 * 上电配置延迟沿用参考程序的 3000 ms，但采用分时状态机，绝不阻塞主循环。
 */
#define CAR_IMU_REPORT_RATE_HZ             (100U)
#define CAR_IMU_RAW_TO_DEG                 (0.1f)
#define CAR_IMU_YAW_POLARITY               (+1.0f)
#define CAR_IMU_DATA_TIMEOUT_MS            (50U)
#define CAR_IMU_STARTUP_CONFIG_DELAY_MS    (3000U)
#define CAR_IMU_CALIBRATION_SAMPLES        (500U)
#define CAR_IMU_MAX_BYTES_PER_UPDATE       (32U)

/* 人机和遥测任务低频运行，避免占用 200 Hz 控制环时间。 */
#define CAR_DEBUG_PERIOD_MS                (200U)
#define CAR_OLED_PERIOD_MS                 (100U)

/*
 * 赛道与任务参数
 * 赛道: 2×1.5m直道 + 2×π×0.5m半圆弯道 ≈ 6.14m
 */
#define CAR_LAP_DISTANCE_MM                (6140U)
#define CAR_AB_DISTANCE_MM                 (1570U)
#define CAR_PARK_TOLERANCE_MM              (20U)

/* 各任务速度档位（蓝牙v/V可在静止时调整当前任务的默认值） */
#define TASK2_BASE_SPEED_MM_S              (700.0f)
#define TASK4_BASE_SPEED_MM_S              (400.0f)
#define TASK5_BASE_SPEED_MM_S              (350.0f)
#define TASK6_BASE_SPEED_MM_S              (350.0f)

#endif /* CAR_CONFIG_H */
