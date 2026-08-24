# A-Distance-Alarm-System-Based-on-STM32F103C8T6
基于STM32F103C8T6的距离报警系统
这是一个基于STM32F103C8T6单片机的简单距离报警系统。使用HC-SR04超声波传感器测量距离，当距离小于5厘米时触发有源蜂鸣器报警，同时距离值实时显示在OLED屏幕上。

功能特点
使用HC-SR04超声波传感器实时测量距离

0.96英寸OLED显示屏（I2C接口）实时显示距离

距离小于5 cm时，有源蜂鸣器发出报警

利用TIM1的通道1和通道2进行输入捕获，精确测量回波脉冲宽度

设计紧凑，成本低廉

硬件需求
STM32F103C8T6 “Blue Pill” 开发板

HC-SR04超声波测距模块

0.96寸 I2C OLED显示屏（SSD1306驱动）

有源蜂鸣器（5V）

面包板及杜邦线若干

5V电源（USB或外部电源）

引脚连接
模块	STM32F103C8T6 引脚
HC-SR04 Trig	PA0（或其他可用GPIO）
HC-SR04 Echo	PA8（TIM1_CH1）和 PA9（TIM1_CH2）¹
OLED SDA	PB7（I2C1_SDA）
OLED SCL	PB6（I2C1_SCL）
蜂鸣器正极	PB12
蜂鸣器负极	GND
VCC（HC-SR04、OLED、蜂鸣器）	5V
GND（所有模块）	GND
¹ 将Echo信号同时连接到TIM1_CH1（PA8）和TIM1_CH2（PA9），分别捕获上升沿和下降沿，这样避免了在单个通道上反复切换捕获边沿，使时序更加稳定。如果希望简化，也可以只使用一个通道，在软件中动态修改捕获极性。

工作原理
单片机向HC-SR04的Trig引脚发送一个10 µs的高电平触发脉冲。

HC-SR04发射超声波并拉高Echo引脚，直到收到回波后拉低。

TIM1配置为输入捕获模式，两个通道分别工作：

通道1（CH1）捕获Echo信号的上升沿；

通道2（CH2）捕获下降沿。

根据两次捕获的时间差即可得到回波脉冲宽度。

距离计算公式：
距离 = (脉冲宽度 × 声速) / 2
其中声速约为343 m/s（20°C时）。

如果计算出的距离小于0.05米（5厘米），则打开有源蜂鸣器；否则关闭蜂鸣器。

距离数值持续更新显示在OLED屏幕上。

软件实现
使用STM32标准库开发。

使用I2C1与OLED通信。

TIM1配置为输入捕获模式，启用两个通道。

使用简单的软件延时或定时器中断产生触发脉冲并控制测量周期。

关键配置代码
c
// TIM1 配置（简化）
htim1.Instance = TIM1;
htim1.Init.Prescaler = 71;        // 定时器时钟1 MHz（72 MHz / 72）
htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
htim1.Init.Period = 0xFFFF;       // 最大计数周期
htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
htim1.Init.RepetitionCounter = 0;

// 通道1输入捕获（上升沿）
sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;
sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
sConfigIC.ICFilter = 0;
HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1);

// 通道2输入捕获（下降沿）
sConfigIC.ICPolarity = TIM_ICPOLARITY_FALLING;
HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_2);

HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
编译与烧录
使用keil5打开工程。

将ST-Link调试器连接到开发板。

编译工程（Project -> Build Project）。

烧录固件（Run -> Run 或 Debug）。

使用说明
为开发板及所有外设提供5V电源。

OLED屏幕会实时显示当前距离（单位：厘米或米，根据代码实现而定）。

将障碍物放置在超声波传感器前方。

当障碍物距离小于5厘米时，蜂鸣器会持续鸣响。

注意事项
HC-SR04需要5V供电才能稳定工作。Echo引脚输出5V电平，而STM32F103的GPIO（除模拟引脚外）是5V容忍的，因此可以将Echo直接连接到PA8/PA9。Trig引脚由3.3V GPIO驱动即可正常触发HC-SR04。

许可证
本项目基于MIT许可证开源。详情请参阅 LICENSE 文件。
