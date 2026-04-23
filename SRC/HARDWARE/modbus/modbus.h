#ifndef __MODBUS_H__
#define __MODBUS_H__

/* 包含头文件 ----------------------------------------------------------------*/
// #include "stm32f1xx_hal.h"

/* 类型定义 ------------------------------------------------------------------*/
/**
 * @brief    Modbus错误状态枚举
 */
typedef enum {
        MB_ERROR_NONE = 0u,
        MB_ERROR_FUNC = 1u,
        MB_ERROR_ADDR = 2u,
        MB_ERROR_DATA = 3u,
        MB_ERROR_DEVICE = 4u,
        MB_ERROR_CONFIRM = 5u,
        MB_ERROR_BUSY = 6u,
        MB_ERROR_PARITY = 8u
} MB_ERROR_T;

/**
 * @brief    Modbus结构体
 */
typedef struct {
        __IO uint8_t RunState;    /* 总线运行状态 */
        __IO MB_ERROR_T ErrorState; /* 总线错误状态 */
        __IO uint16_t times;      /* 单帧接收超时时间 */
        __IO uint8_t ReciveCount; /* 接收数据长度 */
        __IO uint8_t Address;     /* 从站地址 */
} Modbus_T;


typedef struct {
        uint16_t IN1;

} REG_VALUE;

/* 宏定义 --------------------------------------------------------------------*/
#define DEBUG_MODBUS          (0)       /* 是否启用调试输出 */
#define SUPPORT_FUN_CODE_01H  (0)       /* 是否启用功能码01H */
#define SUPPORT_FUN_CODE_02H  (0)       /* 是否启用功能码02H */
#define SUPPORT_FUN_CODE_03H  (1)       /* 是否启用功能码03H */
#define SUPPORT_FUN_CODE_04H  (0)       /* 是否启用功能码04H */
#define SUPPORT_FUN_CODE_05H  (0)       /* 是否启用功能码05H */
#define SUPPORT_FUN_CODE_06H  (1)       /* 是否启用功能码06H */
#define SUPPORT_FUN_CODE_10H  (0)       /* 是否启用功能码10H */

#define MB_SLAVEADDR    0x01
#define MB_ALLSLAVEADDR 0x00

#define FUN_CODE_01H 0x01  // 功能码01H
#define FUN_CODE_02H 0x02  // 功能码02H
#define FUN_CODE_03H 0x03  // 功能码03H
#define FUN_CODE_04H 0x04  // 功能码04H
#define FUN_CODE_05H 0x05  // 功能码05H
#define FUN_CODE_06H 0x06  // 功能码06H
#define FUN_CODE_10H 0x10  // 功能码10H

/* 本例程所支持的功能码,需要添加新功能码还需要在.c文件里面添加 */
#define IS_NOT_FUNCODE(code)                                                                                       \
        (!((code == FUN_CODE_01H) || (code == FUN_CODE_02H) || (code == FUN_CODE_03H) || (code == FUN_CODE_04H) || \
           (code == FUN_CODE_05H) || (code == FUN_CODE_06H) || (code == FUN_CODE_10H)))

#define EX_CODE_NONE 0x00  // 异常码 无异常
#define EX_CODE_01H  0x01  // 异常码
#define EX_CODE_02H  0x02  // 异常码
#define EX_CODE_03H  0x03  // 异常码
#define EX_CODE_04H  0x04  // 异常码

/* Modbus寄存器定义 */
/* 寄存器地址定义 */
#define MODBUS_START                0ul
#define MODBUS_NUMBER               100ul
#define MODBUS_LENGTH               255ul

#define NORMAL_CODE   0x22 /* 34 正常模式安全码 */
#define AGING_CODE    0xEA /* 234 老化模式安全码 */
#define SECURITY_CODE 0xFC /* 252 工厂模式安全码 */

/* 保持寄存器 */
extern __IO uint16_t g_mb_Holding[MODBUS_NUMBER];

/* 定义保持寄存器操作接口 */
#define MB_SET_HOLDING(addr, data) (g_mb_Holding[addr] = data)
#define MB_GET_HOLDING(addr) (g_mb_Holding[addr])
//------------------------------------------------------------------------------
/* 控制指令 CTRL */
#define MB_RW_CTRL_SET_NORMAL       0ul  // 设置切换通道
#define MB_RW_CTRL_SET_CW           1ul  // 设置切换通道[顺时针]
#define MB_RW_CTRL_SET_CCW          2ul  // 设置切换通道[逆时针]
#define MB_RW_CTRL_SET_FREE         3ul	 // 自由模式
#define MB_RW_CTRL_REG_4            4ul  //
#define MB_RW_CTRL_SET_ZERO         5ul  // 复位
#define MB_RW_CTRL_REG_6            6ul  //
#define MB_RW_CTRL_REG_7            7ul  //
#define MB_RW_CTRL_REG_8            8ul
#define MB_RW_CTRL_SET_GOD_MODE     9ul  // 设置模式
/* 只读参数 STATUS */
#define MB_R_STATUS_CHANNEL_CUR     10ul  // 当前通道
#define MB_R_STATUS_CONTROL_STATE   11ul  // 控制状态
#define MB_R_STATUS_MOVE_TIME       12ul  // 上次移动耗时
#define MB_R_STATUS_SW_CODE         13ul  // 软件代号
#define MB_R_STATUS_SW_VERSION      14ul  // 软件版本号
#define MB_R_STATUS_COUNT_1         15ul  // 内部计数1
#define MB_R_STATUS_COUNT_2         16ul  // 内部计数2
#define MB_R_STATUS_REG_17          17ul  //
#define MB_R_STATUS_REG_18          18ul  //
#define MB_R_STATUS_REG_19          19ul  //
/* 运行参数1 OPERATE1 */
#define MB_RW_OPERATE1_ADDRESS      20ul  // 地址
#define MB_RW_OPERATE1_SPEED        21ul  // 速度
#define MB_RW_OPERATE1_DIRECTION    22ul  // 方向
#define MB_RW_OPERATE1_BAUDRATE     23ul  // 波特率
#define MB_RW_OPERATE1_MOVE_COUNT_1 24ul  // 移动次数1
#define MB_RW_OPERATE1_MOVE_COUNT_2 25ul  // 移动次数2
#define MB_RW_OPERATE1_REG_26       26ul
#define MB_RW_OPERATE1_REG_27       27ul
#define MB_RW_OPERATE1_REG_28       28ul
#define MB_RW_OPERATE1_REG_29       29ul
/* 运行参数2 OPERATE2 */
#define MB_RW_OPERATE2_REG_30       30ul
#define MB_RW_OPERATE2_REG_31       31ul
#define MB_RW_OPERATE2_REG_32       32ul
#define MB_RW_OPERATE2_REG_33       33ul
#define MB_RW_OPERATE2_REG_34       34ul
#define MB_RW_OPERATE2_REG_35       35ul
#define MB_RW_OPERATE2_REG_36       36ul
#define MB_RW_OPERATE2_REG_37       37ul
#define MB_RW_OPERATE2_REG_38       38ul
#define MB_RW_OPERATE2_REG_39       39ul
/* 序列号 USER */
#define MB_RW_USER_SN_1             40ul
#define MB_RW_USER_SN_2             41ul
#define MB_RW_USER_SN_3             42ul
#define MB_RW_USER_SN_4             43ul
#define MB_RW_USER_SN_5             44ul
#define MB_RW_USER_SN_6             45ul
#define MB_RW_USER_SN_7             46ul
#define MB_RW_USER_SN_8             47ul
#define MB_RW_USER_SN_9             48ul
#define MB_RW_USER_SN_10            49ul
/* 出厂参数1 FACTORY1 */
#define MB_R_FACTORY1_UID_X0       50ul  // 产品唯一ID
#define MB_R_FACTORY1_UID_X1       51ul  // 产品唯一ID
#define MB_R_FACTORY1_UID_Y0       52ul  // 产品唯一ID
#define MB_R_FACTORY1_UID_Y1       53ul  // 产品唯一ID
#define MB_R_FACTORY1_UID_Z0       54ul  // 产品唯一ID
#define MB_R_FACTORY1_UID_Z1       55ul  // 产品唯一ID
#define MB_RW_FACTORY1_DATA_01      56ul  // 内部数据01
#define MB_RW_FACTORY1_DATA_02      57ul  // 内部数据02
#define MB_RW_FACTORY1_DATA_03      58ul  // 内部数据03
#define MB_RW_FACTORY1_DATA_04      59ul  // 内部数据04
/* 出厂参数2 FACTORY2 */
#define MB_RW_FACTORY2_VALVE_TYPE   60ul  // 阀类型
#define MB_RW_FACTORY2_CTRL_MODE    61ul  // 控制模式
#define MB_RW_FACTORY2_CHANNEL_NUM  62ul  // 通道数
#define MB_RW_FACTORY2_HALF_MODE    63ul  // 半通道
#define MB_RW_FACTORY2_REPLY_MODE   64ul  // 回复模式
#define MB_RW_FACTORY2_SECURE_CODE  65ul  // 安全码
#define MB_RW_FACTORY2_COMPEN_ORG   66ul  // 原点补偿值(单位: 1度)
#define MB_RW_FACTORY2_COMPEN_DIR   67ul  // 方向补偿值(单位: 0.1度)
#define MB_RW_FACTORY2_COMPEN_CW    68ul  // 顺时针补偿值(单位: 0.1度)
#define MB_RW_FACTORY2_COMPEN_CCW   69ul  // 逆时针补偿值(单位: 0.1度)
/* 出厂参数3 FACTORY3 */
#define MB_RW_FACTORY3_REG_70       70ul
#define MB_RW_FACTORY3_REG_71       71ul
#define MB_RW_FACTORY3_REG_72       72ul
#define MB_RW_FACTORY3_REG_73       73ul
#define MB_RW_FACTORY3_REG_74       74ul
#define MB_RW_FACTORY3_REG_75       75ul
#define MB_RW_FACTORY3_REG_76       76ul
#define MB_RW_FACTORY3_REG_77       77ul
#define MB_RW_FACTORY3_REG_78       78ul
#define MB_RW_FACTORY3_REG_79       79ul
/* 后备1 BACKUP1 */
#define MB_RW_BACKUP1_REG_80        80ul
#define MB_RW_BACKUP1_REG_81        81ul
#define MB_RW_BACKUP1_REG_82        82ul
#define MB_RW_BACKUP1_REG_83        83ul
#define MB_RW_BACKUP1_REG_84        84ul
#define MB_RW_BACKUP1_REG_85        85ul
#define MB_RW_BACKUP1_REG_86        86ul
#define MB_RW_BACKUP1_REG_87        87ul
#define MB_RW_BACKUP1_REG_88        88ul
#define MB_RW_BACKUP1_REG_89        89ul
/* 后备2 BACKUP2 */
#define MB_RW_BACKUP2_REG_90        90ul
#define MB_RW_BACKUP2_REG_91        91ul
#define MB_RW_BACKUP2_REG_92        92ul
#define MB_RW_BACKUP2_REG_93        93ul
#define MB_RW_BACKUP2_REG_94        94ul
#define MB_RW_BACKUP2_REG_95        95ul
#define MB_RW_BACKUP2_REG_96        96ul
#define MB_RW_BACKUP2_REG_97        97ul
#define MB_RW_BACKUP2_REG_98        98ul
#define MB_RW_BACKUP2_REG_99        99ul
//------------------------------------------------------------------------------
/* 扩展变量 ------------------------------------------------------------------*/
extern Modbus_T modbus;
extern uint8_t Rx_Buffer[MODBUS_LENGTH];
extern uint8_t Tx_Buffer[MODBUS_LENGTH];

/* 函数声明 ------------------------------------------------------------------*/
void mb_Init(void);
void mb_TimesProcess(void);
void mb_SendBuffer(uint8_t _length);
extern void mb_Receive(uint8_t _recStr);
void mb_Error(void);
void mb_Poll(void);

#endif /* __MODBUS_H__ */
