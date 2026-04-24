# Modbus协议实现

<cite>
**本文引用的文件列表**
- [ags_mb.h](file://SRC/HARDWARE/ags_mb/ags_mb.h)
- [ags_mb.c](file://SRC/HARDWARE/ags_mb/ags_mb.c)
- [ModBusCrc16.h](file://SRC/HARDWARE/ags_mb/ModBusCrc16.h)
- [ModBusCrc16.c](file://SRC/HARDWARE/ags_mb/ModBusCrc16.c)
- [modbus.h](file://SRC/HARDWARE/modbus/modbus.h)
- [modbus.c](file://SRC/HARDWARE/modbus/modbus.c)
- [usInterface.c](file://SRC/HARDWARE/usinterface/usInterface.c)
- [usFunc.c](file://SRC/HARDWARE/usinterface/usFunc.c)
- [elab_common.h](file://SRC/3rd/common/elab_common.h)
</cite>

## 目录
1. [简介](#简介)
2. [项目结构](#项目结构)
3. [核心组件](#核心组件)
4. [架构总览](#架构总览)
5. [详细组件分析](#详细组件分析)
6. [依赖关系分析](#依赖关系分析)
7. [性能考量](#性能考量)
8. [故障排查指南](#故障排查指南)
9. [结论](#结论)
10. [附录](#附录)

## 简介
本文件面向通用开关器项目的Modbus协议实现，系统性梳理了Modbus RTU在项目中的支持情况、实现细节与扩展路径。重点覆盖：
- 支持的功能码：读保持寄存器(03H)、写单个保持寄存器(06H)、写多个保持寄存器(10H)等。
- 数据帧格式、寄存器映射与地址分配规则。
- CRC16校验算法的实现与应用。
- 与AGS协议的对比与兼容处理、协议切换机制。
- 调试方法与常见问题解决方案。
- 集成与扩展的技术指导。

## 项目结构
项目在硬件层提供了两套Modbus实现：
- AGS风格Modbus子集实现（ags_mb）：仅实现部分功能码，聚焦于读保持寄存器与写单个寄存器，适配开关器业务场景。
- 标准Modbus实现（modbus）：完整支持03H/06H/10H功能码，提供完整的寄存器映射与写入逻辑。

此外，串口调试接口（usinterface）提供协议切换与参数配置能力，便于开发与测试。

```mermaid
graph TB
subgraph "硬件抽象层"
USART2["USART2(232)"]
USART3["USART3(485)"]
TIM3["TIM3(波特率定时)"]
end
subgraph "协议实现"
AGS["AGS风格Modbus(ags_mb)"]
STD["标准Modbus(modbus)"]
CRC["CRC16校验(ModBusCrc16)"]
end
subgraph "应用与调试"
UI["串口调试(usinterface)"]
FUNC["命令处理(usFunc)"]
end
USART2 --> AGS
USART3 --> AGS
USART2 --> STD
USART3 --> STD
AGS --> CRC
STD --> CRC
UI --> FUNC
FUNC --> AGS
FUNC --> STD
```

图表来源
- [ags_mb.c:7-473](file://SRC/HARDWARE/ags_mb/ags_mb.c#L7-L473)
- [modbus.c:35-67](file://SRC/HARDWARE/modbus/modbus.c#L35-L67)
- [ModBusCrc16.c:62-74](file://SRC/HARDWARE/ags_mb/ModBusCrc16.c#L62-L74)
- [usInterface.c:15-106](file://SRC/HARDWARE/usinterface/usInterface.c#L15-L106)
- [usFunc.c:707-747](file://SRC/HARDWARE/usinterface/usFunc.c#L707-L747)

章节来源
- [ags_mb.h:1-163](file://SRC/HARDWARE/ags_mb/ags_mb.h#L1-L163)
- [modbus.h:1-213](file://SRC/HARDWARE/modbus/modbus.h#L1-L213)

## 核心组件
- AGS风格Modbus子集（ags_mb）
  - 支持功能码：03H（读保持寄存器）、06H（写单个保持寄存器），以及扩展功能码0x52。
  - 提供CRC16校验、帧解析、错误处理与发送/接收流程。
  - 寄存器映射：线圈位、离散输入、保持寄存器、输入寄存器四类，按宏定义划分。
- 标准Modbus实现（modbus）
  - 支持功能码：03H、06H、10H，提供完整的寄存器读写与异常处理。
  - 寄存器映射：按区域划分（状态、运行参数、用户、出厂参数、后备等），统一通过g_mb_Holding数组管理。
- CRC16校验（ModBusCrc16）
  - 查表法实现，提供ModbusCRC16函数，用于数据帧校验与响应帧生成。
- 串口调试接口（usinterface/usFunc）
  - 提供协议切换命令（PRTCL）、参数读写命令（ADDR/BDR/SPD/CNT等），支持AGS与MODBUS协议切换。

章节来源
- [ags_mb.c:181-423](file://SRC/HARDWARE/ags_mb/ags_mb.c#L181-L423)
- [modbus.c:191-366](file://SRC/HARDWARE/modbus/modbus.c#L191-L366)
- [ModBusCrc16.c:62-74](file://SRC/HARDWARE/ags_mb/ModBusCrc16.c#L62-L74)
- [usFunc.c:707-747](file://SRC/HARDWARE/usinterface/usFunc.c#L707-L747)

## 架构总览
下图展示了Modbus协议在系统中的交互关系：串口驱动负责数据收发，协议实现负责帧解析与功能码处理，CRC模块提供校验，调试接口负责协议切换与参数配置。

```mermaid
sequenceDiagram
participant Host as "主机"
participant UART as "USART2/3"
participant AGS as "AGS子集(ags_mb)"
participant STD as "标准Modbus(modbus)"
participant CRC as "CRC16校验"
participant UI as "串口调试(usFunc)"
Host->>UART : "发送Modbus请求帧"
UART->>AGS : "接收数据回调"
UART->>STD : "接收数据回调"
AGS->>CRC : "校验请求帧"
STD->>CRC : "校验请求帧"
CRC-->>AGS : "校验结果"
CRC-->>STD : "校验结果"
AGS->>AGS : "解析功能码与参数"
STD->>STD : "解析功能码与参数"
AGS-->>UART : "发送响应帧"
STD-->>UART : "发送响应帧"
UI->>AGS : "协议切换/参数配置命令"
UI->>STD : "协议切换/参数配置命令"
```

图表来源
- [ags_mb.c:426-473](file://SRC/HARDWARE/ags_mb/ags_mb.c#L426-L473)
- [modbus.c:469-517](file://SRC/HARDWARE/modbus/modbus.c#L469-L517)
- [ModBusCrc16.c:62-74](file://SRC/HARDWARE/ags_mb/ModBusCrc16.c#L62-L74)
- [usFunc.c:707-747](file://SRC/HARDWARE/usinterface/usFunc.c#L707-L747)

## 详细组件分析

### AGS风格Modbus子集（ags_mb）
- 功能码支持
  - 读保持寄存器(03H)：支持多种子操作（读状态、当前通道、地址、版本、波特率、序列号、速度、切换次数、回复方式、半通道、通道数等），按op_addr区分。
  - 写单个保持寄存器(06H)：支持写通道、地址、复位、波特率、序列号、速度、切换次数、回复方式、半通道、通道数等。
  - 扩展功能(0x52)：预留扩展用途。
- 数据帧与CRC
  - 请求帧：设备地址 + 功能码 + 操作码/地址 + 数据 + CRC16。
  - 响应帧：设备地址 + 功能码 + 数据长度/数据 + CRC16。
  - CRC16采用查表法实现，确保实时性与准确性。
- 错误处理
  - 非法功能码、非法地址、非法数据值、设备故障、从设备忙、非法从站地址等异常码处理。
- 寄存器映射与地址分配
  - 线圈位、离散输入、保持寄存器、输入寄存器四类，分别定义长度与起始地址，便于HMI与上位机对接。
- 通信参数与时序
  - 支持9600/19200/38400波特率，基于每个bit时间计算帧间隔与超时阈值，保证RTU时序要求。

```mermaid
flowchart TD
Start(["接收数据"]) --> Parse["解析帧头与功能码"]
Parse --> CRC["CRC校验"]
CRC --> Valid{"校验通过?"}
Valid --> |否| Err["设置设备故障错误"]
Valid --> |是| FC{"功能码"}
FC --> |03H| ReadHolding["读保持寄存器处理"]
FC --> |06H| WriteSingle["写单个保持寄存器处理"]
FC --> |其他| ErrFun["设置非法功能错误"]
ReadHolding --> Send["发送响应帧"]
WriteSingle --> Send
Err --> SendErr["发送异常响应"]
ErrFun --> SendErr
Send --> End(["结束"])
SendErr --> End
```

图表来源
- [ags_mb.c:426-473](file://SRC/HARDWARE/ags_mb/ags_mb.c#L426-L473)
- [ags_mb.c:181-285](file://SRC/HARDWARE/ags_mb/ags_mb.c#L181-L285)
- [ags_mb.c:287-423](file://SRC/HARDWARE/ags_mb/ags_mb.c#L287-L423)

章节来源
- [ags_mb.h:107-163](file://SRC/HARDWARE/ags_mb/ags_mb.h#L107-L163)
- [ags_mb.c:181-423](file://SRC/HARDWARE/ags_mb/ags_mb.c#L181-L423)
- [ModBusCrc16.c:62-74](file://SRC/HARDWARE/ags_mb/ModBusCrc16.c#L62-L74)

### 标准Modbus实现（modbus）
- 功能码支持
  - 03H：读保持寄存器，支持批量读取，返回字节数为寄存器数量×2。
  - 06H：写单个保持寄存器，写入成功后返回原始请求帧（回显）。
  - 10H：写多个保持寄存器，支持批量写入，校验字节数与寄存器数量一致性。
- 寄存器映射
  - 区域划分：状态寄存器、运行参数、用户寄存器、出厂参数、后备寄存器等，每个区域包含若干寄存器地址，通过宏定义集中管理。
  - 读保持：根据寄存器地址动态更新g_mb_Holding数组，确保读取最新值。
  - 写保持：根据寄存器地址执行对应动作（如切换通道、设置速度、写入序列号等）。
- CRC与异常处理
  - 使用CRC16校验请求帧，校验失败设置设备错误；异常码通过标准Modbus异常响应格式返回。
- 通信参数与时序
  - 与AGS实现一致的波特率与时序参数，保证RTU帧边界与字符间隔满足规范。

```mermaid
sequenceDiagram
participant Host as "主机"
participant UART as "USART2/3"
participant MOD as "标准Modbus"
participant REG as "g_mb_Holding"
participant CRC as "CRC16校验"
Host->>UART : "发送03H请求"
UART->>MOD : "接收数据"
MOD->>CRC : "校验请求帧"
CRC-->>MOD : "校验结果"
MOD->>REG : "读取寄存器值"
REG-->>MOD : "返回寄存器值"
MOD-->>UART : "发送03H响应"
Host->>UART : "发送06H请求"
UART->>MOD : "接收数据"
MOD->>CRC : "校验请求帧"
CRC-->>MOD : "校验结果"
MOD->>REG : "写入寄存器值"
MOD-->>UART : "发送06H响应"
```

图表来源
- [modbus.c:191-366](file://SRC/HARDWARE/modbus/modbus.c#L191-L366)
- [modbus.c:469-517](file://SRC/HARDWARE/modbus/modbus.c#L469-L517)
- [modbus.h:71-198](file://SRC/HARDWARE/modbus/modbus.h#L71-L198)

章节来源
- [modbus.h:1-213](file://SRC/HARDWARE/modbus/modbus.h#L1-L213)
- [modbus.c:191-366](file://SRC/HARDWARE/modbus/modbus.c#L191-L366)
- [modbus.c:523-765](file://SRC/HARDWARE/modbus/modbus.c#L523-L765)

### CRC16校验算法
- 实现方式
  - 查表法：预置高低位查表，逐字节迭代计算，最终组合为16位CRC值。
- 应用场景
  - 请求帧校验：接收端在处理前先校验CRC，失败则设置设备错误。
  - 响应帧生成：构造响应帧后追加CRC16，确保接收端验证通过。
- 性能与可靠性
  - 查表法在嵌入式环境下具有较低CPU开销，适合实时性要求高的RTU通信。

章节来源
- [ModBusCrc16.c:2-74](file://SRC/HARDWARE/ags_mb/ModBusCrc16.c#L2-L74)

### 协议切换与AGS兼容
- 协议切换
  - 通过串口命令TermProtocal（PRTCL）在AGS、HX、MODBUS三种协议间切换，切换后写入EEPROM并生效。
- AGS兼容性
  - AGS风格实现保留了AGS协议的常用读写操作，同时兼容Modbus标准功能码（03H/06H/10H）。
  - 通过统一的CRC与帧格式，保证与主流Modbus工具链的互通。

章节来源
- [usFunc.c:707-747](file://SRC/HARDWARE/usinterface/usFunc.c#L707-L747)
- [ags_mb.c:181-423](file://SRC/HARDWARE/ags_mb/ags_mb.c#L181-L423)
- [modbus.c:191-366](file://SRC/HARDWARE/modbus/modbus.c#L191-L366)

## 依赖关系分析
- 组件耦合
  - ags_mb与ModBusCrc16强耦合（CRC16依赖），modbus与CRC16弱耦合（通过公共函数调用）。
  - 两个Modbus实现相互独立，互不依赖，便于按需选择。
- 外部依赖
  - 串口驱动（USART2/3）与定时器（TIM3）用于波特率与帧间隔控制。
  - EEPROM接口用于参数持久化（地址、波特率、速度、序列号、切换次数、回复方式、半通道、通道数等）。
- 协议接口
  - 串口调试接口提供统一的命令入口，支持协议切换与参数配置，降低集成复杂度。

```mermaid
graph LR
CRC["CRC16(查表法)"] --> AGS["AGS子集(ags_mb)"]
CRC --> MOD["标准Modbus(modbus)"]
USART2["USART2"] --> AGS
USART3["USART3"] --> AGS
USART2 --> MOD
USART3 --> MOD
TIM3["TIM3"] --> AGS
TIM3 --> MOD
UI["串口调试(usFunc)"] --> AGS
UI --> MOD
```

图表来源
- [ModBusCrc16.c:62-74](file://SRC/HARDWARE/ags_mb/ModBusCrc16.c#L62-L74)
- [ags_mb.c:7-473](file://SRC/HARDWARE/ags_mb/ags_mb.c#L7-L473)
- [modbus.c:35-67](file://SRC/HARDWARE/modbus/modbus.c#L35-L67)
- [usFunc.c:707-747](file://SRC/HARDWARE/usinterface/usFunc.c#L707-L747)

章节来源
- [usInterface.c:15-106](file://SRC/HARDWARE/usinterface/usInterface.c#L15-L106)
- [usFunc.c:707-747](file://SRC/HARDWARE/usinterface/usFunc.c#L707-L747)

## 性能考量
- CRC16查表法在嵌入式环境具备良好性能，适合高频通信场景。
- 波特率与时序参数经过优化，满足Modbus RTU字符间隔与帧间隔要求。
- 通过最小接收计数与超时检测，避免无效帧占用资源。
- 建议在高负载场景下：
  - 合理设置帧间隔与超时阈值，避免误判。
  - 对频繁写入的寄存器进行缓存与批量处理，减少EEPROM写入次数。

## 故障排查指南
- CRC校验失败
  - 现象：接收端设置设备错误并丢弃帧。
  - 排查：检查波特率设置、线缆与终端电阻、噪声干扰；确认CRC生成与校验逻辑一致。
- 非法功能码/地址/数据
  - 现象：异常响应帧返回对应异常码。
  - 排查：核对功能码与寄存器地址范围；检查数据长度与字节数是否匹配。
- 从设备忙
  - 现象：写操作返回忙异常。
  - 排查：检查设备状态机，避免在切换过程中接受写请求；必要时增加重试策略。
- 波特率不匹配
  - 现象：通信时断时续或数据乱码。
  - 排查：确认主从两端波特率一致；检查定时器初始化参数。
- 协议切换无效
  - 现象：切换命令执行后仍无法通信。
  - 排查：确认EEPROM写入成功；重启设备使新协议生效；检查命令格式与参数范围。

章节来源
- [ags_mb.c:159-179](file://SRC/HARDWARE/ags_mb/ags_mb.c#L159-L179)
- [modbus.c:167-186](file://SRC/HARDWARE/modbus/modbus.c#L167-L186)

## 结论
本项目提供了两套Modbus实现：AGS风格子集与标准Modbus，既满足开关器业务的轻量化需求，又兼容主流Modbus生态。通过CRC16查表法、完善的错误处理与协议切换机制，系统在可靠性与可维护性方面表现良好。建议在实际部署中结合具体应用场景选择合适的协议实现，并遵循Modbus RTU规范进行参数配置与调试。

## 附录
- 数据帧格式（示例）
  - 请求帧：设备地址 + 功能码 + 寄存器地址/数据 + CRC16
  - 响应帧：设备地址 + 功能码 + 数据长度/数据 + CRC16
- 寄存器映射要点
  - AGS子集：线圈位、离散输入、保持寄存器、输入寄存器四类，按宏定义分配。
  - 标准Modbus：按区域划分，统一通过g_mb_Holding数组管理，支持批量读写。
- 调试命令参考
  - PRTCL：切换协议类型（AGS/HX/MODBUS）
  - ADDR/BDR/SPD/CNT等：读写设备参数，便于现场配置与诊断。