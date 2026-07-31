# MSPM0G3507 循迹车硬件移植与上板清单

## 1. 这份文档解决什么问题

控制算法已经与具体引脚解耦，当前剩余工作主要是：

1. 在 `empty.syscfg` 中创建所需外设；
2. 把生成的资源宏写入各个 `bsp/*.c`；
3. 根据实际接线修改 `config/board_config.h`；
4. 根据实物修改 `config/car_config.h`；
5. 按本文顺序逐项上板验证。

不要编辑以下生成文件：

```text
Debug/ti_msp_dl_config.c
Debug/ti_msp_dl_config.h
Debug/device_linker.cmd
Debug/makefile
```

它们会在重新运行 SysConfig 或构建时被覆盖。

## 2. 需要先提供或确认的硬件信息

- TB6612 的 PWMA、AIN1、AIN2、PWMB、BIN1、BIN2、STBY 接线；
- 左右电机分别接 TB6612 A 还是 B；
- 左右编码器 A/B 相接线；
- 编码器参数已知为 13 PPR、1:30；仍需确认 QEI 是否四倍频及实测一圈计数；
- 轮径；
- 循迹模块 CH0~CH7 的物理左右排列；
- 循迹模块 OUT 的有效电平，以及 5 V 供电时 OUT 的实际高电平；
- 调试 UART TX/RX 接线；
- 500 Hz IMU UART TX/RX 接线和模块串口电平；协议驱动已经完成；
- OLED 已正式分配 PA12=SCL、PA13=SDA；驱动按 128×64 SSD1306、地址 0x3C 移植；

## 3. TB6612 的 SysConfig 配置

### 3.1 PWM

创建两个边沿对齐 PWM 输出：

- PWMA：左电机；
- PWMB：右电机；
- 推荐频率：20 kHz；
- 初始占空比：0%；
- 输出极性应与 `bsp_tb6612.c` 的换算一致。

可以使用同一个定时器的两个 CC 通道，也可以使用两个定时器，但必须检查引脚复用
是否与 QEI、ADC、UART 冲突。

### 3.2 GPIO

创建五个推挽输出：

```text
AIN1 AIN2 BIN1 BIN2 STBY
```

上电初值全部为低。尤其是 STBY 必须默认低，防止 MCU 复位期间电机误动作。

### 3.3 填写 BSP

在 `bsp/bsp_tb6612.c` 完成：

- `BSP_TB6612_SetStandby()`；
- `BSP_TB6612_SetInputs()`；
- `BSP_TB6612_SetPwmPermille()`。

上层传入占空比范围是 `0~1000`，BSP 负责换算成计时器比较值。换算后必须再次
限幅，避免 1000 对应超过 LOAD 的比较值。

## 4. 1 ms 调度定时器

当前原工程已创建 `TICK_TIMER`，使用 TIMG0、BUSCLK/1、Prescaler=1、周期 1 ms、
Periodic Down Counting，并只启用 ZERO 中断。SysConfig 保持 Counter 停止，
`BSP_Time_Init()` 在清零软件时间、清中断和打开 NVIC 后手动启动。

生成宏已经约定为：

```c
TICK_TIMER_INST
TICK_TIMER_INST_INT_IRQN
TICK_TIMER_INST_IRQHandler
TICK_TIMER_INST_LOAD_VALUE /* 当前 40 MHz BUSCLK 下为 39999U，仍是 1 ms。 */
```

真实中断入口由 `bsp/bsp_time.c` 实现，核心行为只有：

```c
void TICK_TIMER_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TICK_TIMER_INST)) {
        case DL_TIMER_IIDX_ZERO:
            BSP_Time_Tick1msFromISR();
            break;
        default:
            break;
    }
}
```

不要在该 ISR 中调用 `App_Scheduler_Run()`。控制和通信任务在主循环执行，避免 ISR
过长并保持中断响应可预测。

验收：UART 遥测的第一列应每毫秒递增，长时间运行不停止；1 秒实测计数增量应接近
1000。当前已完成配置和编译验证，仍需下载后用 Watch/HC-05 验证实际节拍。

## 5. 左硬件 QEI + 右软件 AB 编码器

最终资源已经固定：左编码器使用 TIMG8、PB6/PB7 的 2-input QEI；右编码器使用
GPIOB PB8/PB9 双边沿中断和四状态查表。右编码器必须实现共享向量
`GROUP1_IRQHandler()`，不能使用不存在的 `GPIOB_IRQHandler()`。

参考 SDK：

```text
examples/nortos/LP_MSPM0G3507/driverlib/timg_qei_mode
```

`bsp/bsp_encoder.c` 已完成：左 16 位相邻差值扩展为 32 位累计计数；右 ISR 查表
±1；复位时原子清理硬件/软件状态；诊断接口暴露右轮边沿中断和非法跳变数。

验收方法：

1. 架空车辆；
2. 手动把左轮向车辆前进方向转一圈；
3. 记录计数变化；
4. 右轮重复；
5. 修改极性宏，使两侧向前都为正；
6. 把实测一圈计数写入 `CAR_ENCODER_COUNTS_PER_WHEEL_REV`。

## 6. 八路真实模拟循迹阵列

### 6.1 软件结构

当前模块的八个输出都是连续模拟电压，不再使用 AD0/AD1/AD2 地址线或数字 OUT。
BSP 使用两个 ADC12 外设组成一帧：

```text
ADC0：MEM0~MEM2，共 3 路 -> sensor[0..2]
ADC1：MEM0~MEM4，共 5 路 -> sensor[3..7]
```

两个序列均为 12 位、软件触发、非重复序列。ADC0 的 MEM2 和 ADC1 的 MEM4 完成
中断只置位标志；1 ms 主任务必须等两个标志都到达后，才一次性复制八路结果。这样
不会提交半帧 ADC0 新数据和半帧 ADC1 旧数据，也没有轮询等待、延时或休眠。

### 6.2 已验证的推荐引脚与数组顺序

下面组合已经通过 SysConfig 1.28.0 实际验证，可与 UART0/PA10/PA11、UART1/PB4/PB5、
SWD PA19/PA20、预留 I2C1/PB2/PB3 和预留 UART2/PB15/PB16 同时存在：

| 数组索引 | ADC/MEM | ADC 输入 | MCU 引脚 | 板级注意事项 |
|---:|---|---|---|---|
| 0（最左） | ADC0 MEM0 | ADC0.2 | PA25 | BoosterPack，干净 |
| 1 | ADC0 MEM1 | ADC0.3 | PA24 | BoosterPack，干净 |
| 2 | ADC0 MEM2 | ADC0.7 | PA22 | 断开/确认 J16 光线传感器 |
| 3 | ADC1 MEM0 | ADC1.0 | PA15 | BoosterPack，DAC 未使用时可作 ADC |
| 4 | ADC1 MEM1 | ADC1.2 | PA17 | BoosterPack，干净 |
| 5 | ADC1 MEM2 | ADC1.4 | PB17 | BoosterPack，干净 |
| 6 | ADC1 MEM3 | ADC1.5 | PB18 | BoosterPack，干净 |
| 7（最右） | ADC1 MEM4 | ADC1.6 | PB19 | BoosterPack，干净 |

传感器输出必须按上表从车辆物理最左到最右接线。若整排八根线刚好完全反向，可以
临时设置 `CAR_LINE_ADC_REVERSE_ORDER=1`；若只是中间个别通道错位，应修正接线或
SysConfig MEM 顺序，不能用整体反转掩盖。

### 6.3 SysConfig 配置

添加两个 ADC12 实例，分别命名为 `LINE_ADC0` 和 `LINE_ADC1`。

两者共同设置：

```text
Sampling Operation Mode：Sequence
Repeat Mode：Disabled
Trigger Source：Software
Resolution：12-bit unsigned
Reference：VDDA 3.3 V
Power Down Mode：Manual
ADC clock：ULPCLK / 8
Sample Time 0：40 us
```

`LINE_ADC0`：

```text
Peripheral：ADC0
Start Address：MEM0
End Address：MEM2
MEM0 input：Channel 2 / PA25
MEM1 input：Channel 3 / PA24
MEM2 input：Channel 7 / PA22
Interrupt：MEM2 result loaded
```

`LINE_ADC1`：

```text
Peripheral：ADC1
Start Address：MEM0
End Address：MEM4
MEM0 input：Channel 0 / PA15
MEM1 input：Channel 2 / PA17
MEM2 input：Channel 4 / PB17
MEM3 input：Channel 5 / PB18
MEM4 input：Channel 6 / PB19
Interrupt：MEM4 result loaded
```

保存并生成后，检查 `ti_msp_dl_config.h` 中存在：

```c
LINE_ADC0_INST / LINE_ADC0_INST_INT_IRQN / LINE_ADC0_INST_IRQHandler
LINE_ADC1_INST / LINE_ADC1_INST_INT_IRQN / LINE_ADC1_INST_IRQHandler
```

确认无冲突后，将：

```c
#define CAR_LINE_ADC_READY (0U)
```

改为：

```c
#define CAR_LINE_ADC_READY (1U)
```

当前原工程已经完成上述 SysConfig 配置并正式使用 `1U`。这段切换流程保留给以后
删除、重命名或重新分配 ADC 实例时参考。

如果改变 3+5 分组或序列终点，必须同时修改 `CAR_LINE_ADC0/1_CHANNEL_COUNT`、完成
中断 IIDX 和中断 MASK。只换某个 MEM 对应的模拟引脚而不改变序列长度时，BSP 无需改。

### 6.4 电气与采样要求

- 八个模拟输出都必须位于 `0~VDDA`，当前参考为 3.3 V；严禁把 5 V 模拟量直接送入 ADC；
- 模块和 MSPM0 必须共地；电机地回流不要经过传感器信号地；
- 40 us 采样窗口对常见带比较/放大输出的灰度模块较宽裕；如模块输出阻抗很高，仍需
  用示波器确认采样时不会明显拉低电压；
- 传感器电源旁放置去耦，八根模拟线远离 TB6612 电机端子和 PWM 走线；
- PA22 使用前处理 J16，避免板载光线传感器与外部灰度输出并联。

### 6.5 上板观察与标定

Watch 建议：

```c
BSP_LineADC_IsReady()
BSP_LineADC_GetFrameCount()
BSP_LineADC_GetRestartCount()
BSP_LineADC_GetUnexpectedIrqCount()
LineSensor_GetData()->raw[0..7]
LineSensor_GetData()->normalized[0..7]
LineSensor_GetData()->position
LineSensor_GetData()->lineDetected
```

正常情况下，`raw[]` 应随灰度连续变化，而不是只有 0/4095；`frameCount` 在 1 ms 任务
下应接近每秒 1000，`restartCount` 和 `unexpectedIrqCount` 应保持 0。执行 C 开始标定，
让全部探头都经过赛道线和背景，再用 E 结束；目标线归一化后应接近 1000、背景接近 0。
若黑白强度整体相反，只修改 `CAR_LINE_BLACK_IS_LOW_RAW`；若位置左右整体相反，先检查
物理接线，再考虑 `CAR_LINE_ADC_REVERSE_ORDER`。PID 参数必须根据真实连续模拟数据重调。

## 7. 调试 UART

调试链路当前通过 HC-05 做透明串口。`bsp_uart.c` 已实现 512 字节 TX、256 字节 RX
环形缓冲、RX/TX FIFO 中断和错误统计。用户已在 SysConfig 完成当前分配：

推荐配置：

```text
实例名：HC05_UART
外设：UART1
TX：PB4
RX：PB5
115200 baud, 8 data bits, no parity, 1 stop bit
TX + RX，无硬件流控
FIFO：开启
RX FIFO threshold：>= 1 entry
中断：RX；TX 中断由 BSP 按发送队列状态动态开关
```

`board_config.h` 当前已经启用：

```c
#define CAR_HC05_UART_READY 1U
```

现有非阻塞接口：

- `BSP_UART_TryWrite()`：只入队，队列满时返回 false；
- `BSP_UART_TryWriteString()`：发送不包含结尾 `\0` 的字符串；
- `BSP_UART_TryWriteByte()`：发送一个字节；
- `BSP_UART_TryReadByte()`：RX 队列有数据才返回 true；
- `BSP_UART_Read()`：批量取走当前已经收到的数据；
- 三组诊断计数可区分 RX 溢出、硬件错误和 TX 队列拒绝；
- 不允许等待一个完整字符串发送完毕。

上层命令：R 开始、S 停止、X 紧急停止、C/E 循迹标定、G 偏航角清零、
I 开始 IMU 静止零偏标定。

HC-05 数据模式波特率必须与 SysConfig 一致；常见模块出厂可能为 9600，而当前
20 ms CSV 遥测推荐 115200。PB4 已被 UART1 TX 占用，后续 TB6612 PWM 不得再选
PB4；PB5 位于板卡下方未焊接接口区域，需确认实物引出。如果自动生成文件里暂时找
不到 `HC05_UART_*` 宏，保存 SysConfig 并执行一次 Clean + Build 让生成器刷新，不能
手工补写生成文件。引脚和接线详见 `BOARD_PINOUT.md`。

## 8. 500 Hz 串口 IMU

当前 IMU 不是 I2C 芯片，而是自带解算和串口协议的模块。参考工程确认参数如下：

```text
UART：115200 baud，8N1，无流控
上报频率：500 Hz（每 2 ms 一帧）
每帧：9 bytes
帧格式：0A 03 04 AngleH AngleL DpsH DpsL CRCL CRCH
角度比例：0.1 degree/LSB
角速度比例：0.1 degree/second/LSB
CRC：Modbus CRC16，初值 FFFF，多项式 A001，低字节先传
配置命令：AA 06 01 01 01 AD 00
```

### 8.1 接线与 UART 分配

必须给 IMU 使用独立 UART，不能与调试 CSV 串口共用：

```text
IMU VCC -> 按模块额定电压连接（先核对模块说明）
IMU GND -> MSPM0 GND
IMU TX  -> MSPM0 的 IMU_UART_RX
IMU RX  -> MSPM0 的 IMU_UART_TX
```

本工程当前已经配置 UART0、PA10 作为 MCU TX、PA11 作为 MCU RX；PA10/PA11 在
LaunchPad 上涉及 J21/J22 到 XDS/BoosterPack 的路由，上板前必须检查跳线。还需要
确认模块串口电平是 3.3 V TTL；若模块 TX 是 5 V，不得直接接 MSPM0 RX。

### 8.2 SysConfig

在 `empty.syscfg` 新增一个 UART，建议实例名为 `IMU_UART`：

```text
波特率：115200
数据位：8
校验：None
停止位：1
方向：TX + RX
中断：RX（BSP 运行时还会按需启停 TX 中断）
```

然后在 `board_config.h` 修改：

```c
#define CAR_IMU_UART_READY 1U
```

默认别名期望 SysConfig 生成：

```c
IMU_UART_INST
IMU_UART_INST_INT_IRQN
IMU_UART_INST_IRQHandler
```

若名字不同，只修改 `CAR_IMU_UART_INST`、`CAR_IMU_UART_IRQN` 和
`CAR_IMU_UART_IRQ_HANDLER`。BSP 会通过最后一个宏生成真正的中断入口；如果你已
在其他文件定义同名 IRQHandler，必须只保留一个，并从已有入口调用
`BSP_IMU_UART_IRQHandler()`。

### 8.3 软件数据流和实时性

```text
UART RX ISR
  -> 64 字节 RX 环形缓冲
  -> 1 ms App_Car_SensorTask1ms
  -> IMU_Update(0.001f)
  -> 帧头同步 / CRC / 大端有符号数
  -> 角度回绕 / 零偏 / 超时
  -> IMU_GetData()
```

- 中断中不做浮点计算，也不做 CRC；
- 1 ms 任务单次最多处理 32 字节，正常 500 Hz 数据只约 4.5 bytes/ms；
- 原 `motor_crc.c` 的 512 字节查表没有带入，CRC 已用紧凑位算法融合进 `imu.c`；
- 原参考程序的 3 秒阻塞延时已改为分时计时，到期后非阻塞发送配置命令；
- 连续 20 ms 没有正确新帧时 `valid=false`；
- 原始累计角度跨越 `32767 -> -32768` 时会按模运算连续展开；
- `CAR_IMU_YAW_POLARITY` 可统一反转角度和角速度方向；
- `IMU_StartGyroCalibration()` 默认静止收集 500 帧，约 1 秒完成；
- `IMU_ResetYaw()` 只把应用层相对偏航角清零，不写模块内部寄存器。

上板时先让模块静止，观察调试 CSV 中 `imu_valid`、`yaw_x10`、`gyro_x10` 和
`imu_crc_errors`。若 CRC 持续增长，优先检查波特率、TX/RX 交叉、共地、电平和
串口干扰，不要用未经验证的角度参与车辆转向控制。

## 9. OLED 软件 I2C 和可选硬件 I2C

OLED 已使用独立软件 I2C，不占用硬件 I2C Controller，也不占用 IMU 的 UART。当前驱动
对应 128×64 SSD1306、7 位地址 0x3C，并保留江协科技 OLED V2.0 的显存、字体、
中文和绘图 API。

### 9.1 OLED 接线与电气要求

```text
OLED VCC -> 3.3 V
OLED GND -> GND
OLED SCL -> PA12
OLED SDA -> PA13
```

建议给 OLED 使用 3.3 V，避免某些模块把 I2C 上拉到 5 V。检查模块是否已有上拉；
如果没有，在 SCL/SDA 各加约 4.7 kΩ 到 3.3 V。

### 9.2 SysConfig

正式 SysConfig 已添加 `OLED_GPIO` 组，组内 Pin Name 为 `SCL` 和 `SDA`：

```text
SCL: PA12, Output, Initial Set, No Resistor, Low Drive, Hi-Z Enable
SDA: PA13, Output, Initial Set, No Resistor, Low Drive, Hi-Z Enable
```

BSP 使用输出使能模拟开漏：输出低时主动下拉，输出高时关闭输出并由电阻上拉。
SysConfig 的 `Initial Set + Hi-Z Enable` 还保证应用 BSP 接管前总线处于释放状态。因此
不需要硬件 I2C 外设，也不会把总线高电平做成推挽输出。

然后在 `board_config.h`：

```c
#define CAR_OLED_SOFT_I2C_READY 1U
```

当前已经核对的生成符号为：

```c
OLED_GPIO_PORT
OLED_GPIO_SCL_PIN
OLED_GPIO_SDA_PIN
```

`board_config.h` 已把四个 BSP 别名映射到这些正式符号。

`CAR_OLED_SOFT_I2C_DELAY_CYCLES` 控制位间隔，应使用示波器观察 SCL 后调整。该短
忙等只形成软件 I2C 时序，CPU 不会进入休眠。

### 9.3 调用限制

- `OLED_Init()` 应在供电稳定后、车辆起跑前调用；
- `OLED_IsConnected()` 可检查初始化/最近写入是否全部收到 ACK；
- 完整 `OLED_Update()` 要发送 1 KiB，软件 I2C 下耗时较长；
- 车辆运行时优先使用 `OLED_UpdateArea()` 更新小区域；
- 禁止在 ISR、1 ms 传感器任务或 5 ms 控制任务中刷新 OLED；
- 当前诊断任务每 100 ms 只发送一个 8 像素页，八行约 0.8 秒轮换一遍；
- 初始化期间任意字节 NACK 后停止周期刷新，避免无设备时持续占用 CPU；
- OLED 同步传输仍会造成有界的主循环延迟，正式竞速前应测量单页耗时，必要时降低
  刷新率或关闭 OLED 诊断任务。

OLED 软件 I2C 和 IMU UART 互不占用同一个外设实例，但仍应避免把 OLED 整屏刷新
放进 1 ms IMU 接收任务或 5 ms 控制任务。

## 10. 首次通电安全顺序

MG513 铭牌堵转电流约 2.8 A，而 TB6612 常用连续输出能力约为 1.2 A/通道。不要把
TB6612 的短时峰值能力当成可持续电流；调试时应限制占空比、避免卡死车轮，并关注
驱动芯片温升。后续建议增加堵转判断或电流检测。

1. 不接电机电源，只给 MCU 供电；
2. 确认 STBY 上电保持低；
3. 确认 PWMA/PWMB 上电为低；
4. 接 TB6612 逻辑电源，仍不接电机；
5. 用示波器检查两个 PWM 输出；
6. 架空车轮再接电机电源；
7. 先将占空比限制设低；
8. 单独测试左轮；
9. 单独测试右轮；
10. 确认紧急停止会立刻关闭 STBY；
11. 最后让车轮接地测试。

不要在理论 1560 count/圈尚未实测确认时直接高速闭环运行。

## 11. 建议联调阶段和完成标准

### 阶段 A：基础通信

- 1 ms 时间正确；
- 调试 UART 能收发；
- IMU UART 在 500 Hz 下不溢出且 CRC 稳定；
- R/S/X/C/E/G/I 命令行为正确。

灰度 ADC 可以与通信阶段并行验收：先不接电机电源，观察完整帧计数接近 1 kHz，
逐个改变八路探头下方灰度，确认对应 `raw[]` 连续变化，再验证左右顺序。

### 阶段 B：电机开环

- 两轮分别正反转；
- `Motor_SetTargetPermille()` 正值都使车辆向前；
- 占空比斜坡可观察；
- S/X 能安全停机。

### 阶段 C：编码器

- 向前都为正；
- 一圈计数准确；
- 低速没有异常巨大跳变；
- 高速不丢计数。

### 阶段 D：速度闭环

- 分别调左右 PI；
- 阶跃响应不过度振荡；
- 堵住轮子时输出会限幅；
- 松手后能恢复；
- 相同目标速度下车辆基本直行。

### 阶段 E：循迹

- 八路 ADC 顺序、黑白跨度和连续模拟变化正确；
- 位置符号正确；
- 先只开 P，低速跑通；
- 再加入 D 抑制摆动；
- 最后增加弯道降速和直线速度。

## 12. 工程构建注意事项

复制或新增目录后，在 CCS Theia 中：

1. 右键工程执行 Refresh；
2. 确认 app/bsp/config/control/drivers 都显示；
3. 执行 Clean Project；
4. 执行 Build Project；
5. 如果 SysConfig 报引脚冲突，先解决复用冲突，不要改生成文件。

整个工程明确不使用 MCU 睡眠、WFI、Standby 或低功耗策略。TB6612 的 STBY 只是
电机驱动使能信号，与 MCU 低功耗无关。

## 13. 当前最终资源与 BSP 实现（2026-07-25）

当前正式配置已不再是占位方案：

```text
CPUCLK/MCLK = 80 MHz，ULPCLK = 40 MHz
TB6612 PWM  = TIMG12，PB13/CCP0 + PA31/CCP1，Period=4000，20 kHz
TB6612 GPIO = PB0/PB1/PB12/PB20，STBY=PA28
左编码器    = TIMG8 QEI，PB6/PB7
右编码器    = GPIOB PB8/PB9，两脚双边沿，GROUP1_IRQHandler
```

`bsp_tb6612.c` 已实现真实 GPIO/PWM 写入。初始化顺序固定为：停止 TIMG12、STBY 低、
四根方向线低、两通道 0%、启动 TIMG12；启动 PWM 计数器不等于启用电机，因为 STBY
保持低。向下计数 PWM 使用：

```c
compare = period - (period * dutyPermille / 1000);
```

因此 0% 对应 Compare=4000，100% 对应 Compare=0。

`bsp_encoder.c` 已实现左轮 16 位 QEI 的软件 32 位累计扩展，以及右轮四状态查表。
右轮 ISR 只分派 GPIOB、清 pending、采样 AB、查表增减并更新诊断计数，不做 PID、
OLED、UART、浮点或延时。OLED 诊断现在完整轮换两套 8 页画面：ADC/循迹页和
电机/编码器页；只在 8 页边界切换，避免一轮刷新混合两种内容。

80 MHz 后 OLED 位延时不再写死为 40，而是按：

```c
CPUCLK_FREQ / 800000UL
```

自动换算；在 80 MHz 下为 100 cycles，与原 32 MHz/40 cycles 等比例。这个值只能
作为起点，仍应通过实物显示和示波器 SCL 波形复测。
