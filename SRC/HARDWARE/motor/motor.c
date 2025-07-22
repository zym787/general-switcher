#define _MOTOR_GLOBALS_
#include "common.h"

void MotorCfg(void)
{
    RCC->APB2ENR |= (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC);
    #if (defined A12_901)||(defined A12_909)
    //LED
    GPIOC->CRH &= (GPIO_Crh_P15);
    GPIOC->CRH |= (GPIO_Mode_Out_PP_50MHz_P15);
    //VALVE OPTO
    GPIOA->CRH &= (GPIO_Crh_P15);
    GPIOA->CRH |= (GPIO_Mode_IN_PU_PD_P15);
    GPIOA->ODR |= (GPIO_Pin_15);
    GPIOA->CRL &= (GPIO_Crl_P4 & GPIO_Crl_P5 & GPIO_Crl_P6 & GPIO_Crl_P7);
    GPIOA->CRL |= (GPIO_Mode_Out_PP_50MHz_P4 | GPIO_Mode_Out_PP_50MHz_P5 | GPIO_Mode_Out_PP_50MHz_P6 | GPIO_Mode_Out_PP_50MHz_P7);
    // ISET
    GPIOB->CRL &= (GPIO_Crl_P0);
    GPIOB->CRL |= (GPIO_Mode_IN_PU_PD_P0);
    GPIOB->ODR |= (GPIO_Pin_0);
    GPIOB->CRH &= (GPIO_Crh_P12);
    GPIOB->CRH |= (GPIO_Mode_IN_PU_PD_P12);
    GPIOB->ODR |= (GPIO_Pin_12);
    GPIOA->CRH &= (GPIO_Crh_P11);
    GPIOA->CRH |= (GPIO_Mode_IN_PU_PD_P11);
    GPIOA->ODR |= (GPIO_Pin_11);
    #endif

    #ifdef A12_906
    //LED
    GPIOC->CRH &= (GPIO_Crh_P14);
    GPIOC->CRH |= (GPIO_Mode_Out_PP_50MHz_P14);
    //VALVE OPTO
    GPIOC->CRH &= (GPIO_Crh_P15);
    GPIOC->CRH |= (GPIO_Mode_IN_PU_PD_P15);
    GPIOC->ODR |= (GPIO_Pin_15);
    GPIOA->CRL &= (GPIO_Crl_P4 & GPIO_Crl_P5 & GPIO_Crl_P6 & GPIO_Crl_P7);
    GPIOA->CRL |= (GPIO_Mode_Out_PP_50MHz_P4 | GPIO_Mode_Out_PP_50MHz_P5 | GPIO_Mode_Out_PP_50MHz_P6 | GPIO_Mode_Out_PP_50MHz_P7);
    // ISET
    GPIOB->CRH &= (GPIO_Crh_P12);
    GPIOB->CRH |= (GPIO_Mode_IN_PU_PD_P12);
    GPIOB->ODR |= (GPIO_Pin_12);
    GPIOB->CRL &= (GPIO_Crl_P0);
    GPIOB->CRL |= (GPIO_Mode_IN_PU_PD_P0);
    GPIOB->ODR |= (GPIO_Pin_0);
    GPIOA->CRH &= (GPIO_Crh_P11);
    GPIOA->CRH |= (GPIO_Mode_IN_PU_PD_P11);
    GPIOA->ODR |= (GPIO_Pin_11);

    #endif

    VALVE_RST = 0;
    delay_ms(10);
    VALVE_RST = 1;

    srd[AXXN].signalDIR = &VALVE_DIR;
    srd[AXXN].signalCLK = &VALVE_CLK;
    srd[AXXN].signalCCR1 = &TIM4->CCR1;
    srd[AXXN].signalARR = &TIM4->ARR;
    srd[AXXN].signalCR1 = &TIM4->CR1;
    srd[AXXN].SearchOrg = ValveOrg;
    srd[AXXN].bEmgStop = &valve.bEmgStopV;
	#ifndef A12_901
    ISET(I_18A);
	#endif
}


/*

*/
void InitValve(void)
{
    float ftemp=0.0;
    if(valve.status&VALVE_INITING)
    {
        switch(valve.initStep)
        {
            case 0:
                if(valve.retryTms<RETRY_TIMES)
                {
//                    printd("\r start round");
                    VALVE_ENA = ENABLE;
//                    ISET(valve.iSet);
                    ++valve.retryTms;
                    I2CPageRead_Nbytes(ADDR_NOW_POS, LEN_NOW_POS, &valve.portLast);
                    valve.initStep = 1;
                }
                break;
            case 1:
                if(!MotionStatus[AXSV])
                {
                    if(!VALVE_OPT || (valve.portLast==POS_B&&!VALVE_OPT) || (valve.portLast!=POS_A&&VALVE_OPT))
                    {// 未挡住，或者挡住了但上一个位置是B
                        AxisMoveRel(AXSV, rdc.stepRound, accel[AXSV], decel[AXSV], speed[AXSV]);
                        valve.initStep = 3;
                    }
                    else
                    {// 反转半个角度值

                        ftemp = (float)rdc.stepRound/valveFix.fix.portCnt;
                        ftemp /= 2;
                        AxisMoveRel(AXSV, -ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                        valve.initStep = 2;
                    }
                }
                break;
            case 2:
                if(!VALVE_OPT)
                {
                    srd[AXSV].accel_count = -rdc.stepP1dgr;
                    srd[AXSV].run_state = DECEL;
                    valve.initStep = 1;
                }
                break;
            case 3:
                if(!VALVE_OPT)
                    valve.initStep = 4;
                break;
            case 4:
                valve.initStep = 5;
                break;
            case 5:
                #ifdef IOCTRL
                if(bIoCtrl)
                {
                    valve.initStep = 6;
//                #else
                }
                else
                {
                    if(!MotionStatus[AXSV])
                    {
                        if(valve.bHalfSeal)
                        {
                            // 复位密封
                            ftemp = (float)rdc.stepRound/valveFix.fix.portCnt;
                            ftemp /= 2;
                            AxisMoveRel(AXSV, -(int)ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                        }
                        valve.bNewInit = 1;
                        valve.initStep = 6;
                    }
                }
                #endif
                break;
            case 6:
                if(!MotionStatus[AXSV])
                {
                    valve.bReInit = 1;
                    valve.initStep = 0;
                    valve.portDes = 0;
                    #ifdef IOCTRL
                    if(bIoCtrl)
                    {
                        valve.portCur = POS_A;
                    }
                    else
                    {
                        if(valve.bHalfSeal)
                            valve.portCur = POS_M;
                        else
                            valve.portCur = POS_A;
                        I2CPageWrite_Nbytes(ADDR_NOW_POS, LEN_NOW_POS, &valve.portCur);
                    }
                    #endif
                    valve.status &= ~VALVE_INITING;
                    valve.status &= ~VALVE_RUNNING;
                    valve.status |= VALVE_RUN_END;
                    VALVE_ENA = DISABLE;
//                    ISET(I_05A);
                    // 清时间，保证不会连续复位转动
//                    timerPara.timeMilli = 0;
//                    printd("\r NO.%d", valveFix.fix.portCnt);
                    printd("\r\n spd%d acc%d dec%d", speed[AXSV], accel[AXSV], decel[AXSV]);
                }
                break;
            default:
                break;
        }
    }
}

/*
    初始化完成后进行通道寻位，预给出两圈的行程，肯定会找到两次目标位置
    找到一个通道点，位置加1，直到找到目标位置，激活急停
    如果超过一圈没有找到目标位置，启动重新初始化，重新初始化的次数超过3次，报错退出
*/
void ProcessValve(void)
{
    float ftemp=0;
    if(!(valve.status&VALVE_INITING))
	{
        if(!MotionStatus[AXSV])
        {
            if(valve.status==VALVE_RUN_END)
        	{
                if(valve.portCur!=valve.portDes && (valve.portDes==POS_A||valve.portDes==POS_B))
        	    {
                    syspara.lastTime = 0;
                    ftemp = (float)rdc.stepRound/valveFix.fix.portCnt;
                    if(valve.bNewInit)
                    {
                        if(valve.bHalfSeal)
                            ftemp /= 2;
                    }
                    if(valve.portDes==POS_A)
                    {
                        ftemp *= 2;
                        valve.initStep = 4;
                    }
                    else
                    {
                        ftemp += (float)valveFix.fix.dirGap*rdc.stepP01dgr;
                        ftemp *= -1;
                    }
                    VALVE_ENA = ENABLE;
                    if(valve.bNewInit)
                    {
                        valve.bNewInit = 0;
                        AxisMoveRel(AXSV, (int)ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                    }
                    else
                    {
                        AxisMoveAbs(AXSV, (int)ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                    }
                    // 清空计数，避免数据暂留
    				valve.status &= ~VALVE_RUN_END; 	// 清除运行结束标志
    				valve.status |= VALVE_RUNNING; 	    // 置位运行标志
                    valve.statusLast = VALVE_RUNNING;
                    syspara.protectTimeOut = 0;
                }
        	}
            else if(valve.statusLast==VALVE_RUNNING)
            {
                valve.statusLast = 0;
                if(valve.portDes==POS_A && !VALVE_OPT)
                {
                    valve.portDes = POS_N;
                    valve.status = VALVE_RUN_ERR;
                    VALVE_ENA = DISABLE;
                    printd("\r\n signal err");
                    return;
                }
                I2CPageWrite_Nbytes(ADDR_NOW_POS, LEN_NOW_POS, &valve.portDes);
                valve.portCur = valve.portDes;
                valve.portDes = POS_N;
                valve.retryTms = 0;
                valve.status = VALVE_RUN_END;
                ++syspara.totalCnt;
    //                ISET(I_05A);
                VALVE_ENA = DISABLE;
//                EnableReceive();
            }
        }
    }
}

/*
    原点与端口光耦的信号激时，分别激活急停功能，确保停止的位置够精确
*/
void ValveOrg(void)
{
    if(VALVE_OPT && valve.optLast==1)
    {
        valve.optLast = 0;
        position[AXSV] = -rdc.stepP1dgr*valveFix.fix.org;
        srd[AXSV].accel_count = -rdc.stepP1dgr*valveFix.fix.org;
        srd[AXSV].run_state = DECEL;
        if(valve.status&VALVE_INITING && valve.initStep==4)
        {// 初始化完成，找到10号位原点
            valve.initStep = 5;
        }
        else if(valve.portDes==POS_A)
        {
            valve.initStep = 0;
        }
    }
    else if(!VALVE_OPT)
    {
        valve.optLast = 1;
    }
}


/*
    设置地址为64号的时候，模块会自动启动烧机测试模式
*/
void TestBurn(void)
{
    static uint8 tmWait=0;
    if(ModbusPara.mAddrs==ADDR_MAX)
    {
        if(timerPara.timeWaitMill>SEC)
        {
            timerPara.timeWaitMill = 0;
            if(++tmWait>intCtrl)
            {// 30秒间隔，启动模块运转到下一个通道
                tmWait = 0;
                if(valve.status==VALVE_RUN_END)
                {
                    if(valve.portCur!=POS_A)
                    {
                        valve.initStep = 4;
                        valve.portDes = POS_A;
                    }
                    else
                        valve.portDes = POS_B;
                }
            }
        }
    }
}


