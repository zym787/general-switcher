#define _MAIN_GLOBALS_
#include "common.h"

uint8 moduleAddrDflt = 1, valveFixDflt = 3, valveFixDir=0,
      valvePortCnt=10, baudDflt=2, spdDflt=20, IODflt=0, IntDflt=5, IsetDflt=2;


void IOconfig(void)
{
    #ifdef IOCTRL
    RCC->APB2ENR |= RCC_APB2Periph_AFIO;
    RCC->APB2ENR |= (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB);

    #ifdef A12_901
    // FB OUT
    GPIOA->CRH &= (GPIO_Crh_P8);
    GPIOA->CRH |= (GPIO_Mode_Out_PP_50MHz_P8);
    GPIOA->ODR |= (GPIO_Pin_8);
    // KEY IN
    GPIOB->CRL &= (GPIO_Crl_P3);
    GPIOB->CRL |= (GPIO_Mode_IN_PU_PD_P3);
    #endif

    #ifdef A12_909
    // FB OUT
    GPIOB->CRH &= (GPIO_Crh_P13);
    GPIOB->CRH |= (GPIO_Mode_Out_PP_50MHz_P13);
    GPIOB->ODR |= (GPIO_Pin_13);
    // KEY IN
    GPIOB->CRL &= (GPIO_Crl_P5);
    GPIOB->CRL |= (GPIO_Mode_IN_PU_PD_P5);
    #endif

    #ifdef A12_906
    // FB OUT
    GPIOB->CRH &= (GPIO_Crh_P13);
    GPIOB->CRH |= (GPIO_Mode_Out_PP_50MHz_P13);
    GPIOB->ODR |= (GPIO_Pin_13);
    // KEY IN
    GPIOB->CRH &= (GPIO_Crh_P14);
    GPIOB->CRH |= (GPIO_Mode_IN_PU_PD_P14);
    #endif
    #endif
}

// 检测任务
// 1 IO(实时)
// 2 超时检测任务(1s)
#define SINGLE_RUN_TIMEOUT          5           // 运行5秒超时
#define SINGLE_INITING_TIMOUT       14           // 转一圈差不多3秒，复位单次是两圈
void EveryHSec(void)
{
    #ifdef IOCTRL
    if(bIoCtrl)
    {
        if(timerPara.timeMilli>DCSEC)
        {
            timerPara.timeMilli = 0;
            if(IO_IN)               // AI -- 1
            {// 未接通
                if(valve.status==VALVE_RUN_END)
                {
                    if(valve.portCur==POS_B)
                    {
                        valve.portDes = POS_A;
                    }
                 #if IO_RS  // IO_RS 1 A 232/485/IO
                    #ifdef A12_901
                        IO_OUT = ON;    // AI -- 1  BO -- 1
                    #endif
                    #ifdef A12_906
                        IO_OUT = OFF;   // AI -- 1  BO -- 0
                    #endif
                    #ifdef A12_909
                        IO_OUT = ON;    // AI -- 1  BO -- 0(无AB区别)
                    #endif
                 #else      // IO_RS 0 B IO
                    #ifdef A12_901
                        IO_OUT = OFF;   // AI -- 1  BO -- 0
                    #endif
                    #ifdef A12_906
                        IO_OUT = ON;    // AI -- 1  BO -- 1
                    #endif
                    #ifdef A12_909
                        IO_OUT = ON;    // AI -- 1  BO -- 0(无AB区别)
                    #endif
                #endif
                }
            }
            else                    // AI -- 0
            {// 接通
                if(valve.status==VALVE_RUN_END)
                {
                    if(valve.portCur==POS_A)
                    {
                        valve.portDes = POS_B;
                    }
                 #if IO_RS  // IO_RS 1 A 232/485/IO
                    #ifdef A12_901
                        IO_OUT = OFF;   // AI -- 0  BO -- 0
                    #endif
                    #ifdef A12_906
                        IO_OUT = ON;    // AI -- 0  BO -- 1
                    #endif
                    #ifdef A12_909
                        IO_OUT = OFF;   // AI -- 0  BO -- 1(无AB区别)
                    #endif
                 #else      // IO_RS 0 B IO
                    #ifdef A12_901
                        IO_OUT = ON;    // AI -- 0  BO -- 1
                    #endif
                    #ifdef A12_906
                        IO_OUT = OFF;   // AI -- 0  BO -- 0
                    #endif
                    #ifdef A12_909
                        IO_OUT = OFF;   // AI -- 0  BO -- 1(无AB区别)
                    #endif
                #endif
                }
            }
        }
    }
    #endif
    // 每秒检测一次
    if(timerPara.sec>SEC)
    {
        timerPara.sec = 0;
        if(valve.status==VALVE_RUN_END)
        {
            if(syspara.totalCnt!=syspara.totalCntLst)
            {
                syspara.totalCntLst = syspara.totalCnt;
                I2CPageWrite_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, (uint8*)&syspara.totalCnt);
            }
        }
        // 超时报错
        // 单通道间做5秒的超时处理，避免长时间堵转烧坏电路
        if((valve.status == VALVE_RUNNING&& 
            syspara.protectTimeOut > SINGLE_RUN_TIMEOUT*SEC) ||
            (valve.status&VALVE_INITING && 
            syspara.protectTimeOut > SINGLE_INITING_TIMOUT*SEC))
        {
            if(!(valve.status&VALVE_RUN_ERR))
            {
                valve.portDes = 0;
                valve.status = VALVE_RUN_ERR;
                VALVE_ENA = DISABLE;
                printd("\r\n time out err");
            }
            else
            {
                VALVE_ENA = DISABLE;
            }
        }
        if(syspara.protectTimeOut > (SINGLE_INITING_TIMOUT+1)*SEC)
        {
            valve.status = VALVE_RUN_ERR;
            VALVE_ENA = DISABLE;
            printd("\r\n Timeout protection!");
        }
    }
}


void EnableReceive(void)
{
    USART2->CR1 |= 1<<8;              //PE中断使能,接收缓冲区非空中断使能
	USART2->CR1 |= 1<<5;              //接收缓冲区非空中断使能

	USART3->CR1 |= 1<<8;              //PE中断使能,接收缓冲区非空中断使能
	USART3->CR1 |= 1<<5;              //接收缓冲区非空中断使能
}

void DisableReceive(void)
{
    USART2->CR1 &= ~(1<<8);              //PE中断失能,接收缓冲区非空中断使能
	USART2->CR1 &= ~(1<<5);              //接收缓冲区非空中断失能

	USART3->CR1 &= ~(1<<8);              //PE中断使能,接收缓冲区非空中断使能
	USART3->CR1 &= ~(1<<5);              //接收缓冲区非空中断使能
}

void ParameterInit(void)
{
    uint8 ReadBuf[2]={0,0};

    I2CPageRead_Nbytes(ADDR_BOARD_ID, LEN_BOARD_ID, ReadBuf);
    if(ReadBuf[0]==0x88 && ReadBuf[1]==0x66)
    {
        printd("\r Read stored data");

        I2CPageRead_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, &ModbusPara.mAddrs);
        printd("\r Addr:%d", ModbusPara.mAddrs);

        I2CPageRead_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
        (valveFix.fix.portCnt&&valveFix.fix.portCnt>24)?(valveFix.fix.portCnt=10):(valveFix.fix.portCnt);
        printd("\r Port Cnt:%d", valveFix.fix.portCnt);

        I2CPageRead_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &valveFix.fix.org);
        if(valveFix.fix.org==0)
        {
            valveFix.fix.org = 3;
            printd("\r 原点补偿为0度,缺省写入%d度", valveFix.fix.org);
            I2CPageWrite_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &valveFixDflt);
        }
        else
        {
            printd("\r Fix:%d", valveFix.fix.org);
        }
        I2CPageRead_Nbytes(ADDR_DIR_FIX, LEN_DIR_FIX, &valveFix.fix.dirGap);
        printd("\r Dir:%d", valveFix.fix.dirGap);

        I2CPageRead_Nbytes(ADDR_BAUD, LEN_BAUD, &bdrate);
        printd("\r baud:%d", bdrate);

        I2CPageRead_Nbytes(ADDR_SPD, LEN_SPD, ReadBuf);
        spdVx2 = ReadBuf[0];
        spdVx2 <<= 8;
        spdVx2 |= ReadBuf[1];
        printd("\r speed:%d", spdVx2);

        I2CPageRead_Nbytes(ADDR_IO_CTRL, LEN_IO_CTRL, &bIoCtrl);
        printd("\r IO:%d", bIoCtrl);

        I2CPageRead_Nbytes(ADDR_INTVL, LEN_INTVL, &intCtrl);
        printd("\r Interval:%d", intCtrl);
		#ifndef A12_901
        I2CPageRead_Nbytes(ADDR_ISET, LEN_ISET, &valve.iSet);
        ISET(valve.iSet);
        printd("\r current:%d", valve.iSet);
		#endif
        I2CPageRead_Nbytes(ADDR_RDC_RATE, LEN_RDC_RATE, &rdc.rate);
		switch(rdc.rate)
		{
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
		}
		rdc.stepRound = P_ROUND;
		rdc.stepRound *= SCALE;
		rdc.stepRound *= rdc.rate;
        I2CPageRead_Nbytes(ADDR_HALF_SEAL, LEN_HALF_SEAL, &valve.bHalfSeal);

        printd("\r Fix:");
        for(uint32 i=0; i<valveFix.fix.portCnt; i++)
            printd(" %d", valveFix.array[i]);
        
        I2CPageRead_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, ((uint8*)&syspara.totalCnt));
        printd("\r\n total count %d", syspara.totalCnt);
    }
    else
    {
        printd("\r Write default data");
        ReadBuf[0] = 0x88;
        ReadBuf[1] = 0x66;
        I2CPageWrite_Nbytes(ADDR_BOARD_ID, LEN_BOARD_ID, ReadBuf);
        I2CPageWrite_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, &moduleAddrDflt);
        ModbusPara.mAddrs = moduleAddrDflt;
        I2CPageWrite_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valvePortCnt);
        valveFix.fix.portCnt = valvePortCnt;
        I2CPageWrite_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &valveFixDflt);
        valveFix.fix.org = valveFixDflt;
        I2CPageWrite_Nbytes(ADDR_DIR_FIX, LEN_DIR_FIX, &valveFixDir);
        valveFix.fix.dirGap = valveFixDir;
        I2CPageWrite_Nbytes(ADDR_BAUD, LEN_BAUD, &baudDflt);
        bdrate = baudDflt;
        ReadBuf[0] = spdDflt>>8;
        ReadBuf[1] = spdDflt;
        I2CPageWrite_Nbytes(ADDR_SPD, LEN_SPD, ReadBuf);
        spdVx2 = spdDflt;
        I2CPageWrite_Nbytes(ADDR_IO_CTRL, LEN_IO_CTRL, &IODflt);
        bIoCtrl = IODflt;
        I2CPageWrite_Nbytes(ADDR_INTVL, LEN_INTVL, &IntDflt);
        intCtrl = IntDflt;
        I2CPageWrite_Nbytes(ADDR_ISET, LEN_ISET, &IsetDflt);
        valve.iSet = IsetDflt;
		#ifndef A12_901
        ISET(valve.iSet);
		#endif
    }
//    if(!spdVx2)
//    {
//        spdVx2 = spdDflt;
//    }
    speed[AXSV] = 100;
    accel[AXSV] = 100;
    decel[AXSV] = 200;
    speed[AXSV] *= (INIT_SPD);  // 使用初始化速度找原点
    speed[AXSV] *= (rdc.rate);
    accel[AXSV] *= (INIT_SPD);
    accel[AXSV] *= (rdc.rate);
    decel[AXSV] *= (INIT_SPD);
    decel[AXSV] *= (rdc.rate);
    printd("\r\n Init motion!  Slow Down!  spd%d acc%d dec%d", speed[AXSV], accel[AXSV], decel[AXSV]);

    valve.status = VALVE_INITING;
    valve.ErrBlinkTime = NORMAL_BLINK;
    valve.passByOne = 0;
    valve.bReInit = 1;
}

int main(void)
{
	Stm32_Clock_Init(9);					//系统时钟设置
	delay_init(72);	   	 					//延时初始化
	JTAG_Set(JTAG_SWD_DISABLE);
    delay_ms(100);
	Usart1_Init(72, 115200);				//串口初始化默认为115200
    iic_INIT();
    ModbusInit();
    TIM2_Init(999,71);                      //ms_1 中断
    TIM4_Init(65535,35);                    //X轴脉冲定时器
    MotorCfg();
    IOconfig();
    delay_ms(100);
    printd("\r\n Version:%s  Time:%s", SOFT_VER_C, __TIME__);
    ParameterInit();
    UsrCmdInit();
	while(1)
	{
        InitValve();
        ProcessValve();
        ModbusProces();
        EveryHSec();
        TestBurn();
        DebugOut();
    }
}

void DebugOut(void)
{
    if(timerPara.timeDbg > 1000)
    {
        timerPara.timeDbg = 0;
        LED_WORK = !LED_WORK;
        printd("\r\n >>sta:%02x port:%02x dest:%02x opt:%02x %d initstep%d IO_IN%d IO_OUT%d", 
            valve.status, valve.portCur, valve.portDes, VALVE_OPT, IO_IN, 
            valve.initStep, IO_IN, IO_OUT);
    }
}





