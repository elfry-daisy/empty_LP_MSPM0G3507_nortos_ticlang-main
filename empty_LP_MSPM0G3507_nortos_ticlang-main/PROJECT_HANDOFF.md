# MSPM0G3507 电赛循迹车工程交接说明

## 1. 文档用途

本文档用于把当前工程完整交接给下一位开发者，回答以下问题：

- 当前工程为什么这样分层；
- 已经实现了哪些功能；
- 哪些代码只是接口或安全占位；
- 系统从传感器到电机的完整数据流是什么；
- 当前工程为什么能编译、但还不能直接驱动实车；
- 下一步应当先配置什么、怎样逐项验收；
- 哪些参数必须根据实物修改；
- 当前版本有哪些已知限制和风险。

硬件接线、SysConfig 外设创建和 BSP 填写步骤请同时阅读
`CAR_PORTING.md`。

## 2. 工程基本信息

- 目标 MCU：TI MSPM0G3507，Cortex-M0+；
- 开发板：LP_MSPM0G3507；
- SDK：MSPM0 SDK 2.11.00.07；
- 编译器：TI Arm Clang 5.1.1.LTS；
- 工程类型：NoRTOS；
- 电机驱动：TB6612FNG；
- 车辆结构：左右双直流电机差速车；
- 当前循迹传感器：八路独立模拟灰度阵列，使用 ADC0 三路和 ADC1 五路；
- 外设：AB 相编码器、500 Hz 串口 IMU、PA12/PA13 软件 I2C SSD1306 OLED、调试 UART。

工程路径：

```text
C:\ti\workspace_ccstheia\empty_LP_MSPM0G3507_nortos_ticlang
```

SDK 参考路径：

```text
C:\ti\mspm0_sdk_2_11_00_07
```

## 3. 本次开发做了什么

原工程最初只有 `empty.c` 和只启用系统时钟的 `empty.syscfg`。本次工作在不修改
SDK、不手改 SysConfig 生成文件的前提下，建立了完整软件骨架；随后与用户逐项通过
SysConfig GUI 确认并正式配置 ADC、UART、1 ms Timer 和 OLED GPIO。

文件变更规模：新建约 50 个文件并修改原入口 `empty.c`；其中包含 24 个 C 源文件。
`empty.syscfg` 现已由用户在 GUI 中正式加入 ADC0/1、UART0/1、TIMG0 和 OLED GPIO。

主要工作如下：

1. 建立配置层、BSP 层、器件驱动层、控制层和应用层；
2. 为 TB6612 实现双通道正反转、滑行、短刹车和 STBY 控制逻辑；
3. 实现电机有符号占空比、极性、限幅、死区、斜坡和紧急停止；
4. 实现编码器增量、速度、RPM、累计距离和低通滤波；
5. 实现八路真实 ADC 的逐通道标定、归一化和加权位置解算；使用 ADC0/ADC1
   双序列完成中断组成完整帧，包含超时重启与诊断计数；
6. 实现通用 PID，包含输出限幅、积分限幅、抗饱和和微分滤波；
7. 实现循迹方向环、弯道降速、差速混合和左右轮速度环；
8. 实现 NoRTOS 毫秒任务调度和整车状态机；
9. 实现 UART 调试协议骨架和整数缩放 CSV 遥测；
10. 移植 500 Hz 串口 IMU 协议，融合 Modbus CRC16、非阻塞 UART 环形缓冲、
    角度回绕、超时、零偏标定和诊断计数；
11. 把江协科技 OLED V2.0 软件 I2C 驱动、字库和绘图函数完整移植到 MSPM0G3507；
12. 修改 `empty.c`，把全部模块接入持续运行的主循环；
13. 使用工程实际 TI Arm Clang 编译全部源码并完成完整链接验证；
14. 检查工程中不存在 MCU 睡眠、WFI 或低功耗策略调用。
15. 核对并接入用户已完成的 HC-05 SysConfig 配置：UART1、PB4 TX、PB5 RX、
    115200 8N1、FIFO、RX `>= 1 entry` 中断。
16. 因硬件改为八路独立模拟灰度，移除原数字复用方案，重写双 ADC 非阻塞 BSP，
    并通过完整候选 SysConfig 验证 ADC 3+5 分组、备用 I2C1 和备用 UART2 可共存。
17. 核对并接入 OLED SysConfig：`OLED_GPIO`、PA12 SCL、PA13 SDA、Initial Set、
    Hi-Z Enable；增加 100 ms 低优先级逐页诊断刷新。

## 4. 当前完成度总览

| 模块 | 状态 | 是否可直接上硬件 | 说明 |
|---|---|---:|---|
| 工程分层和配置文件 | 已完成 | 是 | 参数集中管理 |
| TB6612 上层逻辑 | 已完成 | 等待台架验证 | A/B 通道、方向、刹车、滑行，真实 GPIO/PWM BSP 已接入 |
| 电机控制 | 已完成 | 等待台架验证 | 极性、限幅、斜坡、死区、安全停机 |
| 编码器数学处理 | 已完成 | 等待台架验证 | 左硬件 QEI、右软件 AB、速度、RPM、距离、滤波 |
| 循迹算法 | 已完成首版 | 等待上板验证 | 真实模拟量、标定、质心、丢线 |
| 通用 PID | 已完成 | 是 | 仍需实车调参 |
| 左右速度闭环 | 已完成首版 | 编码器接入后可用 | 默认参数只是起点 |
| 循迹方向闭环 | 已完成首版 | 传感器接入后可用 | 默认使用 PD |
| 差速混合 | 已完成 | 是 | 超限时按比例缩放 |
| 任务调度器 | 软件和 1 ms Timer 已完成 | 等待上板校时 | TIMG0、1 ms ZERO ISR；无休眠、无阻塞延时 |
| 整车状态机 | 已完成基础状态 | 是 | 尚无环岛等比赛元素 |
| HC-05 调试 UART 应用协议 | 已完成 | 等待上板验证 | R/S/X/C/E/G/I 和 CSV |
| HC-05 调试 UART 底层 | 软件、SysConfig、构建已通过 | 等待上板验证 | UART1、PB4/PB5、FIFO/ISR |
| IMU UART 底层 | 软件、SysConfig、构建已通过 | 等待上板验证 | UART0、PA10/PA11、RX/TX 环形缓冲 |
| PWM/GPIO 底层 | 已实现 | 等待台架验证 | TIMG12 PB13/PA31、20 kHz；五根控制线已映射 |
| 编码器底层 | 已实现 | 等待台架验证 | TIMG8 PB6/PB7 + GPIOB PB8/PB9、GROUP1 ISR |
| 八路循迹 ADC BSP | 软件、SysConfig 已完成 | 等待上板验证 | ADC0 3路 + ADC1 5路、非阻塞完整帧 |
| 500 Hz 串口 IMU | 协议完成 | UART 配置后可用 | 9 字节帧、CRC、回绕、超时、标定 |
| SSD1306 OLED 驱动 | 软件、GPIO和实物显示已通过 | 80 MHz 后待复测 | PA12/PA13、0x3C、ADC与电机/编码器双诊断画面 |
| 按键、电池、Flash | 未实现 | 否 | 后续增强项 |
| 十字/环岛/停车线 | 未实现 | 否 | 依赛题规则开发 |

“已完成首版”表示软件结构和基本算法已经存在，但必须经过实车数据验证和参数
调整，不能理解为比赛参数已经调好。

### 4.1 进度估算

以下百分比用于交接排期，不代表比赛完成度可以简单相加：

| 工作包 | 当前估算 | 判定依据 |
|---|---:|---|
| 软件架构和模块接口 | 100% | 分层、接口和主循环已建立并链接通过 |
| 与引脚无关的基础算法 | 90% | 首版已实现，仍缺实车数据验证 |
| MSPM0 外设/SysConfig 接入 | 95% | IMU、HC-05、ADC、TIMG0、OLED、PWM、QEI/右编码器 GPIO 均已配置；剩余实物验证 |
| 电机和编码器台架验证 | 0% | 尚未连接实物 |
| 基础速度闭环 | 30% | 软件存在，硬件未接入、参数未整定 |
| 基础连续线循迹 | 60% | 双 ADC 已正式启用，解算和控制存在；缺传感器接线、标定与实车验证 |
| 比赛赛道元素 | 0% | 尚未取得具体赛题规则 |
| IMU | 80% | 协议/UART BSP/诊断完成，缺实际引脚和上板数据验证 |
| OLED | 90% | 驱动/字库/PA12/PA13/诊断页完成，缺实物 ACK、波形和显示验证 |
| 安全与诊断增强 | 20% | 有丢线/急停，缺欠压、堵转、超时等 |

当前里程碑可定义为：**“软件框架 V0.1 完成，等待接线表进入硬件移植阶段”**。

## 5. 软件分层与依赖方向

```text
empty.c
  └─ app/                  整车状态和任务调度
      ├─ control/          PID、方向环、速度环、差速混合
      │   └─ drivers/      TB6612、电机、编码器、循迹、IMU、OLED
      │       └─ bsp/      MSPM0 外设边界，唯一允许接触具体引脚的位置
      └─ config/           车辆参数和接线记录
```

依赖只能从上往下：

- 应用层可以调用控制层和驱动层；
- 控制层不能直接访问 MSPM0 寄存器；
- 驱动层不能直接依赖业务状态机；
- 只有 BSP 应该使用 `ti_msp_dl_config.h` 和 DriverLib；
- `Debug/ti_msp_dl_config.*` 是生成文件，禁止手改。

这样做的目的，是让更换引脚、定时器、ADC 通道甚至传感器型号时，不需要重写
PID、循迹和整车状态机。

## 6. 运行时完整数据流

### 6.1 循迹方向链路

```text
ADC0 MEM0~2 + ADC1 MEM0~4 软件触发
  → 两个末尾 MEM 完成中断分别置位
  → 两组均完成后提交八路真实 12 位结果
  → 每通道黑白标定
  → 0~1000 归一化强度
  → 加权质心位置 position(-1~+1)
  → 循迹 PD/PID
  → steeringMmS
```

位置符号约定：

- `position < 0`：赛道线偏车辆左侧；
- `position = 0`：线位于阵列中心；
- `position > 0`：赛道线偏车辆右侧。

正 `steeringMmS` 表示右转，差速混合会提高左轮目标速度并降低右轮目标速度。

当前模块输出连续模拟电压，`raw[0..7]` 是真实 0~4095 ADC 结果。默认 ADC0 负责
数组 0~2，ADC1 负责数组 3~7；两个序列都完成后才提交，避免跨 ADC 半新半旧。
每次提交后立即重启下一帧，1 ms 任务下目标帧率接近 1 kHz。任一 ADC 连续五次
任务轮询仍未完成时，BSP 会重新武装两个序列并增加重启诊断计数，不会阻塞主循环。

### 6.2 速度闭环链路

```text
forwardMmS + steeringMmS
  → 左右目标轮速
  → 与左右编码器实测速度比较
  → 左右独立 PI/PID
  → -1000~+1000 电机指令
  → 电机极性/限幅/斜坡/死区
  → TB6612 A/B 通道
```

### 6.3 IMU 链路

```text
115200 8N1 / 500 Hz / 9 字节帧
  → UART RX ISR 排空硬件 FIFO
  → 64 字节软件环形缓冲
  → 1 ms 任务帧头同步
  → Modbus CRC16
  → 0.1° 和 0.1°/s 缩放
  → 16 位角度回绕展开
  → 零偏、超时和诊断
  → IMU_GetData()
```

IMU 是独立 UART，不能与调试 CSV 串口共用。CRC 错误、协议头错误、软件缓冲
溢出和主任务漏取帧都有独立计数，方便区分接线问题和调度负载问题。

### 6.4 丢线处理

短时间丢线时：

- 循迹模块保留最后一次有效位置；
- 方向控制器降低前进速度；
- 按最后一次偏移方向继续搜线。

超过 `CAR_LINE_LOST_STOP_MS` 后：

- 状态转为 `CAR_STATE_LINE_LOST`；
- 禁止 TB6612 输出；
- 复位速度环；
- 需要重新发送启动命令才能运行。

## 7. 调度和实时性

调度器以 `BSP_Time_GetMs()` 为时间基准：

| 周期 | 任务 | 设计原因 |
|---:|---|---|
| 1 ms | IMU UART 解析、领取循迹 ADC 完整帧 | 接住 500 Hz IMU；灰度目标约 1 kHz 帧率 |
| 5 ms | 编码器、方向环、速度环、电机 | 200 Hz 控制周期 |
| 10 ms | 状态和安全检查 | 状态逻辑无需进入高速 ISR |
| 20 ms | UART 遥测 | 避免调试输出占用过多带宽 |
| 100 ms | OLED 一页诊断数据 | 每次最多 120 字节；八页约 0.8 秒轮换一次 |

当前 `TICK_TIMER/TIMG0` 已按 1 ms 周期配置。定时器 ISR 只做：

1. 判断并清除 1 ms 定时器中断标志；
2. 调用 `BSP_Time_Tick1msFromISR()`；
3. 立即退出。

IMU UART ISR 只排空 RX FIFO到环形缓冲、填充 TX FIFO并退出；CRC、浮点缩放、
偏航角和标定全部在 1 ms 主循环任务中处理。

禁止在 ISR 中执行：

- OLED 全屏刷新；
- `printf`/`snprintf`；
- 阻塞 I2C；
- 长循环；
- PID 调参协议解析；
- 任何延时。

主循环始终调用 `App_Scheduler_Run()`，不存在 `WFI`、Sleep 或 Standby。

## 8. 状态机说明

| 状态 | 进入条件 | 电机行为 | 离开方式 |
|---|---|---|---|
| IDLE | 上电初始化完成 | STBY 禁止 | R/C/X |
| CALIBRATING | 收到 C | 电机禁止，持续采样极值 | E/X |
| RUNNING | 收到 R | 正常闭环控制 | S/X/长时间丢线 |
| LINE_LOST | 丢线超时 | 电机禁止 | R/S/X |
| STOPPED | 收到 S | 电机禁止 | R/C/X |
| FAULT | 收到 X | 紧急停机 | 当前未开放运行时恢复，需复位或后续新增明确恢复接口 |

上电不会自动启动，避免下载程序后车辆突然运动。

## 9. 参数和单位约定

### 9.1 电机指令

- 类型：`int16_t`；
- 范围：`-1000~+1000`；
- 单位：千分比 PWM；
- 正值：逻辑前进；
- 负值：逻辑后退；
- 实际物理方向由左右极性宏修正。

### 9.2 速度和距离

- 轮速：mm/s；
- 累计距离：mm；
- 电机转速显示：RPM；
- 控制周期：秒，例如 `0.005f`。

### 9.3 循迹位置

- 归一化强度：`0~1000`；
- 位置：约 `-1.0~+1.0`；
- 置信度：`0.0~1.0`。

### 9.4 方向控制输出

方向 PID 输出不是直接 PWM，而是左右轮之间的速度差，单位为 mm/s。这样方向环
和速度环可以分别调节，也更容易适应电池电压变化。

## 10. 必须替换的占位参数

以下参数目前不能视为实车定值：

```c
CAR_WHEEL_DIAMETER_MM
CAR_ENCODER_COUNTS_PER_WHEEL_REV
CAR_MOTOR_LEFT_POLARITY
CAR_MOTOR_RIGHT_POLARITY
CAR_ENCODER_LEFT_POLARITY
CAR_ENCODER_RIGHT_POLARITY
CAR_LINE_ADC_REVERSE_ORDER
CAR_LINE_BLACK_IS_LOW_RAW
CAR_LINE_DETECT_SUM_MIN
CAR_LINE_ELEMENT_THRESHOLD
所有 PID 参数
```

根据 MG513 铭牌参数，`CAR_ENCODER_COUNTS_PER_WHEEL_REV` 当前按
`13 PPR × 30 减速比 × AB 四倍频 = 1560` 推导。该值已经比占位值更合理，但厂家
对 PPR 的定义可能不同，仍必须用 QEI 手转车轮一圈实测确认。

## 11. UART 调试约定

控制命令：

| 字符 | 功能 |
|---|---|
| R/r | 开始运行 |
| S/s | 正常停止 |
| X/x | 紧急停止并进入 FAULT |
| C/c | 开始传感器标定 |
| E/e | 结束传感器标定 |
| G/g | 当前 IMU 相对偏航角清零 |
| I/i | 开始 IMU 静止角速度零偏标定 |

CSV 字段：

```text
time_ms,state,line_position_x1000,left_speed_mm_s,right_speed_mm_s,
left_pwm_permille,right_pwm_permille,line_detected,imu_valid,
yaw_degrees_x10,gyro_z_dps_x10,imu_crc_errors
```

这里指的是通过 HC-05 给手机输出 CSV 的调试 UART。`bsp_uart.c` 已实现 512 字节
TX、256 字节 RX 环形缓冲、FIFO 中断搬运和错误统计。用户已在 `empty.syscfg` 配置
`HC05_UART = UART1`、PB4 TX、PB5 RX、115200 8N1、FIFO 和 RX `>= 1 entry`
中断，因此 `CAR_HC05_UART_READY=1`。IMU 独占 UART0，已经在 `bsp_imu_uart.c`
实现独立环形缓冲和中断服务。两路串口不能共用同一个外设实例；调试数据队列满时
允许丢帧，控制任务绝不能等待发送完成。

PB4 在板卡资料中同时是 PWM 候选，后续给 TB6612 分配 PWM 时必须避开它；PB5 位于
下方未焊接接口区域，需要确认实物引出方式。当前分配已经避开 PA18 的默认 BSL 风险。

## 12. 已完成的验证

完成日期：2026-07-23。

验证内容：

- 24 个 C 源文件使用 TI Arm Clang 编译成功且 `-Wall -Wextra` 零告警；
- 打开 `-Wall -Wextra`；
- 使用 MSPM0G3507 的设备宏；
- 与现有 SysConfig 生成对象、启动文件、DriverLib 和 libc 完整链接；
- 主应用已接入 IMU，但尚未调用 OLED；验证镜像约 11504 字节代码、967 字节 BSS；
- 强制打开 `CAR_IMU_UART_READY=1` 并用 MSPM0G3507 UART0/IRQ 宏编译了真实
  DriverLib 分支，确认环形缓冲、中断入口和 FIFO API 均可编译、完整链接；启用
  硬件分支后的验证镜像约 12056 字节代码、1063 字节 BSS；
- 协议向量验证得到配置命令 CRC=`0x00AD`，线上顺序=`AD 00`；正反方向
  `int16_t` 角度回绕增量分别验证为 `+1/-1`；
- 额外使用 OLED 初始化、字库、浮点显示、圆弧、Printf 和整屏刷新做了强制链接
  探针，约 22144 字节代码、1541 字节 BSS，证明 OLED 的数学库和格式化依赖完整；
- HC-05 UART 的 `READY=0` 安全分支零告警编译通过；临时映射 UART1 后，真实
  RX/TX FIFO、中断环形缓冲和 `UART1_IRQHandler` 分支也完整编译链接，且不与
  IMU 的 `UART0_IRQHandler` 冲突；模拟启用后的镜像约 13072 字节代码、
  1851 字节 BSS；
- 在用户加入 HC-05 SysConfig 之前，曾对 `IMU READY=1 / HC-05 READY=0` 配置执行
  原工程增量构建，
  `bsp_uart.c`、`bsp_imu_uart.c`、`app_debug.c` 和整车最终链接均成功；
- 八路模拟 ADC 驱动分别在 `CAR_LINE_ADC_READY=0` 安全分支、临时 ADC0/ADC1 宏
  映射后的 `READY=1` 分支，以 TI Arm Clang、`-Wall -Wextra -Werror` 编译通过；
- 建立独立验证 SysConfig，确认推荐八路 ADC、现有 UART0/UART1、SWD、预留
  I2C1/PB2/PB3 和 UART2/PB15/PB16 可以同时生成且无 PinMux 冲突；
- 使用该验证配置实际生成的 `LINE_ADC0/1` 宏再次编译真实 DriverLib 分支成功；
- ADC ISR 只置完成标志，主任务在两组均完成后提交；实现中没有阻塞等待或延时；
- 用户已在原工程正式保存 `LINE_ADC0/ADC0`、`LINE_ADC1/ADC1`、`IMU_UART/UART0`
  和 `HC05_UART/UART1`；`CAR_LINE_ADC_READY=1` 后执行 Clean 全量构建并链接成功；
- 最终 map 明确包含来自 `bsp_line_adc.o` 的 `ADC0_IRQHandler`、`ADC1_IRQHandler`，
  以及互不冲突的 `UART0_IRQHandler`、`UART1_IRQHandler`；
- 用户已正式保存 `TICK_TIMER/TIMG0`：BUSCLK/1、Load=31999、Periodic、1 ms、
  ZERO 中断、SysConfig 不自动启动；`BSP_Time_Init()` 负责清状态、开 NVIC 和启动；
- TIMG0 接入后再次执行 Clean 全量构建成功，最终 map 中 `TIMG0_IRQHandler` 明确
  来自 `bsp_time.o`，并与 ADC0/ADC1/UART0/UART1 四个已有 ISR 同时存在；
- 2026-07-24 用户正式保存 `OLED_GPIO`，磁盘 `.syscfg` 明确为 PA12/PA13、Initial
  Set、Hi-Z Enable；使用正式完整配置重新运行 SysConfig 1.28.0 验证和生成成功；
- 生成结果明确包含 `OLED_GPIO_PORT`、`OLED_GPIO_SCL_PIN`、`OLED_GPIO_SDA_PIN`，且
  两脚均为 `DL_GPIO_HIZ_ENABLE`、无内部电阻、低驱动强度；
- 启用 `CAR_OLED_SOFT_I2C_READY=1` 后，`app_debug.c`、`app_scheduler.c`、
  `bsp_oled_soft_i2c.c`、`oled.c` 已使用正式生成头和器件选项完成 TI Arm Clang
  单文件编译检查；随后 9 个代码/文档文件已通过 SHA-256 核对同步到原工程；
- OLED 接入后执行 `gmake clean` 与 `gmake -j4 all`，SysConfig 重新生成、24 个应用
  C 源文件编译和最终链接全部成功；最终 ELF 为 322352 字节，map 报告 Flash 实际
  使用 `0x61D0`（25040 字节）、SRAM 区域使用 `0x0B55`（2901 字节）；
- 最终 map 同时包含 `ADC0_IRQHandler`、`ADC1_IRQHandler`、`TIMG0_IRQHandler`、
  `UART0_IRQHandler`、`UART1_IRQHandler`，以及 `App_Debug_OLEDTask`、`OLED_Init`、
  `OLED_UpdateArea` 和四个软件 I2C BSP 函数；
- 真实原工程构建已触发 SysConfig 重新生成并完整链接成功；生成结果明确为
  `HC05_UART_INST=UART1`、`UART1_IRQHandler`、RX=GPIOB/PB5、TX=GPIOB/PB4，
  115200 baud、FIFO 和 RX one-entry threshold；
- 链接映射中 `UART0_IRQHandler`（IMU）与 `UART1_IRQHandler`（HC-05）同时存在，
  地址不同且无重复定义；
- 没有未解析符号和重复定义；
- 没有 MCU 休眠或低功耗入口。

这项验证只能证明软件结构和链接关系正确，不能代替示波器、电机台架和赛道实测。

## 13. 还差什么

### 第一优先级：让基础硬件产生可信数据

1. 在 `empty.syscfg` 继续创建双 PWM、方向 GPIO和双 QEI；`TICK_TIMER/TIMG0`、
   `LINE_ADC0/1`、IMU UART0 和 HC-05 UART1 已正式分配；
2. ADC 按 PA25、PA24、PA22、PA15、PA17、PB17、PB18、PB19 从最左到最右配置；
3. 保留 I2C1/PB2/PB3 和 UART2/PB15/PB16，不再把它们分配给普通 GPIO；
4. 填写所有 BSP TODO；
5. 按传感器最左到最右确认八个 MEM 顺序、黑白电压范围，并断开 PA22 的 J16 负载；
6. 测量编码器每车轮一圈实际计数；
7. 验证左右电机和编码器极性；
8. 用串口/Watch 确认时间、灰度完整帧计数和编码器数据连续更新。

### 第二优先级：完成基础闭环

1. 架空车轮测试每个电机的正反转；
2. 单独调左轮速度 PI；
3. 单独调右轮速度 PI；
4. 让车辆在无循迹控制时稳定直行；
5. 完成黑白标定并观察归一化值；
6. 低速调循迹 PD；
7. 再逐步增加基础速度和弯道减速强度。

### 第三优先级：比赛功能

1. 根据赛题定义十字、直角、环岛、坡道和停车线；
2. 为每种元素增加明确状态和超时保护；
3. 加入电池电压、堵转和传感器超时保护；
4. 上板验证 IMU 方向、CRC、零偏和角度回绕后，再用于定角转向；
5. 最后加入 OLED 菜单和 Flash 参数保存。

## 14. 当前已知限制

- MG513 标称堵转电流约 2.8 A，明显高于 TB6612 常用约 1.2 A/通道连续能力；
  虽然峰值规格可能短时覆盖，但堵转会快速发热，必须避免长时间堵转并考虑硬件保护；
- PWM/GPIO/编码器 BSP 已接入真实资源，但尚未完成电机和编码器台架验证；上电默认
  仍保持 STBY 低、方向低、PWM 0%，不会自动运行；
- 八路 ADC 左到右 0～7 和 OLED 显示已经实物确认；黑白极性、动态标定和电机干扰
  工况仍需验证；
- 默认 PID 未经实车调节；
- 编码器速度滤波系数固定在源码中；
- 循迹路口判断只是基于活跃通道数量的初步启发式；
- 丢线恢复只使用最后方向，没有结合 IMU；
- FAULT 清除策略较简单，后续可要求长按按键或明确复位命令；
- HC-05 已分配 UART1/PB4/PB5且完整构建通过，但尚未连接实物验证透传收发；
- PB4 已被 HC-05 TX 占用，后续 TB6612 PWM 不能再选 PB4；PB5 的物理引出需确认；
- 八路模拟输出尚未上板测量；每路都必须限制在 0~3.3 V，不能把 5 V 模拟量直接
  输入 MSPM0 ADC；
- 推荐 ADC 方案只有 PA22 涉及板载光线传感器，使用前必须确认/断开 J16；
- 标定结果只保存在 RAM，上电后丢失；
- OLED 已按 128×64 SSD1306/0x3C 移植，并在 PA12/PA13 上实物显示正常；CPU 改为
  80 MHz 后位延时已自动换算，仍需复测显示稳定性和 SCL 波形；
- OLED 使用同步软件 I2C，完整 1 KiB 刷新不能放进实时控制路径；当前每 100 ms
  只更新一页，正式竞速前仍要测量单页耗时并评估是否进一步降频或关闭显示；
- 串口 IMU 已配置 UART0、PA10 TX、PA11 RX，但尚未上板确认 J21/J22 路由、CRC、
  零偏和物理方向；
- 没有单元测试框架，验证主要为编译、链接和后续硬件测试。

## 15. 下一位开发者的推荐入口

首次接手时建议按以下顺序阅读：

1. `PROJECT_HANDOFF.md`：了解整体状态和工程方法；
2. `PIN_ALLOCATION_RECOMMENDATION.md`：按已验证方案配置八路 ADC，并保留 I2C/UART；
3. `BOARD_PINOUT.md`：查 LaunchPad 引脚、跳线、冲突和当前占用；
4. `CAR_PORTING.md`：完成硬件资源；
5. `config/car_config.h`：核对车辆参数；
6. `config/board_config.h`：记录接线；
7. `bsp/bsp_*.c`：完成实际 DriverLib 调用；
8. `app/app_car.c`：理解整车数据流；
9. `drivers/imu.c` 和 `bsp/bsp_imu_uart.c`：确认 IMU 协议和中断入口；
10. `control/line_control.c` 和 `control/speed_control.c`：开始调参；
11. `drivers/line_sensor.c`：验证传感器方向和位置符号。

## 16. 交接验收标准

只有满足以下条件，才应把“基础循迹功能”标记为完成：

- 上电电机不误转；
- 1 ms 时间连续且无明显抖动；
- 两轮逻辑正速度都对应车辆前进；
- 两个编码器逻辑正计数都对应车辆前进；
- 车轮转一圈的计数误差在可接受范围；
- 每路循迹传感器黑白跨度足够且无明显坏道；
- `position` 左负右正与控制方向一致；
- 左右速度闭环分别稳定；
- 低速循迹不持续振荡；
- 丢线超时能够可靠停机；
- 调试 UART、IMU UART 或 OLED 故障不会造成永久等待；OLED 单页同步传输必须实测耗时；
- 运行代码中仍没有阻塞延时和低功耗入口。

## 17. 工程做事方法与后续协作规范

这一节记录本项目采用的工作方法。目的不是规定个人写法，而是让下一位开发者在
缺少上下文、任务中断或更换硬件时，仍能沿着同一套证据链继续推进，不把“能编译”
误认为“已经能比赛”。

### 17.1 先区分事实、推导、选择和待验证项

开始实现前，先把信息分成四类：

| 类型 | 例子 | 处理方式 |
|---|---|---|
| 已知事实 | MG513 13 PPR、减速比 30、IMU 500 Hz | 原样记录来源，不擅自改写 |
| 软件推导 | `13×30×4=1560 count/rev` | 写出公式和前提，保留实测复核项 |
| 设计选择 | TX 队列 512 B、控制周期 5 ms | 说明为什么这样选以及改变后的影响 |
| 硬件未知 | TB6612 引脚、ADC 顺序、HC-05 第二 UART | 使用宏和 `READY` 开关，不猜引脚 |

只要一个结论仍依赖实物，就明确写成“待上板验证”，不使用“已经完成”掩盖不确定性。

### 17.2 先读现状，再做最小范围修改

每次接手应先检查：

1. 当前源文件、`empty.syscfg` 和自动生成宏；
2. CCS 实际纳入构建的源文件；
3. 已经启用的外设、中断入口和引脚；
4. 工作区里是否存在用户未提交或未交接的修改；
5. 交接文档是否有落后于代码的描述。

修改只覆盖本任务需要的范围。SysConfig 负责 PinMux、时钟和静态外设参数；BSP
负责 DriverLib 和中断；驱动负责设备协议；控制层不直接访问寄存器。不要为了修一处
UART 顺手重排整个工程，也不要覆盖与本任务无关的用户修改。

### 17.3 分层方向必须单向

本工程坚持以下依赖方向：

```text
app -> control/drivers -> bsp -> SysConfig/DriverLib -> hardware
```

- `app/` 只组织状态和任务；
- `control/` 只处理控制数学和单位；
- `drivers/` 处理器件语义、协议、诊断和数据有效性；
- `bsp/` 隔离 MSPM0 外设和中断；
- `config/` 集中记录参数、极性、资源别名和 `READY` 开关；
- 自动生成文件不手改。

这样换引脚通常只改 SysConfig 或 `board_config.h` 别名，换传感器只改驱动，不需要
重写 PID 和整车状态机。

### 17.4 先定义实时性契约，再写函数

电赛控制代码中，每个函数都要先回答：

- 能否在 ISR 调用；
- 是否会等待硬件；
- 最坏执行时间是否有界；
- 队列满、设备掉线或数据错误时怎样退化；
- 错误会不会阻止电机控制继续运行。

本工程的默认原则是：

- ISR 只搬运数据、确认中断、更新时间戳；
- CRC、浮点、格式化和状态机留在主循环；
- 通信发送只入队，空间不足返回 `false`；
- 遥测可以丢帧，控制数据不能被遥测阻塞；
- 不使用 `WFI`、Sleep、Standby 或 MCU 低功耗等待；
- 不在 1 ms/5 ms 实时任务里整屏刷新 OLED；
- 不使用无超时的等待循环。

### 17.5 用安全占位分支隔离未配置硬件

硬件资源未确定时保留：

```c
#define CAR_xxx_READY (0U)
```

`READY=0` 分支必须仍能编译、链接并安全返回；只有 SysConfig 已生成真实实例、引脚
无冲突且宏名称核对完成后，才改为 `1U`。这能保证软件模块可以提前完成，同时避免
下载后误转电机、误开中断或引用尚不存在的自动生成符号。

### 17.6 注释解释“为什么”，接口表达“怎么用”

代码注释重点写：

- 电气或协议限制；
- 并发关系和生产者/消费者；
- 为什么选择某个缓冲长度、阈值或超时；
- 失败策略和安全后果；
- 哪些参数必须实测。

避免逐行翻译语法。接口命名应表达行为，例如 `TryWrite` 明确表示可能因队列满而
失败；单位写进变量或宏名，例如 `_MS`、`_HZ`、`MmS`、`Permille`，避免裸数字和
隐式单位换算。

### 17.7 验证分层，结论也分级

完成度按以下证据等级报告：

1. **静态检查通过**：接口、数组边界、单位和中断命名已检查；
2. **编译通过**：目标编译器和真实设备宏下零错误；
3. **链接通过**：整车镜像无未解析符号和重复中断；
4. **SysConfig 生成通过**：真实实例、PinMux、IRQ 和 Makefile 已生成；
5. **台架验证通过**：示波器、逻辑分析仪或架空车轮测得正确；
6. **实车验证通过**：负载、干扰、赛道和异常工况下满足要求。

必要时分别验证 `READY=0` 和强制 `READY=1` 两个分支。强制映射实例的编译链接
只能证明 API 和符号关系正确，不能代替真实 PinMux 和接线验证。

### 17.8 每次交接必须回答五个问题

1. 这次具体修改了哪些文件和接口；
2. 哪些内容已经完成，完成到哪一级证据；
3. 哪些内容还没做，为什么没做；
4. 下一步应该按什么顺序操作；
5. 出问题时应观察哪些计数、波形或状态量。

文档中的“当前状态”必须随代码一起更新。任务被网络或工具中断后，先重新读取实际
文件、生成宏和最近构建结果，再从断点继续；不要凭聊天记忆重做已经完成的修改。

### 17.9 引脚分配的固定流程

1. 先看 `BOARD_PINOUT.md` 中的板级占用、跳线和接口位置；
2. 在一张资源表里保留 SWD、已用 UART、供电和特殊启动脚；
3. 给必须使用特定复用的外设先选引脚，例如 QEI、PWM、UART；
4. 普通 GPIO 最后分配；
5. 让 SysConfig 验证同一外设实例和 PinMux 是否成立；
6. 保存后核对生成的 `*_INST`、`*_PIN`、`*_IRQN` 和 `*_IRQHandler`；
7. Clean + Build；
8. 最后按信号逐项上板测量。

板卡图片上的“UART/PWM/ADC/SPI”等文字是接口常用角色，不等同于芯片完整复用表；
Excel 中的“可以使用”也只表示板级条件允许。最终能否组合为同一个外设实例，必须
以当前封装下 SysConfig 的无冲突结果为准。

## 18. 2026-07-25 最终电机、编码器和 80 MHz 接入记录

### 18.1 本轮已经完成的代码

- `config/board_config.h`：正式启用 `CAR_TB6612_READY=1`、
  `CAR_ENCODER_READY=1`，把 TIMG12、五根 TB6612 GPIO、TIMG8 QEI 和右编码器
  GPIOB 生成宏集中映射为 `CAR_*` 别名；
- `bsp/bsp_tb6612.c`：实现安全初始化、STBY、两组方向脚和 0～1000 千分比 PWM；
- `bsp/bsp_encoder.c/.h`：实现左轮 16 位 QEI 软件扩展、右轮 PB8/PB9 双边沿 AB
  查表、共享 `GROUP1_IRQHandler()` 分派、原子复位和诊断快照；
- `app/app_debug.c`：OLED 增加电机/编码器诊断画面，显示累计/周期计数、轮速、
  速度目标、电机目标/实际占空比、使能、右轮 IRQ 和非法跳变；
- `bsp/bsp_board.c`：移除已经过期的重复启动 TODO，明确每个 BSP 独立负责启动；
- OLED 软件 I2C 延时改为 `CPUCLK_FREQ/800000`，80 MHz 下自动得到 100 cycles。

### 18.2 已核对的正式资源

| 功能 | 最终配置 |
|---|---|
| 主时钟 | MCLK/CPUCLK 80 MHz，ULPCLK 40 MHz |
| 左 PWM | TIMG12 CCP0，PB13，20 kHz |
| 右 PWM | TIMG12 CCP1，PA31，20 kHz |
| TB6612 方向/STBY | PB0、PB1、PB12、PB20、PA28，均初始低 |
| 左编码器 | TIMG8 QEI，PB6/PB7，LOAD=65535 |
| 右编码器 | GPIOB PB8/PB9，双边沿，Group 1 共享中断 |
| 理论计数 | `13 PPR × 30 × 4 = 1560 count/输出轴一圈` |

TIMG12 实际使用 80 MHz，不是 ULPCLK 40 MHz，所以 Period=4000 才是 20 kHz。
TIMG0 和两个 UART 使用 40 MHz，SysConfig 已分别重算为 1 ms 和约 115190.78 baud。

### 18.3 证据等级与仍未完成内容

当前 SysConfig CLI 已生成并确认真实宏、PinMux、80 MHz 时钟树和 GROUP1 IIDX。
2026-07-25 已把本轮 10 个预期文件同步至正式工程，并在正式 `Debug` 目录执行：

```text
gmake clean
gmake -j4 all
```

构建退出码为 0，没有编译器或链接器 Warning/Error，成功生成新的 `.out`、`.hex` 和
`.map`。Clean 后的正式 `ti_msp_dl_config.h` 已确认：

```text
CPUCLK_FREQ                     = 80000000
MOTOR_PWM_INST_CLK_FREQ         = 80000000
TICK_TIMER_INST_LOAD_VALUE      = 39999
IMU_UART_INST_FREQUENCY         = 40000000
HC05_UART_INST_FREQUENCY        = 40000000
```

map 中已确认 `GROUP1_IRQHandler`、`ADC0_IRQHandler`、`ADC1_IRQHandler`、
`TIMG0_IRQHandler`、`UART0_IRQHandler`、`UART1_IRQHandler`，以及全部 TB6612/
Encoder BSP 函数均由真实对象文件提供。源码调用扫描未发现 `__WFI`、Sleep、STOP、
STANDBY 或 SHUTDOWN 入口。本轮证据等级已达到“SysConfig 生成、目标编译、链接通过”；
尚不能替代下面的台架和实车验证。

上板仍待完成：

1. 不接电机主电源，测 STBY=低、四根方向线=低、两路 PWM 初始 0%；
2. 明确启动后测 PB13/PA31 均为 20 kHz，并核对占空比；
3. 确认编码器 A/B 高电平不超过 3.3 V；
4. 手转左轮和右轮，检查 OLED 计数连续、方向正确、ERR 不增长；
5. 每只输出轴完整转一圈，确认是否约 1560 count；
6. 车轮悬空后再接电机主电源，以低占空比逐轮开环测试；
7. 最后才进入速度 PID 和循迹联调。

任何实物方向相反，优先修改 `CAR_MOTOR_LEFT/RIGHT_POLARITY` 或
`CAR_ENCODER_LEFT/RIGHT_POLARITY`，不要在控制算法各处散落负号。

## 19. 主逻辑功能启用方式

现有程序并不是只显示 OLED：`App_Car_Init()` 与 `App_Scheduler_Init()` 已经默认启用
所有不造成上电误跑的后台链路，包括 IMU、八路 ADC、循迹位置解算、编码器、HC-05、
OLED 和状态保护。编码器在 IDLE 也持续更新，循迹 ADC 在 IDLE 也持续采样；只有
`LineControl → VehicleMixer → SpeedControl → Motor` 的闭环输出要求状态为 RUNNING。

正常使用时通过 HC-05 单字符命令控制：

```text
C：进入八路传感器动态标定，电机禁止
E：结束标定并保留 RAM 极值
R：进入 RUNNING，启动完整循迹和速度闭环
S：正常停止并拉低 STBY
X：紧急停止并锁存 FAULT
G：当前 IMU 偏航角清零
I：开始 IMU 静止零偏标定
```

`empty.c` 顶部新增两个默认注释的上电选项：

```c
/* #define CAR_POWER_ON_START_CALIBRATION */
/* #define CAR_POWER_ON_START_TRACKING    */
```

取消第一行注释会在初始化后自动进入传感器标定，电机仍然禁止；取消第二行会在初始化
后自动执行 `App_Car_Start()`，立即释放 TB6612 STBY 并进入完整闭环。两项同时启用会
由预处理器报错，避免含糊的上电状态。默认保持两行注释，满足“上电绝不自动运行”。

不要把 `LineSensor_Sample()`、`Encoder_Update()`、`Motor_Update()` 或 OLED 刷新再次
手工塞进 `while(1)`；这些调用已经由调度器按固定周期执行，重复调用会改变采样率、
滤波和斜坡行为。

本功能选择区已经用 TI ARM Clang 和正式 `device.opt` 分别验证：默认 IDLE、自动标定、
自动循迹三个合法分支均编译通过；同时定义两项时按设计触发互斥 `#error`。同步正式
工程后执行 `gmake -j4 all`，最终编译、链接和 HEX 生成退出码为 0。

## 20. 实车循迹转向极性修正

实测灰度模块黑线约 100、白底约 4095，因此 `CAR_LINE_BLACK_IS_LOW_RAW=1` 正确：
归一化后黑线接近 1000、白底接近 0，不应反转 ADC 黑白逻辑。实测黑线在车辆左侧时，
控制层生成左/右目标速度约 87/309，说明位置计算和差速混合关系有效；左右编码器、
实体电机对应和编码方向也已由用户确认。

针对最终底盘/传感器安装产生的整车转向极性，新增：

```c
#define CAR_LINE_STEERING_POLARITY (-1.0f)
```

该宏在 `line_control.c` 中统一乘到正常循迹 PID 输出和丢线搜索输出。`-1.0f` 翻转当前
转向，`+1.0f` 恢复原软件约定；不要用 `CAR_LINE_BLACK_IS_LOW_RAW`、电机极性或编码器
极性代替它。宏乘在完整 PID 输出之后，因此 P/I/D 符号保持一致。
