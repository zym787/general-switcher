#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef _MAIN_GLOBALS_
#define PEXT
#else
#define PEXT extern
#endif

// clang-format off

#define DESCRIPTION         "Switch Valve"
#define SOFT_REVISION       (uint16_t)0x0029    /* 软件修改版次 */
#define SOFTWARE_VERSION    "r29"                /* 软件修改版次 */
#ifdef RS232_485_CONTROL
  #define CONTROL     "Only 232/485 AGS"
  #ifndef C_901
    #ifdef AGING_MODE
        #define SOFT_NAME   "vA.A.1"
        #define SOFT_VER_NUM    (uint32_t)0xAA100000
    #else
        #define SOFT_NAME   "v1.3.1"
        #define SOFT_VER_NUM    (uint32_t)0x13100000
    #endif
  #else
    #define SOFT_NAME   "v1.3.1C"
    #define SOFT_VER_NUM    (uint32_t)0x131C0000
  #endif
#else
  #if IO_RS  // IO_RS 1 A IO
    #define CONTROL     "IO 232/485"
    #define SOFT_NAME   "v1.3.1A"
    #define SOFT_VER_NUM    (uint32_t)0x131A0000    /* A 232/485/IO */
  #else      // IO_RS 0 B IO
    #define CONTROL     "IO 232/485"
    #define SOFT_NAME   "v1.3.1B"
    #define SOFT_VER_NUM    (uint32_t)0x131B0000    /* B IO */
  #endif
#endif
#define BOARD_0     0x88
#define BOARD_1     0x66
#define SOFT_VER    (SOFT_VER_NUM + SOFT_REVISION)
#define SOFT_VER_C  SOFT_NAME##"-"##SOFTWARE_VERSION
//  V1.2.9r7        2024.09.26  原点补偿做减速区间 (TZY)
//  V1.3.0r0        2025.01.14  修改通信丢包 (TZY)
//  V1.3.0r1        2025.03.05  修改序列号地址重复 (TZY)
//  V1.3.0r2        2025.05.19  应用方向补偿 (TZY)
//  V1.3.0r3        2025.05.21  修改通道数限制
//  V1.3.0r4        2025.06.20  修复读序列号
//  V1.3.1A/B       2025.06.26  统一IO  宏定义IO_RS 1-A(232/485/IO) 0-B(IO)
//                              |- A    1.3.0r4 232/485+IO  所有都支持
//                              |- B    1.2.9r7 IO          仅支持IO
//  V1.3.2A/B       2025.06.30  修复超时保护
//  V1.3.3A/B       2025.06.30  修复初始化降速,增加版本输出
//  V1.3.1A/B       2025.07.10  修复超时保护,修复默认参数,修复LED闪烁,重置版本号
//  v1.3.1A/B-r1    2025.07.21  支持0地址,修复默认参数写入乱码,支持03读版本,修改版本号
//                              新增系统可操作寄存器地址映射表
//  v1.3.1A/B-r2    2025.07.22  分离软件版本号中的修改版次和版本名称
//                              修复
//  v1.3.1A/B-r1    2025.07.29  分离版本号中可变和不可变部分,去除寄存器映射表
//                              下载口新增MOVES查询/写入切换次数指令(0-2^32)
//                              AGS: 删除读写补偿指令,增加写功能时长度匹配,写速度范围限制在20-200
//                              优化协议栈,定义异常码,使用宏优化语句
//  v1.3.1A/B-r2    2025.07.30  修复09读写速度错误,速度修改为1个字节范围20-200
//                              修复默认减速比错误并设置为4,IO、半通道默认关闭,最大通道数限制为16
//                              序列号、切换次数必须手动清空,无法使用IIC清空
//  v1.3.1A/B-r3    2025.07.31  对地址(0-63)、波特率(1-3)、速度(20-200)、通道数(3-16)进行限制
//                              修复无法使用广播地址AA等地址问题
//  v1.3.1A/B-r4    2025.07.31  修复默认电流设置问题,优化注释等文本
//                  2025.08.01  修复下载口设置速度失败
//                              新增BAUD,SPD,INT读功能
//                              下载口参数增加范围限制ADDR(0-63),CNT(3-16),POS(1-最大通道)
//                              BAUD(1-3),SPD(20-200),INT(0-255),ISET(0-4),RDCR(1,4,10,16),HALF(0,1)
//  v1.3.1A/B-r5    2025.08.01  修复老化地址64支持,修复POS切换时状态限制
//  v1.3.1A/B-r6    2025.08.04  修复老化地址64bug,AGS无法在老化地址06写操作但可以03读操作
//  v1.3.1AB-r7     2025.08.05  修复开启IO在B位置无法就近复位的情况
//                              新增调试口切换时间输出,添加调试信息输出开关
//  v1.3.1AB-r8     2025.08.07  修复在A状态复位报错问题,修复下载口RST复位
//                              修复半通道下无法就近复位情况,优化LED闪烁条件
//  v1.3.1AB-r9     2025.08.12  修复切换次数写入不及时问题,修复99读通道数错误,使用宏开关定义不同版本
//  v1.3.1AB-r10    2025.08.21  新增20减速比,20减速比速度限制在1-50,正常速度限制1-200
//                              下载口设置地址,通道数,波特率,速度,减速比,半通道增加默认值写入
//                              修复下载口读波特率错误
//  v1.3.1AB-r11    2025.09.04  输出三种版本程序,默认波特率修改为9600,
//                                1.3.1   232/485版本屏蔽IO,仅支持串口
//                                1.3.1AB 默认打开IO,区分不同电平标准
//  v1.3.1AB-r12    2025.09.05  增加下载口点检指令INSP,会打印出所有参数
//  v1.3.1-r13      2025.09.05  修复20减速比支持
//  v1.3.1-r14      2025.11.13  新增0B回复方式设置,移动指令可以屏蔽回复
//                              汉化部分指令,点检模式支持中文,支持中文显示所有指令
//  v1.3.1-r15      2025.11.15  新增灯状态输出 C版本
//                              A:IN 1  OUT 0,B:IN 0  OUT 1,
//                              M:IN 0  OUT 0,ERROR:IN 1 OUT 1
//  v1.3.1-r16      2025.11.20  仅新增电机老化模式,仅支持正反转,版本vA.A.1
//                              使用dbg_printf替换调试输出,电机老化模式放开速度限制
//  v1.3.1-r17      2025.12.03  仅修复C版本IO IN无法输出错误
//  v1.3.1-r18      2025.12.03  仅降低LED优先级，提高通信性能
//  v1.3.1-r19      2025.12.04  仅修改 LED输出检测210ms执行一次
//  v1.3.1-r20      2026.03.04  调整906方向,点检指令增加波特率值显示
//  v1.3.1-r21      2026.03.04  增加DIRECTION_SWITCH宏调整906方向
//  v1.3.1-r22      2026.03.09  串口新增0D功能码读写半通道功能
//  v1.3.1-r23      2026.03.23  AGS增加读指令长度限制
//  v1.3.1-r24      2026.03.26  屏蔽AGS增加读指令长度限制
//  v1.3.1-r25      2026.03.26  优化AGS读指令长度限制,减少冗余
//  v1.3.1-r26      2026.03.27  修复错误长度读指令使设备死机问题
//                  2026.04.09  统一波特率显示,优化下载口显示及点检模式
//  v1.3.1-r27      2026.04.22  支援modbus,修复485串口无法接收,修复下载口读电流错误,优化代码
//  v1.3.1-r28      2026.04.27  修复modbus复位功能错误,优化modbus性能
//  v1.3.1-r29      2026.04.28  修复modbus地址功能错误,modbus参数增加范围限制


#ifdef A12_901
    #define IO_OUT          PAout(8)
    #ifndef C_901
        #define IO_IN           PBin(3)
    #else
        #define IO_IN           PBout(3)
    #endif
#endif
#ifdef A12_909
    #define IO_OUT          PBout(13)
    #define IO_IN           PBin(5)
#endif
#ifdef A12_906
    #define IO_OUT          PBout(13)
    #define IO_IN           PBin(14)
#endif

//----EEPROM存储地址分配---//
//------------------------------------------------------------------------------------------------------------
#define ADDR_BOARD_ID           0
#define LEN_BOARD_ID            2

#define ADDR_MODULE_NUM         (ADDR_BOARD_ID+LEN_BOARD_ID)
#define LEN_MODULE_NUM          1

#define ADDR_VALVE_FIX          (ADDR_MODULE_NUM+LEN_MODULE_NUM)
#define LEN_VALVE_FIX           1

#define ADDR_DIR_FIX            (ADDR_VALVE_FIX+LEN_VALVE_FIX)
#define LEN_DIR_FIX             1

#define ADDR_PORT_CNT           (ADDR_DIR_FIX+LEN_DIR_FIX)
#define LEN_PORT_CNT            1

#define ADDR_BAUD               (ADDR_PORT_CNT+LEN_PORT_CNT)
#define LEN_BAUD                1

#define ADDR_SPD                (ADDR_BAUD+LEN_BAUD)
#define LEN_SPD                 1

#define ADDR_NOW_POS            (ADDR_SPD+LEN_SPD)
#define LEN_NOW_POS             1

#define ADDR_IO_CTRL            (ADDR_NOW_POS+LEN_NOW_POS)
#define LEN_IO_CTRL             1

#define ADDR_INTVL              (ADDR_IO_CTRL+LEN_IO_CTRL)
#define LEN_INTVL               1

#define ADDR_ISET               (ADDR_INTVL+LEN_INTVL)
#define LEN_ISET                1

#define ADDR_SN                 (ADDR_ISET+LEN_ISET)
#define LEN_SN                  5

#define ADDR_RDC_RATE			(ADDR_SN+LEN_SN)
#define LEN_RDC_RATE			1

#define ADDR_HALF_SEAL			(ADDR_RDC_RATE+LEN_RDC_RATE)
#define LEN_HALF_SEAL			1

#define ADDR_TOTAL_CNT        	(ADDR_HALF_SEAL+LEN_HALF_SEAL)
#define LEN_TOTAL_CNT           4

#define ADDR_REPLY_MODE        	(ADDR_TOTAL_CNT+LEN_TOTAL_CNT)
#define LEN_REPLY_MODE          1

#define ADDR_INIT_STATE        	(ADDR_REPLY_MODE+LEN_REPLY_MODE)
#define LEN_INIT_STATE          1

#define ADDR_PROTOCOL             (ADDR_INIT_STATE+LEN_INIT_STATE)
#define LEN_PROTOCOL             1

#define ADDR_BURN_CNT             (ADDR_PROTOCOL+LEN_PROTOCOL)
#define LEN_BURN_CNT             4

#define ADDR_GOD_MODE             (ADDR_BURN_CNT+LEN_BURN_CNT)
#define LEN_GOD_MODE             1

// clang-format on

#define NORMAL_BLINK            1500       // 正常运行的闪烁间隔
#define RETRY_TIME_OUT          1100       // 重试的闪烁间隔
#define ERROR_BLINK             400

PEXT uint8_t bdrate, bIoCtrl, intCtrl, spdVx2;

/**
 * @brief     : 控制协议枚举
 */
typedef enum PROTOCOL {
        AGS_MODBUS, /* AGS协议 基于Modbus魔改 */
        EXT_COMM,   /* HX协议 帧头+帧尾 */
        MODBUS,     /* Modbus协议 */

        PROTOCOL_NUM
} Protocol_T;

/**
 * @brief     : 波特率枚举
 */
typedef enum BAUDRATETYPE {
        BAUD_NONE = 0u,
        BAUD_9600 = 1u,
        BAUD_19200 = 2u,
        BAUD_38400 = 3u,

        BAUD_NUM
} BaudRate_T;

typedef enum GODMODE {
        GD_NORMAL = 0u,
        GD_FACTORY = 1u,
        GD_AGING = 2u,
} GodMode_T;

extern uint16_t BaudRate_V[BAUD_NUM];
extern uint16_t BaudRate_Time[BAUD_NUM];

typedef struct {                  /* 系统参数 */
        Protocol_T protocol_type; /* 协议 */
        BaudRate_T baudrate;      /* 波特率 */
        bool bCountLastTime;      /* 切换时间计时标志 */
        uint32_t totalCnt;        /* 切换次数 */
        uint32_t totalCntLst;
        uint32_t burnCnt;        /* 烧机次数 */
        uint16_t lastTime;       /* 上次切换时间 */
        uint32_t protectTimeOut; /* 超时保护时间 */
        uint8_t replyMode;       /* 回复方式 */
        GodMode_T GodMode;       /* 当前模式 */
} _SYS_T;
PEXT _SYS_T syspara;

PEXT int main(void);
PEXT void DebugOut(void);
PEXT void dlyInTimer(void);
PEXT uint8_t IntDly(uint16_t intMs);
PEXT void EnableReceive(void);
PEXT void DisableReceive(void);
PEXT void ErrBlink(void);
PEXT void ee_ReadTest(void);
#undef PEXT
#endif
