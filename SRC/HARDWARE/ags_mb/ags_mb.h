#ifndef _AGS_MB_H_
#define _AGS_MB_H_

#ifdef _MODBUS_GLOBALS_
#define PEXT
#else
#define PEXT extern
#endif

// #define DEBUG_AGS_MB    /* AGS */

//======================  参数宏定义=================================
#define MAX_ADDR                    1
#define	MB_SALVE_DEF_ADDR           1
//---- 本机支持MODBUS功能代码-------------------------------------------------
#define CHECK_COIL_bSTATUS			1 		//  读输出位状态
#define GET_INPUT_bSTATUS			2 		//  读输入位状态
#define GET_HOLDING_REGT			3		//  读保持寄存器
#define GET_INPUT_REGT				4		//  读输入寄存器
#define SET_COIL_SINGLE				5		//  强置单线圈	 
#define PRESET_HOLDING_sREGT		6		//  预置单个保持寄存器
#define SET_COIL_MULTIPLE			15		//  强制多个位输出
#define PRESET_HOLDING_mREGT		16		//  预置多个保持寄存器
#define EXTERN_FUNC			        0x52    //  扩展功能
/* 异常码 */
#define ERR_NOT						0x00    /* 00 无异常码 成功 */
#define ERR_MB_FUN					0x01    /* 01 非法功能异常码 */
#define ERR_MB_ADDR					0x02    /* 02 非法数据地址 */
#define ERR_MB_DATA					0x03    /* 03 非法数据值,超出某功能的极限值 */
#define ERR_MB_DEVICE				0x04    /* 04 从站设备故障 主模式响应超出等待时间；从模式设备地址出错；包括字符间隔时间 超过1.5个字符 */
#define ERR_MB_AFFIRM               0x05    /* 05 从设备确认 */
#define ERR_MB_BUSY                 0x06    /* 06 从设备忙 */
#define ERR_MB_DEVICE_ADDR          0x0E    /* 0E 非法从站设备地址 */
//---- 通信时间参数-------------------------------------------------
#define ONE_DATA_BIT				10		// 采用方式1:10bit
#define BAUD_RATE_9600			    9600L	// MODBUS波特列
#define BAUD_RATE_19200			    19200L	// MODBUS波特列
#define BAUD_RATE_38400			    38400L	// MODBUS波特列
//每个bit的时间为1/BR,每个字节为8位+停止位+校验位=10位,则每个字节耗时10/BR,半字节为5/BR,转般为us乘以1000000
#define MODBUS_TIME_9600			(520)		// 520uS=(10/9600)/2
#define MODBUS_TIME_19200			(260)		// 260uS=(10/19200)/2
#define MODBUS_TIME_38400			(130)		// 130uS=(10/38400)/2
#define BUS_IDLE_TIME				10		// 帧空闲(开始或结束)超过3.5~4个接收字符时间
#define FRAME_ERR_TIME				4		// 数据接收超时,接收字符之间 超过1.5~2个接收字符时间
#define NO_RESPONSE_TIME			500		// 通讯中断超时 约500*MODBUS_TIME
#define NO_RESPONSE_TIME_BC			500		// 通讯中断超时 约500*MODBUS_TIME
//---- 通信状态参数-------------------------------------------------
#define MB_IDLE						0X00	// 总线空闲、结束帧、起始帧
#define MB_WAIT_HOST				0X01	// 主设备等待响应
#define MB_BUSY						0X50	// 总线忙
#define MB_RECIVE					0X51	// 总线接收
#define MB_SEND						0X52	// 总线发送
#define MB_RECIVE_END				0X53	// 总线发送
#define MB_NO_RESPONSE				0X54	// 主模式响应的当前查询的设备,从模式设备地址出错
#define MB_RECIVE_ERR				0X55	// 总线接收过程有出错
//---- 通信地址参数,此处少改?------------------------------------------------
#define MB_Broadcast_ADDR			0xAA	// 广播地址

#define MB_MAX_ADDR					128		// 模块最大地址

#define MB_REPEAT					2		// 数据传输出错,后重新处理的次数
#define MB_ADDR_ERR_FLAG			0xFF	// 地址错误,无效标记
#define MB_ADDR_NULL				0xFF	// 模块不存在

//=====================函数和变量声明=============================
//协议传输变量
#define LENGTH_MB_DATA				140	    // 11*12 +4*2
#define LENGTH_MB_PARA				288	    // 140*2+8
#define LEAST_RCV_CNT               3       // 最小接收字节数
typedef struct
{
    uint8_t     sRUN;					// 总线运行状态
    uint8_t     sERR;					// 总线出错状态
    uint8_t     rCnt;					// 总线接收计数
    uint8_t     HostRept;				// 作为主设备的重复次数
    uint16_t    times;					// 总线计时1
    uint8_t     mAddrs;					// 设备地址
    uint8_t     res;				    // 保留---凑2的倍数
    uint8_t     rBuf[LENGTH_MB_DATA];	// 待解析的接收数据
    uint8_t     tBuf[LENGTH_MB_DATA];	// 待发送数据
} _AGS_MB_PRARM_T;

/* 波特率 */
enum {
  UART_BAUD_DEF = 0U,   /**< Default 9600bps */
  UART_BAUD_9600 = 1U,  /**< 9600bps */
  UART_BAUD_19200 = 2U, /**< 19200bps */
  UART_BAUD_38400 = 3U, /**< 38400bps */
};

//线圈变量
#define SUM_COIL_BIT				64		// 线圈的位个数
#define LENGTH_COIL_REG			    SUM_COIL_BIT/8

//离散量输入变量
#define SUM_DiscreteREG_BIT			32		// 离散量的位个数
#define LENGTH_DiscreteREG			SUM_DiscreteREG_BIT	/8

//保持寄存器变量
#define SUM_HoldingREG_WORD		    120		// 保持寄存器的个数
#define LENGTH_HoldingREG			SUM_HoldingREG_WORD*2

//输入寄存器变量
#define SUM_InputREG_WORD			20		// 保持寄存器的个数
#define LENGTH_InputREG				SUM_HoldingREG_WORD*2

//用户定义各个变量、寄存器的使用功能
#define COIL_NUM01				    0
#define DisREG_NUM01			    (COIL_NUM01 + SUM_COIL_BIT)
#define HoldingREG01				0
#define InputREG01				    (HoldingREG01 + SUM_HoldingREG_WORD)

//HMI的通讯字地址分配
// 保持寄存器
#define 	HMI_hREG01_BasePic			0							// 画面地址
#define 	HMI_hREG02_CMD				1							// 控制命令地址

// 输入寄存器

// HMI的通讯位地址分配
// 线圈变量: 0~15it
#define	HMI_COIL_BIT0		0x01	// 
#define	HMI_COIL_BIT1		0x02	// 
#define	HMI_COIL_BIT2		0x04	// 
#define	HMI_COIL_BIT3		0x08	// 
#define	HMI_COIL_BIT4		0x10	// 
#define	HMI_COIL_BIT5		0x20	// 
#define	HMI_COIL_BIT6		0x40	// 
#define	HMI_COIL_BIT7		0x80	// 

//离散输入变量:16~71 bit : 16~23 [0]  24`31[1] 32~39 [2] 40~47 [3] 48~55 [4] 56~63 [5] 64~71 [6]
#define	HMI_DisIN_BIT0		0x01	// 
#define	HMI_DisIN_BIT1		0x02	// 
#define	HMI_DisIN_BIT2		0x04	// 
#define	HMI_DisIN_BIT3		0x08	// 
#define	HMI_DisIN_BIT4		0x10	// 
#define	HMI_DisIN_BIT5		0x20	// 
#define	HMI_DisIN_BIT6		0x40	// 
#define	HMI_DisIN_BIT7		0x80	// 

//----变量声明----------------
PEXT _AGS_MB_PRARM_T	    ags_mbParam;
PEXT uint8_t 	CoilBitPara[LENGTH_COIL_REG];
PEXT uint8_t 	DiscreteBitPara[LENGTH_DiscreteREG];
PEXT uint8_t 	HoldingREGPara[LENGTH_HoldingREG];
PEXT uint8_t 	InputREGPara[LENGTH_InputREG];

//----函数声明----------------
PEXT uint16_t  ModbusCRC16( uint8_t  *puchMsg, uint16_t usDataLen) ;
PEXT void ags_mbInit(void);
PEXT void ags_mbSend(uint8_t length) ;
PEXT void ags_mbReceive(uint8_t res);
PEXT void ags_mbTimesProcess(void);
PEXT void ags_mbError(void) ;
PEXT void ags_mbReadHoldingRegisters(void) ;
PEXT void ags_mbPresetSingleHoldingRegister(void);
PEXT void MB_PresetMultipleHoldingRegisters(void);
PEXT void ags_mbProcess(void);
PEXT void ModbusRequest(uint8_t style, uint8_t sAdd,uint16_t val);

#undef PEXT
#endif
