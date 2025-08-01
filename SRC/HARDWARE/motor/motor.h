#ifndef _MOTOR_H_
#define _MOTOR_H_

#ifdef _MOTOR_GLOBALS_
#define PEXT
#else
#define PEXT extern
#endif

#define I_26A               0x00
#define I_22A               0x01
#define I_18A               0x02
#define I_16A               0x03
#define I_05A               0x04

#if (defined A12_901)||(defined A12_909)
#define LED_WORK            PCout(15)
#define VALVE_OPT           PAin(15)
#define VALVE_ENA		    PAout(4)
#define VALVE_RST		    PAout(5)
#define VALVE_DIR		    PAout(6)
#define VALVE_CLK		    PAout(7)

#ifdef A12_909
#define M_ISET_1           PBout(0)
#define M_ISET_2           PBout(12)
#define M_ISET_3           PAout(11)
#endif
#endif

#ifdef A12_906
#define LED_WORK            PCout(14)
#define VALVE_OPT           PCin(15)
#define VALVE_ENA		    PAout(6)
#define VALVE_RST		    PAout(7)
#define VALVE_DIR		    PAout(4)
#define VALVE_CLK		    PAout(5)

#define M_ISET_1           PBout(12)
#define M_ISET_2           PBout(0)
#define M_ISET_3           PAout(11)
#endif

#if (defined A12_906)||(defined A12_909)
#define ISET(val)   \
    M_ISET_1=(val>>0&0x01);\
    M_ISET_2=(val>>1&0x01);\
    M_ISET_3=(val>>2&0x01);
#endif

#define RETRY_TIMES         2                       //重试次数

#define VALVE_NONE          0x00
#define VALVE_INITING       0x80
#define VALVE_RUN_END       0x40
#define VALVE_RUN_ERR       0x20
#define VALVE_RUNNING       0x08

#define PULSE_CNT_EN        0

/* 光感状态枚举 */
enum OPTO_STATE
{
    OPT_GAP = 0ul,  /* 缺口狭缝 0 */
    OPT_BLOCKER     /* 挡光片   1 */
};

#define POS_N               0x00    /* 空位置   0 */
#define POS_A               0x01    /* A位置    1 */
#define POS_B               0x02    /* B位置    2 */
#define POS_M               0xff    /* 半通道   FF */

#define AXSV                0

#define BYTE_RANGE          255

#define AGS_ADDR_MIN        0       /* AGS地址最小 0 */
#define AGS_ADDR_MAX        63      /* AGS地址最大 64 */
#define INIT_SPD            20      /* 初始化找位速度 */
#define SPD_MIN             20      /* 最小速度 */
#define SPD_MAX             200     /* 最大速度 */
#define CHANNEL_MIN         3       /* 最小通道数 */
#define CHANNEL_MAX         16      /* 最大通道数 */
#define BAUD_MIN            1       /* 最小波特率 */
#define BAUD_MAX            3       /* 最大波特率 */

#define RDC01               1
#define RDC04               4
#define RDC10               10
#define RDC16               16

#ifdef A12_901
#define SCALE               64                      //当前细分数为64
#endif
#ifdef A12_909
#define SCALE               64                      //当前细分数为64
#endif
#ifdef A12_906
#define SCALE               16                      //当前细分数为64
#endif
#define P_ROUND             200                     //每圈大步数为200
#define RT_INIT             2


#ifdef A12_901
#define STEPS_1_DEGREE_RD01      (35.6)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD01     (3.6)                  //每0.1度需走的步数为12800/3600=3.555~
#define STEPS_1_DEGREE_RD04      (142.2)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD04     (14.2)                  //每0.1度需走的步数为12800/3600=3.555~
#define STEPS_1_DEGREE_RD10      (355.6)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD10     (35.6)                  //每0.1度需走的步数为12800/3600=3.555~
#define STEPS_1_DEGREE_RD16      (568.9)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD16     (56.9)                  //每0.1度需走的步数为12800/3600=3.555~
#endif
#ifdef A12_909
#define STEPS_1_DEGREE_RD01      (35.6)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD01     (3.6)                  //每0.1度需走的步数为12800/3600=3.555~
#define STEPS_1_DEGREE_RD04      (142.2)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD04     (14.2)                  //每0.1度需走的步数为12800/3600=3.555~
#define STEPS_1_DEGREE_RD10      (355.6)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD10     (35.6)                  //每0.1度需走的步数为12800/3600=3.555~
#define STEPS_1_DEGREE_RD16      (568.9)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD16     (56.9)                  //每0.1度需走的步数为12800/3600=3.555~
#endif
#ifdef A12_906
#define STEPS_1_DEGREE_RD01      (8.9)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD01     (0.9)                  //每0.1度需走的步数为12800/3600=3.555~
#define STEPS_1_DEGREE_RD04      (35.6)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD04     (3.6)                  //每0.1度需走的步数为12800/3600=3.555~
#define STEPS_1_DEGREE_RD10      (88.9)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD10     (8.9)                  //每0.1度需走的步数为12800/3600=3.555~
#define STEPS_1_DEGREE_RD16      (142.2)                 //每度需走的步数为12800/360=35.555~
#define STEPS_01_DEGREE_RD16     (14.2)                  //每0.1度需走的步数为12800/3600=3.555~
#endif


typedef struct
{
    unsigned int OptStep;           //光耦脉冲步数统计
    unsigned int OptHigh;           //光耦高电平脉冲步数
    unsigned int OptLow;            //光耦低电平脉冲步数
    unsigned int stpCnt;            //初始化后开始补偿的步数

    unsigned int BaudRate;          //运行的波特率值

    unsigned short ErrBlinkTime;    //错误灯提示间隔
    unsigned char Addr;             //模块地址
    unsigned char status;           //通阀当前状态
    unsigned char optLast;
    unsigned char retryTms;         //走位重试的次数

    unsigned char initStep;         //阀初始化的步骤
    unsigned char portCur;          //通阀当前位置编号
    unsigned char portDes;          //通阀目标位置编号
    unsigned char direct;           //通阀目标位置编号
    unsigned char portLast;         // 上一个位置
    unsigned char iSet;             // 上一个位置
    unsigned char lastIO;             // 上一个状态
    unsigned char fixOrg;           // 原点补偿

    unsigned char dirLast;          //
    unsigned char statusLast;       //
    unsigned char passByOne;        //重新校准1号位标志
    unsigned char bStrtCnt;         //新一轮找位,丢弃首个滞留计步
    unsigned char bHalfSeal;        // 是否半通道

    unsigned char bNewInit;         // 刚复位完成
    unsigned char bReInit;          // 再次复位转动
    unsigned char bEmgStopV;        // 再次复位转动
    unsigned char SnCode[LEN_SN];   // 序列码
} _VALVE_T;
PEXT _VALVE_T valve;

typedef struct
{
	uint8_t     rate;
	uint32_t    stepRound;
	float       stepP1dgr;
	float       stepP01dgr;
}RDC_T;
PEXT RDC_T rdc;


#define FIX_VAL_LENGHT      14
typedef union
{
    unsigned char array[FIX_VAL_LENGHT];
    struct
    {
        unsigned char port1Val;
        unsigned char port2Val;
        unsigned char port3Val;
        unsigned char port4Val;

        unsigned char port5Val;
        unsigned char port6Val;
        unsigned char port7Val;
        unsigned char port8Val;

        unsigned char port9Val;
        unsigned char port10Val;
        unsigned char port11Val;
        unsigned char port12Val;

        unsigned char org;              // 原点补偿 FIXO
        unsigned char dirGap;           // 方向补偿 FIXG
        unsigned char portCnt;          //
    } fix;
} _12VALVE_FIX;
PEXT _12VALVE_FIX valveFix;

PEXT void MotorCfg(void);
PEXT void InitValve(void);
PEXT void ValveProcess(void);
PEXT void ProcessValve(void);
PEXT void ValveOrg(void);
PEXT void TestBurn(void);

#undef PEXT
#endif


