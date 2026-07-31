# LP-MSPM0G3507 循迹小车引脚分配建议

## 1. 结论先行

在保留当前 IMU、HC-05 和 SWD 的前提下，推荐把八路模拟灰度阵列分成 ADC0 三路与
ADC1 五路，并明确预留一组硬件 I2C 和一组硬件 UART：

| 资源 | 实例 | 引脚 | 当前结论 |
|---|---|---|---|
| IMU 串口 | UART0 | PA10 TX / PA11 RX | 原工程已配置 |
| HC-05 串口 | UART1 | PB4 TX / PB5 RX | 原工程已配置 |
| 八路灰度前 3 路 | ADC0 | PA25 / PA24 / PA22 | 推荐，PinMux 已验证 |
| 八路灰度后 5 路 | ADC1 | PA15 / PA17 / PB17 / PB18 / PB19 | 推荐，PinMux 已验证 |
| OLED 软件 I2C | 普通 GPIO | PA12 SCL / PA13 SDA | 原工程已配置，Hi-Z 开漏方式 |
| 备用硬件 I2C | I2C1 | PB2 SCL / PB3 SDA | 推荐保留，PinMux 已验证 |
| 备用硬件 UART | UART2 | PB15 TX / PB16 RX | 推荐保留，PinMux 已验证 |
| 下载调试 | SWD | PA19 SWDIO / PA20 SWCLK | 必须保留 |

以上资源同时存在时，已经通过 SysConfig 1.28.0 的 `Validating` 和代码生成，没有
PinMux 冲突。验证文件位于：

```text
work/verify_adc_syscfg/verify_adc_and_reserved_io.syscfg
```

这里的“已验证”只表示器件外设实例、封装和 PinMux 可以同时成立，不代表原工程已经
启用这些 ADC/I2C/UART，也不替代实物电平、跳线和噪声测试。

## 2. 八路模拟灰度接线

代码约定 `raw[0]` 永远代表车辆最左侧探头，`raw[7]` 永远代表最右侧探头。推荐按照
下面的顺序接线，避免上层质心算法和车辆物理方向互相打架。

| 探头位置 | 代码索引 | SysConfig 序列 | ADC 输入 | MCU 引脚 | 备注 |
|---|---:|---|---|---|---|
| 最左 | 0 | `LINE_ADC0` MEM0 | ADC0.2 | PA25 | BoosterPack，无已知板载负载 |
| 左 2 | 1 | `LINE_ADC0` MEM1 | ADC0.3 | PA24 | BoosterPack，无已知板载负载 |
| 左 3 | 2 | `LINE_ADC0` MEM2 | ADC0.7 | PA22 | 必须检查或断开 J16 板载光线传感器 |
| 左 4 | 3 | `LINE_ADC1` MEM0 | ADC1.0 | PA15 | 不启用 DAC 时可作 ADC 输入 |
| 右 4 | 4 | `LINE_ADC1` MEM1 | ADC1.2 | PA17 | BoosterPack，无已知板载负载 |
| 右 3 | 5 | `LINE_ADC1` MEM2 | ADC1.4 | PB17 | BoosterPack，无已知板载负载 |
| 右 2 | 6 | `LINE_ADC1` MEM3 | ADC1.5 | PB18 | BoosterPack，无已知板载负载 |
| 最右 | 7 | `LINE_ADC1` MEM4 | ADC1.6 | PB19 | BoosterPack，无已知板载负载 |

如果实物排线天然是右到左，优先交换排线；确实不便交换时，可把
`CAR_LINE_ADC_REVERSE_ORDER` 改为 `1U`。不要为了修左右方向去修改质心权重公式。

## 3. 为什么选择这些 ADC 引脚

这组引脚的目标不是单纯“凑够八个 ADC”，而是在板级风险、接线方便和后续扩展之间
做平衡：

- 避开 PA0/PA1 的 5 V Open-Drain 特性，不把特殊脚当普通模拟输入；
- 避开 PA2~PA6 的 ROSC/LF/HF 晶振相关功能；
- 避开 PA18 的默认 BSL、S1 和 SW2 路由；
- 保留 PA19/PA20 给 SWD 下载和调试；
- 避开 PA21/PA23 VREF 引脚；
- 避开 PA16 的 SW2 路由；
- 避开 PB24 板载温度传感器；
- 避开 PB25 未焊接的 SMA/ADC 位置；
- 不占用 PB2/PB3，给硬件 I2C 留出一整组；
- 不占用 PB15/PB16，给 UART2 留出一整组。

唯一需要额外处理的是 PA22。它与板载光线传感器通过 J16 相关，外接灰度模拟输出前
必须根据开发板实物确认 J16 状态，避免板载器件与外部模块同时加载 ADC 节点。

## 4. 建议在 CCS SysConfig 中怎样配置 ADC

不要手工修改自动生成的 `ti_msp_dl_config.h/.c`。应在 `empty.syscfg` 的图形界面中
添加两个 ADC12 实例，并让 SysConfig 生成外设初始化和宏。

### 4.1 `LINE_ADC0`

```text
实例名：LINE_ADC0
外设：ADC0
Sampling Operation Mode：Sequence
Repeat Mode：Disabled
Trigger：Software
Resolution：12-bit unsigned
Reference：VDDA（3.3 V）
Power Down Mode：Manual
ADC clock：ULPCLK / 8
Sample Time 0：40 us
Start Address：MEM0
End Address：MEM2

MEM0：Channel 2 / PA25
MEM1：Channel 3 / PA24
MEM2：Channel 7 / PA22

Interrupt：MEM2 result loaded
```

### 4.2 `LINE_ADC1`

```text
实例名：LINE_ADC1
外设：ADC1
Sampling Operation Mode：Sequence
Repeat Mode：Disabled
Trigger：Software
Resolution：12-bit unsigned
Reference：VDDA（3.3 V）
Power Down Mode：Manual
ADC clock：ULPCLK / 8
Sample Time 0：40 us
Start Address：MEM0
End Address：MEM4

MEM0：Channel 0 / PA15
MEM1：Channel 2 / PA17
MEM2：Channel 4 / PB17
MEM3：Channel 5 / PB18
MEM4：Channel 6 / PB19

Interrupt：MEM4 result loaded
```

保存并生成后，核对自动生成头文件里存在以下命名：

```c
LINE_ADC0_INST
LINE_ADC0_INST_INT_IRQN
LINE_ADC0_INST_IRQHandler

LINE_ADC1_INST
LINE_ADC1_INST_INT_IRQN
LINE_ADC1_INST_IRQHandler
```

首次配置时，确认宏名与 `config/board_config.h` 的别名一致并完整构建通过后，才把：

```c
#define CAR_LINE_ADC_READY (0U)
```

改为：

```c
#define CAR_LINE_ADC_READY (1U)
```

`READY=0` 是未配置硬件时的安全状态：原工程现在已经完成上述配置，因此当前正式值
为 `1U`。如果以后删除 ADC 实例或改名，应在重新适配前先恢复为 `0U`。

## 5. 预留硬件 I2C

推荐预留：

```text
实例：I2C1
实例名：SPARE_I2C
SCL：PB2
SDA：PB3
```

PB2/PB3 是 BoosterPack 常用 I2C 位置，且在本方案中没有被 ADC、UART 或 SWD 占用。
注意事项：

- SCL 和 SDA 需要上拉到 3.3 V；
- 不允许上拉到 5 V；
- 总线较长或电机干扰明显时可从 4.7 kΩ 起步，根据波形调整；
- 现在的 OLED 驱动是软件 I2C，不要求占用这组硬件 I2C；最好把 PB2/PB3 真正留给
  将来需要硬件控制器、速率或中断能力的传感器。

## 6. 预留硬件 UART

推荐预留：

```text
实例：UART2
实例名：SPARE_UART
TX：PB15
RX：PB16
```

这组引脚均位于 BoosterPack 接口且没有已知板载负载。MSPM0G3507 还有 UART2 和
UART3，因此 UART0 给 IMU、UART1 给 HC-05 并不意味着串口已经用完。PB15/PB16
作为 UART2 比 PA8/PA9 或 PB6/PB7 更利于保持现有接口规划稳定。

预留不等于现在必须在原工程中添加实例。若暂时不用，文档和接线层面不分配这些脚
即可；以后启用时再通过 SysConfig 添加并检查全局冲突。

## 7. 已完成的电机和编码器资源

八路 ADC、两个串口、备用 I2C/UART 固定后，电机与编码器也已经完成最终分配：

| 功能 | 最终资源 | 状态 |
|---|---|---|
| TB6612 PWMA/PWMB | TIMG12 CCP0=PB13、CCP1=PA31 | 80 MHz/4000=20 kHz，生成通过 |
| TB6612 AIN1/AIN2/BIN1/BIN2 | PB0/PB1/PB12/PB20 | GPIO 输出，初始低 |
| TB6612 STBY | PA28 | GPIO 输出，初始低；只在明确启动时释放 |
| 左编码器 | TIMG8 QEI，PB6/PB7 | 2-input QEI，LOAD=65535 |
| 右编码器 | GPIOB PB8/PB9 | 双边沿中断，`GROUP1_IRQHandler` 软件 AB 解码 |
| 1 ms 系统时基 | TIMG0 / `TICK_TIMER` | 已配置为 1 ms Periodic ZERO 中断，不占用引脚 |
| OLED 软件 I2C | PA12 SCL / PA13 SDA | 已正式配置；不占用 PB2/PB3，给硬件 I2C 留口 |

该方案仍完整保留 I2C1 PB2/PB3 和 UART2 PB15/PB16。理论编码器一圈计数为
`13×30×4=1560`，但方向符号和准确计数都必须通过手转车轮一整圈实测后再确认。

## 8. 建议避开的引脚

| 引脚 | 风险或用途 | 建议 |
|---|---|---|
| PA0/PA1 | 5 V Open-Drain 特殊脚和板级上拉/LED 路由 | 不作普通推挽 GPIO、UART 或 ADC 首选 |
| PA2~PA6 | ROSC、低频和高频晶振相关 | 保留，不分配 |
| PA18 | 默认 BSL、S1、SW2/J15 | 不用于关键实时信号 |
| PA19/PA20 | SWDIO/SWCLK | 保留下载调试 |
| PA21/PA23 | VREF-/VREF+ | 保留，不作普通 I/O |
| PA16 | SW2 路由 | 有替代时避开 |
| PB24 | 板载温度传感器 | 有替代时避开 |
| PB25 | SMA/ADC 焊位，实物接入不便 | 不作为八路阵列首选 |
| PB22/PB26/PB27 | 板载 RGB LED | 除非处理 J5/J6/J7，否则不作关键输入 |

PA10/PA11 已用于 IMU 且涉及 J21/J22 路由；PB4/PB5 已用于 HC-05。它们不是“一律
不能用”的风险脚，但已经成为固定资源，后续不可重复分配。

## 9. 上板前后核对清单

### 9.1 接线前

- [ ] 模块与 MSPM0 共地；
- [ ] 用万用表或示波器确认八路模拟输出始终在 0~3.3 V；
- [ ] 若模块使用 5 V 供电，已确认输出不会随之升到 5 V；
- [ ] PA22 对应 J16 已处理；
- [ ] `raw[0]` 到 `raw[7]` 的物理顺序为最左到最右；
- [ ] 模拟线远离 TB6612 PWM、电机相线和大电流地回路；
- [ ] PB2/PB3、PB15/PB16 没有被其他普通 GPIO 临时占用。

### 9.2 SysConfig 后

- [ ] `LINE_ADC0` 是 ADC0、MEM0~MEM2、末尾中断为 MEM2；
- [ ] `LINE_ADC1` 是 ADC1、MEM0~MEM4、末尾中断为 MEM4；
- [ ] 分辨率为 12 位无符号、参考为 VDDA 3.3 V；
- [ ] 两组都是软件触发、Sequence、非 Repeat；
- [ ] SysConfig 没有红色 PinMux 冲突；
- [ ] 自动生成宏与 `board_config.h` 别名一致；
- [x] 自动生成宏已核对，并将 `CAR_LINE_ADC_READY` 正式改为 `1U`；

### 9.3 运行时

- [ ] `raw[0..7]` 随黑白和高度连续变化，不是长期只有 0 或 4095；
- [ ] `BSP_LineADC_GetFrameCount()` 持续增长，约每毫秒增长一次；
- [ ] `BSP_LineADC_GetRestartCount()` 正常情况下不持续增长；
- [ ] `BSP_LineADC_GetUnexpectedIrqCount()` 正常情况下保持 0；
- [ ] 逐个遮挡探头时，变化的数组索引与物理位置一致；
- [ ] 黑线与白底极性相反时只修改 `CAR_LINE_BLACK_IS_LOW_RAW`；
- [ ] 完成动态黑白标定后再调循迹 PD/PID 参数。

## 10. 当前状态与下一步

软件层已经完成八路真实 ADC 驱动、每路标定、归一化、质心位置、丢线和路口基础判断。
ADC 真分支先用临时 SysConfig 自动生成宏做过严格编译验证；随后用户已经在原工程
`empty.syscfg` 中正式加入 `LINE_ADC0`、`LINE_ADC1`，并恢复 IMU UART0 与 HC-05
UART1。随后又正式加入 `OLED_GPIO`：PA12=SCL、PA13=SDA、Initial Set、Hi-Z Enable。
这些实例以及 TIMG12 PWM、TIMG8 QEI、右编码器 GPIO 同时保存且无 PinMux 冲突；
`CAR_LINE_ADC_READY`、`CAR_OLED_SOFT_I2C_READY`、`CAR_TB6612_READY` 和
`CAR_ENCODER_READY` 均已正式启用为 `1U`。用户已经用 OLED 确认八路 ADC 从左到右
对应索引 0～7，OLED 也已正常显示。CPU 改为 80 MHz 后软件 I2C 延时已经按主频自动
换算为 100 cycles，仍需再观察显示和 SCL 波形。电机与编码器尚未通电台架验证。

本文不要求也不引入任何休眠、WFI、Standby 或低功耗流程。ADC 采样和主循环均按
电赛实时控制思路持续运行。
