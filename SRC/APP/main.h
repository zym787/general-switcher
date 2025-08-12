#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef _MAIN_GLOBALS_
#define PEXT
#else
#define PEXT extern
#endif

#define DESCRIPTION         "Switch Valve"
#define SOFT_REVISION       (uint16_t)0x0009    /* 软件修改版次 */
#define SOFTWARE_VERSION    "r9"                /* 软件修改版次 */
#if IO_RS  // IO_RS 1 A 232/485/IO
#define CONTROL     "232/485+IO AGS"
#define SOFT_NAME   "v1.3.1A"
#define SOFT_VER_NUM    (uint32_t)0x131A0000    /* A 232/485/IO */
#else      // IO_RS 0 B IO
#define CONTROL     "Only IO Control"
#define SOFT_NAME   "v1.3.1B"
#define SOFT_VER_NUM    (uint32_t)0x131B0000    /* B IO */
#endif
#define BOARD_0     0x88
#define BOARD_1     0x66
#define SOFT_VER    (SOFT_VER_NUM + SOFT_REVISION)
#define SOFT_VER_C  SOFT_NAME##"-"##SOFTWARE_VERSION
//  V1.2.9r7     2024.09.26 原点补偿做减速区间 (TZY)
//  V1.3.0r0     2025.01.14 修改通信丢包 (TZY)
//  V1.3.0r1     2025.03.05 修改序列号地址重复 (TZY)
//  V1.3.0r2     2025.05.19 应用方向补偿 (TZY)
//  V1.3.0r3     2025.05.21 修改通道数限制
//  V1.3.0r4     2025.06.20 修复读序列号
//  V1.3.1A/B    2025.06.26 统一IO  宏定义IO_RS 1-A(232/485/IO) 0-B(IO)
//                          |- A    1.3.0r4 232/485+IO  所有都支持
//                          |- B    1.2.9r7 IO          仅支持IO
//  V1.3.2A/B    2025.06.30 修复超时保护
//  V1.3.3A/B    2025.06.30 修复初始化降速,增加版本输出
//  V1.3.1A/B    2025.07.10 修复超时保护,修复默认参数,修复LED闪烁,重置版本号
//  v1.3.1A/B-r1 2025.07.21 支持0地址,修复默认参数写入乱码,支持03读版本,修改版本号
//                          新增系统可操作寄存器地址映射表
//  v1.3.1A/B-r2 2025.07.22 分离软件版本号中的修改版次和版本名称
//                          修复
//  v1.3.1A/B-r1 2025.07.29 分离版本号中可变和不可变部分,去除寄存器映射表
//                          下载口新增MOVES查询/写入切换次数指令(0-2^32)
//                          AGS: 删除读写补偿指令,增加写功能时长度匹配,写速度范围限制在20-200
//                               优化协议栈,定义异常码,使用宏优化语句
//  v1.3.1A/B-r2 2025.07.30 修复09读写速度错误,速度修改为1个字节范围20-200
//                          修复默认减速比错误并设置为4,IO、半通道默认关闭,最大通道数限制为16
//                          序列号、切换次数必须手动清空,无法使用IIC清空
//  v1.3.1A/B-r3 2025.07.31 对地址(0-63)、波特率(1-3)、速度(20-200)、通道数(3-16)进行限制
//                          修复无法使用广播地址AA等地址问题
//  v1.3.1A/B-r4 2025.07.31 修复默认电流设置问题
//                          优化注释等文本
//              2025.08.01  修复下载口设置速度失败
//                          新增BAUD,SPD,INT读功能
//                          下载口参数增加范围限制  ADDR(0-63),CNT(3-16),POS(1-最大通道)
//                                                  BAUD(1-3),SPD(20-200),INT(0-255)
//                                                  ISET(0-4),RDCR(1,4,10,16),HALF(0,1)
//  v1.3.1A/B-r5 2025.08.01 修复老化地址64支持,修复POS切换时状态限制
//  v1.3.1A/B-r6 2025.08.04 修复老化地址64bug,AGS无法在老化地址06写操作但可以03读操作
//  v1.3.1AB-r7 2025.08.05  修复开启IO在B位置无法就近复位的情况
//                          新增调试口切换时间输出,添加调试信息输出开关
//  v1.3.1AB-r8 2025.08.07  修复在A状态复位报错问题,修复下载口RST复位
//                          修复半通道下无法就近复位情况,优化LED闪烁条件
//  v1.3.1AB-r9 2025.08.12  修复切换次数写入不及时问题,修复99读通道数错误,使用宏开关定义不同版本

#ifdef A12_901
#define IO_OUT          PAout(8)
#define IO_IN           PBin(3)
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

#define NORMAL_BLINK            1500       // 正常运行的闪烁间隔
#define RETRY_TIME_OUT          1100       // 重试的闪烁间隔
#define ERROR_BLINK             400

PEXT uint8_t bdrate, bIoCtrl, intCtrl, spdVx2;

typedef struct
{
    bool        bCountLastTime;     /* 切换时间计时标志 */
    uint32_t    totalCnt;           /* 切换次数 */
    uint32_t    totalCntLst;
    uint16_t    lastTime;           /* 上次切换时间 */
    uint32_t    protectTimeOut;     /* 超时保护时间 */
}_SYS_T;
PEXT _SYS_T syspara;

PEXT int main(void);
PEXT void DebugOut(void);
PEXT void dlyInTimer(void);
PEXT unsigned char IntDly(unsigned short intMs);
PEXT void EnableReceive(void);
PEXT void DisableReceive(void);
PEXT void ErrBlink(void);

#undef PEXT
#endif
