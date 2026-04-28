#define _MAIN_GLOBALS_
#include "common.h"

// clang-format off
uint8_t valveFixDflt    = 5,    /* 默认原点补偿 */
        valveFixDir     = 0,    /* 默认方向补偿 */
        rateDflt        = 4,    /* 默认减速比 */
        IntDflt         = 5,    /* 默认烧机老化时间间隔 */
        IsetDflt        = 0;    /* 默认电流设置 */

uint8_t ReadBuf[2] = {0, 0};
// clang-format on

void IOconfig(void)
{
        /* PB1 为485芯片收发切换引脚 */
        RCC->APB2ENR |= (RCC_APB2Periph_GPIOB);
        GPIOB->CRL &= (GPIO_Crl_P1);
        GPIOB->CRL |= (GPIO_Mode_Out_PP_50MHz_P1);
        RX_EN();  // 开机为接收模式

}

/* 检测任务 */
#define SINGLE_RUN_TIMEOUT    5   // 运行5秒超时
#define SINGLE_INITING_TIMOUT 14  // 转一圈差不多3秒,复位单次是两圈
void task_Cycle_1Sec(void) {
        /* 超时检测任务 (1s) */
        if (SEC > timerPara.sec) 
        {
                return;
        }
        timerPara.sec = 0;

        /* 保存切换次数 */
        if (VALVE_RUN_END == valve.status) {
                if (syspara.totalCnt != syspara.totalCntLst) /* 保存切换次数 */
                {
                        syspara.totalCntLst = syspara.totalCnt;
                        I2CPageWrite_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, (uint8*)&syspara.totalCnt);
                }
        }

        /* 单通道间做5秒的超时处理,避免长时间堵转烧坏电路 */
        if ((valve.status == VALVE_RUNNING && syspara.protectTimeOut > SINGLE_RUN_TIMEOUT * SEC) ||
                (valve.status & VALVE_INITING && syspara.protectTimeOut > SINGLE_INITING_TIMOUT * SEC)) {
                if (!(valve.status & VALVE_RUN_ERR)) {
                        valve.portDes = 0;
                        valve.status = VALVE_RUN_ERR; /* 超时报错 */
                        VALVE_ENA = DISABLE;
                } else {
                        VALVE_ENA = DISABLE;
                }
                printd("\r\n time out error! (initstep%d,%dms)", valve.initStep, syspara.protectTimeOut);
                valve.ErrBlinkTime = ERROR_BLINK;
        }
        /* 15秒超时锁机 */
        if (syspara.protectTimeOut > (SINGLE_INITING_TIMOUT + 1) * SEC) {
                valve.status = VALVE_RUN_ERR; /* 超时报错 */
                VALVE_ENA = DISABLE;
                printd("\r\n %d Timeout protection! (initstep%d,%dms)", SINGLE_INITING_TIMOUT + 1,
                        valve.initStep, syspara.protectTimeOut);
                valve.ErrBlinkTime = ERROR_BLINK;
        }
        
}

void task_Scheduler(void)
{
        /* IO及LED任务 */
        bsp_IODetect();

        /* 1秒任务 */
        task_Cycle_1Sec();
}

/**
 * @brief    参数读取函数
 */
void param_Read(void)
{
        printd("\r 读取系统保存参数");
        /* 地址 */
        I2CPageRead_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, &ags_mbParam.mAddrs);
        modbus.Address = ags_mbParam.mAddrs;
        printd("\r 地址:%d", ags_mbParam.mAddrs);
        /* 原点补偿 */
        I2CPageRead_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &valveFix.fix.org);
        if (0 == valveFix.fix.org) {
                valveFix.fix.org = valveFixDflt;
                printd("\r 原点补偿为0度,缺省写入%d度", valveFix.fix.org);
                I2CPageWrite_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &valveFix.fix.org);
        } else {
                printd("\r 原点补偿:%d (1度)", valveFix.fix.org);
        }
        /* 方向补偿 */
        I2CPageRead_Nbytes(ADDR_DIR_FIX, LEN_DIR_FIX, &valveFix.fix.dirGap);
        printd("\r 方向补偿:%d (0.1度)", valveFix.fix.dirGap);
        /* 通道数 */
        I2CPageRead_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
        if (CHANNEL_MIN <= valveFix.fix.portCnt && CHANNEL_MAX >= valveFix.fix.portCnt) {
                printd("\r 通道数:%d", valveFix.fix.portCnt);
        } else {
                valveFix.fix.portCnt = CHANNEL_DEF;
                printd("\r 通道数超限,默认写入%d 请重新设置!", valveFix.fix.portCnt);
                I2CPageWrite_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
        }
        /* 波特率 */
        I2CPageRead_Nbytes(ADDR_BAUD, LEN_BAUD, &syspara.baudrate);
        if (BAUD_9600 <= syspara.baudrate && BAUD_38400 >= syspara.baudrate) {
                printd("\r 波特率:%d %dbps", syspara.baudrate, BaudRate_V[syspara.baudrate]);
        } else {
                syspara.baudrate = BAUD_9600;
                printd("\r 波特率超限,默认写入%d 9600bps 请重新设置!", syspara.baudrate);
                I2CPageWrite_Nbytes(ADDR_BAUD, LEN_BAUD, &syspara.baudrate);
        }
        /* 速度 */
        I2CPageRead_Nbytes(ADDR_SPD, LEN_SPD, &valve.spd);
        if (SPD_MIN <= valve.spd && SPD_MAX >= valve.spd) {
                printd("\r 速度:%d 转/分钟", valve.spd);
        } else {
                valve.spd = INIT_SPD;
                printd("\r 速度超限,默认写入%d 请重新设置!", valve.spd);
        }
#ifdef IOCTRL
        /* IO控制 */
        I2CPageRead_Nbytes(ADDR_IO_CTRL, LEN_IO_CTRL, (uint8_t *)&syspara.ioCtrl);
        printd("\r IO控制:%d %s", syspara.ioCtrl, (0 == syspara.ioCtrl ? "关" : "开"));
#endif
        /* 老化间隔 */
        I2CPageRead_Nbytes(ADDR_INTVL, LEN_INTVL, &syspara.agingInterval);
        printd("\r 老化间隔:%d 秒", syspara.agingInterval);
        /* 电流设置 */
        /* 906/909 支持电流设置 */
#ifndef A12_901
        I2CPageRead_Nbytes(ADDR_ISET, LEN_ISET, &valve.iSet);
        ISET(valve.iSet);
        printd("\r 电流:%d %sA  0(Max)-4(Min)", valve.iSet,
               (0 == valve.iSet
                    ? "2.6"
                    : (1 == valve.iSet ? "2.2" : (2 == valve.iSet ? "1.8" : (3 == valve.iSet ? "1.6" : "0.5")))));
#endif
        /* 序列号 */
        I2CPageRead_Nbytes(ADDR_SN, LEN_SN, valve.SnCode);
        printd("\r 序列号:");
        for (uint8_t i = 0; i < 5; ++i)
                printd(" %02X", *(valve.SnCode + i));

        /* 控制协议 */
        I2CPageRead_Nbytes(ADDR_PROTOCOL, LEN_PROTOCOL, &syspara.protocol_type);
        printd("\r\n 控制协议: %d %s", syspara.protocol_type,
               (syspara.protocol_type) == AGS_MODBUS ? "AGS"
               : (syspara.protocol_type) == MODBUS   ? "MODBUS"
                                                     : "wrong type");

        /* 减速比 */
        I2CPageRead_Nbytes(ADDR_RDC_RATE, LEN_RDC_RATE, &rdc.rate);
        switch (rdc.rate) {
                case RDC01:
                        rdc.stepP1dgr = STEPS_1_DEGREE_RD01;
                        rdc.stepP01dgr = STEPS_01_DEGREE_RD01;
                        break;
                case RDC04:
                        rdc.stepP1dgr = STEPS_1_DEGREE_RD04;
                        rdc.stepP01dgr = STEPS_01_DEGREE_RD04;
                        break;
                case RDC10:
                        rdc.stepP1dgr = STEPS_1_DEGREE_RD10;
                        rdc.stepP01dgr = STEPS_01_DEGREE_RD10;
                        break;
                case RDC16:
                        rdc.stepP1dgr = STEPS_1_DEGREE_RD16;
                        rdc.stepP01dgr = STEPS_01_DEGREE_RD16;
                        break;
                case RDC20:
                        rdc.stepP1dgr = STEPS_1_DEGREE_RD20;
                        rdc.stepP01dgr = STEPS_01_DEGREE_RD20;
                        break;
                default:
                        printd("\r 减速比参数错误,缺省写入4");
                        rdc.rate = RDC04;
                        rdc.stepP1dgr = STEPS_1_DEGREE_RD04;
                        rdc.stepP01dgr = STEPS_01_DEGREE_RD04;
                        break;
        }
        rdc.stepRound = P_ROUND;    // 单圈步数 200
        rdc.stepRound *= SCALE;     // 细分
        rdc.stepRound *= rdc.rate;  // 减速比
        printd("\r 减速比:%d 一圈步数:%d", rdc.rate, rdc.stepRound);
        /* 半通道 */
        I2CPageRead_Nbytes(ADDR_HALF_SEAL, LEN_HALF_SEAL, &valve.bHalfSeal);
        printd("\r 半通道:%d %s", valve.bHalfSeal, (0 == valve.bHalfSeal ? "关" : "开"));
        /* 补偿 */
        // printd("\r Fix:");
        // for(uint32 i=0; i<valveFix.fix.portCnt; i++)
        //     printd(" %d", valveFix.array[i]);
        /* 切换次数 */
        I2CPageRead_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, ((uint8 *)&syspara.totalCnt));
        printd("\r\n 切换次数:%d", syspara.totalCnt);
        /// 回复方式
        I2CPageRead_Nbytes(ADDR_REPLY_MODE, LEN_REPLY_MODE, &syspara.replyMode);
        printd("\r 回复方式:%d", syspara.replyMode);
}

/**
 * @brief    默认参数写入
 */
void param_Write(void)
{
        printd("\r 写入默认参数");
        // 板号
        ReadBuf[0] = BOARD_0;
        ReadBuf[1] = BOARD_1;
        I2CPageWrite_Nbytes(ADDR_BOARD_ID, LEN_BOARD_ID, ReadBuf);
        /* 地址 1 */
        ags_mbParam.mAddrs = AGS_ADDR_DEF;
        modbus.Address = ags_mbParam.mAddrs;
        I2CPageWrite_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, &ags_mbParam.mAddrs);
        /* 原点补偿 5度 */
        valveFix.fix.org = valveFixDflt;
        I2CPageWrite_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &valveFix.fix.org);
        /* 方向补偿 0 */
        valveFix.fix.dirGap = valveFixDir;
        I2CPageWrite_Nbytes(ADDR_DIR_FIX, LEN_DIR_FIX, &valveFix.fix.dirGap);
        /* 通道数 10 */
        valveFix.fix.portCnt = CHANNEL_DEF;
        I2CPageWrite_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
        /* 波特率 1 9600 */
        syspara.baudrate = BAUD_9600;
        I2CPageWrite_Nbytes(ADDR_BAUD, LEN_BAUD, &syspara.baudrate);
        /* 速度 20 */
        valve.spd = INIT_SPD;
        I2CPageWrite_Nbytes(ADDR_SPD, LEN_SPD, &valve.spd);
#ifdef IOCTRL
        /* IO控制 1 开启 */
        syspara.ioCtrl = ON;
#else
        /* IO控制 0 不开启 */
        syspara.ioCtrl = OFF;
#endif
        I2CPageWrite_Nbytes(ADDR_IO_CTRL, LEN_IO_CTRL, (uint8_t *)&syspara.ioCtrl);

        /* 老化间隔 5秒 */
        syspara.agingInterval = IntDflt;
        I2CPageWrite_Nbytes(ADDR_INTVL, LEN_INTVL, &syspara.agingInterval);
        /* 电流设置 906/909  0 最大 */
        valve.iSet = IsetDflt;
        I2CPageWrite_Nbytes(ADDR_ISET, LEN_ISET, &valve.iSet);
#ifndef A12_901
        ISET(valve.iSet);
#endif
        /* 序列号必须手动清空 */
        /* 协议 */
        syspara.protocol_type = AGS_MODBUS;
        I2CPageWrite_Nbytes(ADDR_PROTOCOL, LEN_PROTOCOL, &syspara.protocol_type);
        /* 减速比 4 */
        rdc.rate = RDC04;
        rdc.stepP1dgr = STEPS_1_DEGREE_RD04;
        rdc.stepP01dgr = STEPS_01_DEGREE_RD04;
        I2CPageWrite_Nbytes(ADDR_RDC_RATE, LEN_RDC_RATE, &rdc.rate);
        /* 半通道 0 */
        valve.bHalfSeal = OFF;
        I2CPageWrite_Nbytes(ADDR_HALF_SEAL, LEN_HALF_SEAL, &valve.bHalfSeal);
        /* 切换次数必须手动清空 */
        /// 回复方式
        syspara.replyMode = REPLYMODE_AGS; /* 默认AGS标准回复方式 */
        I2CPageWrite_Nbytes(ADDR_REPLY_MODE, LEN_REPLY_MODE, &syspara.replyMode);

        /* 写入参数后 锁定驱动? */
        VALVE_ENA = DISABLE;
        printd("\r 写入成功,请复位!!!");
}

void ParameterInit(void)
{
        /* 读取板号判断是否第一次进行初始化 */
        I2CPageRead_Nbytes(ADDR_BOARD_ID, LEN_BOARD_ID, ReadBuf);
        /* 读取默认参数 */
        if (BOARD_0 == ReadBuf[0] && BOARD_1 == ReadBuf[1]) {
                param_Read();
        }
        /* 写入默认参数 */
        else {
                param_Write();
        }
        /* 使用初始化速度找原点 20RPM */
        printd("\r\n 初始化电机!  减速!");
        bsp_ValveUpdateSpeed(INIT_SPD);

        valve.status = VALVE_INITING;
        valve.ErrBlinkTime = NORMAL_BLINK;
}

int main(void)
{
        Stm32_Clock_Init(9); /* 系统时钟设置 */
        delay_init(72);      /* 延时初始化 */
#if 0
    JTAG_Set(JTAG_SWD_DISABLE);
#else
        JTAG_Set(JTAG_SWD_ENABLE);
#endif
        delay_ms(100);
        Usart1_Init(72, 115200); /* 串口初始化为115200 */
        iic_INIT();
        TIM2_Init(999, 71);   /* 10Khz的计数频率 */
        TIM4_Init(65535, 35); /* X轴脉冲定时器 */
        bsp_ValveGpioInit();
        IOconfig();
        delay_ms(100);
#ifdef AGING_MODE
        printd(
            "\r\n Version:%s(%08X)  Time: %s %s \
            \r\n 正反切换老化程序 仅支持下载口设置参数\
            \r\n PCB:%s  %s \r\n",
            SOFT_VER_C, SOFT_VER, __DATE__, __TIME__, PCB_VR, HARDWARE_DESCRIPTION);
#else
        printd(
            "\r\n Version:%s(%08X)  Time: %s %s \
            \r\n Description:%s (%s)\
            \r\n PCB:%s  %s \r\n",
            SOFT_VER_C, SOFT_VER, __DATE__, __TIME__, DESCRIPTION, CONTROL, PCB_VR, HARDWARE_DESCRIPTION);
#endif
#ifndef RELEASE
        DIR_FLAG;
#endif
        I2CPageRead_Nbytes(ADDR_PROTOCOL, LEN_PROTOCOL, &syspara.protocol_type);
        /* 根据协议初始化 */
        if (syspara.protocol_type == AGS_MODBUS) {
                ags_mbInit(); /* AGS协议 */
        } else if (syspara.protocol_type == MODBUS) {
                    mb_Init(); /* 初始化Modbus协议 */
        }
        ParameterInit();
        UsrCmdInit();
        while (1) {
#ifndef AGING_MODE
                bsp_ValveInit();
                bsp_ValveProcess();
                /* 协议栈轮询 */
                if (syspara.protocol_type == AGS_MODBUS) {
                        ags_mbProcess(); /* AGS协议 */
                } else if (syspara.protocol_type == MODBUS) {
                        mb_Poll(); /* 解析Modbus数据帧 */
                }
                task_Scheduler();
                DebugOut();
#endif
                bsp_ValveAgingMode();
                ErrBlink();
        }
}

void DebugOut(void)
{
        if (timerPara.timeDbg > SEC * 3) {
                timerPara.timeDbg = 0;
#if 0
#ifdef IOCTRL
        printd("\r\n >>状态:%02x  当前位:%02x 目标位:%02x  光感:%02x  IO_IN:%02x IO_OUT:%02x",
            valve.status, valve.portCur, valve.portDes, VALVE_OPT, IO_IN, IO_OUT);
#else
        printd("\r\n >>状态:%02x  当前位:%02x 目标位:%02x  光感:%02x",
            valve.status, valve.portCur, valve.portDes, VALVE_OPT);
#endif
#endif
        }
}

void ErrBlink(void)
{
        /* 设置led闪烁间隔 */
        if (timerPara.timeOut > valve.ErrBlinkTime) {
                timerPara.timeOut = 0;
                LED_WORK = !LED_WORK;
        }
}
