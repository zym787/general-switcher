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

#define BYTE_RANGE_MIN      0
#define BYTE_RANGE_MAX      255
#define AGS_ADDR_MIN        0       /* AGS地址最小 0 */
#define AGS_ADDR_MAX        63      /* AGS地址最大 63 */
#define BURN_ADDR           64      /* 老化地址 64 */
#define MOTOR_AGING_ADDR    65      /* 电机老化地址 65 */
#define AGS_ADDR_DEF        1       /* 默认地址 1 */
#define INIT_SPD            20      /* 初始化找位速度 */
#define SPD_MIN             1       /* 最小速度 */
#define SPD_MAX             200     /* 最大速度 */
#define SPD_MIN_RDCR20      1       /* 20减速比 最小速度 */
#define SPD_MAX_RDCR20      50      /* 20减速比 最大速度 */
#define CHANNEL_MIN         3       /* 最小通道数 */
#define CHANNEL_MAX         16      /* 最大通道数 */
#define CHANNEL_DEF         10      /* 默认通道数 */
#define BAUD_MIN            1       /* 最小波特率 */
#define BAUD_MAX            3       /* 最大波特率 */
#define BAUD_DEF            1       /* 默认波特率 */

#define RDC01               1
#define RDC04               4
#define RDC10               10
#define RDC16               16
#define RDC20               20

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


#ifdef A12_901  /* A12_901 64细分 64*200=12800 */
#define STEPS_1_DEGREE_RD01      (35.6)     // 每1度需走的步数为    12800/360=35.555~
#define STEPS_01_DEGREE_RD01     (3.6)      // 每0.1度需走的步数为  12800/3600=3.555~
#define STEPS_1_DEGREE_RD04      (142.2)    // 每1度需走的步数为    12800*4/360=142.222~
#define STEPS_01_DEGREE_RD04     (14.2)     // 每0.1度需走的步数为  12800*4/3600=14.222~
#define STEPS_1_DEGREE_RD10      (355.6)    // 每1度需走的步数为    12800*10/360=355.555~
#define STEPS_01_DEGREE_RD10     (35.6)     // 每0.1度需走的步数为  12800*10/3600=35.555~
#define STEPS_1_DEGREE_RD16      (568.9)    // 每1度需走的步数为    12800*16/360=568.888~
#define STEPS_01_DEGREE_RD16     (56.9)     // 每0.1度需走的步数为  12800*16/3600=56.888~
#define STEPS_1_DEGREE_RD20      (711.1)    // 每1度需走的步数为    12800*20/360=711.111~
#define STEPS_01_DEGREE_RD20     (71.1)     // 每0.1度需走的步数为  12800*20/3600=71.111~
#endif
#ifdef A12_909  /* A12_909 64细分 64*200=12800 */
#define STEPS_1_DEGREE_RD01      (35.6)     // 每1度需走的步数为    12800/360=35.555~
#define STEPS_01_DEGREE_RD01     (3.6)      // 每0.1度需走的步数为  12800/3600=3.555~
#define STEPS_1_DEGREE_RD04      (142.2)    // 每1度需走的步数为    12800*4/360=142.222~
#define STEPS_01_DEGREE_RD04     (14.2)     // 每0.1度需走的步数为  12800*4/3600=14.222~
#define STEPS_1_DEGREE_RD10      (355.6)    // 每1度需走的步数为    12800*10/360=355.555~
#define STEPS_01_DEGREE_RD10     (35.6)     // 每0.1度需走的步数为  12800*10/3600=35.555~
#define STEPS_1_DEGREE_RD16      (568.9)    // 每1度需走的步数为    12800*16/360=568.888~
#define STEPS_01_DEGREE_RD16     (56.9)     // 每0.1度需走的步数为  12800*16/3600=56.888~
#define STEPS_1_DEGREE_RD20      (711.1)    // 每1度需走的步数为    12800*20/360=711.111~
#define STEPS_01_DEGREE_RD20     (71.1)     // 每0.1度需走的步数为  12800*20/3600=71.111~
#endif
#ifdef A12_906  /* A12_906 16细分 16*200=3200 */
#define STEPS_1_DEGREE_RD01      (8.9)      // 每1度需走的步数为    3200/360=8.888~
#define STEPS_01_DEGREE_RD01     (0.9)      // 每0.1度需走的步数为  3200/3600=0.888~
#define STEPS_1_DEGREE_RD04      (35.6)     // 每1度需走的步数为    3200*4/360=35.555~
#define STEPS_01_DEGREE_RD04     (3.6)      // 每0.1度需走的步数为  3200*4/3600=3.555~
#define STEPS_1_DEGREE_RD10      (88.9)     // 每1度需走的步数为    3200*10/360=88.888~
#define STEPS_01_DEGREE_RD10     (8.9)      // 每0.1度需走的步数为  3200*10/3600=8.888~
#define STEPS_1_DEGREE_RD16      (142.2)    // 每1度需走的步数为    3200*16/360=142.222~
#define STEPS_01_DEGREE_RD16     (14.2)     // 每0.1度需走的步数为  3200*16/3600=14.222~
#define STEPS_1_DEGREE_RD20      (177.8)    // 每1度需走的步数为    3200*20/360=177.777~
#define STEPS_01_DEGREE_RD20     (17.8)     // 每0.1度需走的步数为  3200*20/3600=17.777~
#endif


typedef struct
{
    uint32_t OptStep;           //光耦脉冲步数统计
    uint32_t OptHigh;           //光耦高电平脉冲步数
    uint32_t OptLow;            //光耦低电平脉冲步数
    uint32_t stpCnt;            //初始化后开始补偿的步数

    uint32_t BaudRate;          //运行的波特率值

    uint16_t ErrBlinkTime;    //错误灯提示间隔
    uint8_t Addr;             //模块地址
    uint8_t status;           //通阀当前状态
    uint8_t optLast;
    uint8_t retryTms;         //走位重试的次数

    uint8_t initStep;         //阀初始化的步骤
    uint8_t portCur;          //通阀当前位置编号
    uint8_t portDes;          //通阀目标位置编号
    uint8_t direct;           //通阀目标位置编号
    uint8_t portLast;         // 上一个位置
    uint8_t iSet;             // 上一个位置
    uint8_t lastIO;             // 上一个状态
    uint8_t fixOrg;           // 原点补偿

    uint8_t dirLast;          //
    uint8_t statusLast;       //
    uint8_t bStrtCnt;         //新一轮找位,丢弃首个滞留计步
    uint8_t bHalfSeal;        // 是否半通道

    uint8_t bNewInit;         // 刚复位完成
    uint8_t bEmgStopV;        // 再次复位转动
    uint8_t SnCode[LEN_SN];   // 序列码
    uint8_t spd;              // 当前速度

} _VALVE_T;
PEXT _VALVE_T valve;

typedef struct
{
    uint8_t     rate;
    uint32_t    stepRound;
    float       stepP1dgr;
    float       stepP01dgr;
} RDC_T;
PEXT RDC_T rdc;


#define FIX_VAL_LENGHT      14
typedef union
{
    uint8_t array[FIX_VAL_LENGHT];
    struct
    {
        uint8_t port1Val;
        uint8_t port2Val;
        uint8_t port3Val;
        uint8_t port4Val;

        uint8_t port5Val;
        uint8_t port6Val;
        uint8_t port7Val;
        uint8_t port8Val;

        uint8_t port9Val;
        uint8_t port10Val;
        uint8_t port11Val;
        uint8_t port12Val;

        uint8_t org;              // 原点补偿 FIXO
        uint8_t dirGap;           // 方向补偿 FIXG
        uint8_t portCnt;          //
    } fix;
} _12VALVE_FIX;
PEXT _12VALVE_FIX valveFix;

PEXT void bsp_ValveGpioInit(void);
PEXT void bsp_ValveInit(void);
PEXT void bsp_ValveProcess(void);
PEXT void bsp_ValveOrigin(void);
PEXT void bsp_ValveAgingMode(void);

#undef PEXT
#endif


