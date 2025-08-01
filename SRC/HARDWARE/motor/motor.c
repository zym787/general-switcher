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
    /* 电流设置 906/909  0 最大 */
#ifndef A12_901
    ISET(I_26A);
#endif
}


/*
 * 阀门初始化
 */
void InitValve(void)
{
    float ftemp = 0.0;
    if(valve.status & VALVE_INITING)
    {
        switch(valve.initStep)
        {
            case 0:     /* 初始化重试逻辑 */
                if(RETRY_TIMES > valve.retryTms)
                {
//                    printd("\r start round");
                    VALVE_ENA = ENABLE;
//                    ISET(valve.iSet);
                    ++valve.retryTms;
                    I2CPageRead_Nbytes(ADDR_NOW_POS, LEN_NOW_POS, &valve.portLast);
                    valve.ErrBlinkTime = RETRY_TIME_OUT;
                    printd("\r RETRY %d %d", valve.retryTms, RETRY_TIME_OUT);
                    valve.initStep = 1;
                    valve.ErrBlinkTime = RETRY_TIME_OUT;
                    printd("\r RETRY %d %d", valve.retryTms, RETRY_TIME_OUT);
                }
                break;
            case 1:     /* 根据光感信号决定移动方向 */
                if(!MotionStatus[AXSV])
                {
                    /* 未挡住,或者挡住了但上一个位置是B */
                    if  ((OPT_GAP == VALVE_OPT) ||
                        ((OPT_GAP == VALVE_OPT) && valve.portLast == POS_B) ||
                        ((OPT_BLOCKER == VALVE_OPT) && (valve.portLast!=POS_A)))
                    {
                        AxisMoveRel(AXSV, rdc.stepRound, accel[AXSV], decel[AXSV], speed[AXSV]);    /* 电机相对移动一圈 */
                        valve.initStep = 3;
                    }
                    /* 挡住,或者挡住了但上一个位置是A */
                    else
                    {
                        /* 反转半个通道角度值 */
                        ftemp = (float)rdc.stepRound / valveFix.fix.portCnt / 2;
                        AxisMoveRel(AXSV, -ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);   /* 电机反向移动半个通道角度 */
                        valve.initStep = 2;
                    }
                }
                break;
            case 2:     /* 未挡住时调整减速 */
                if(OPT_GAP == VALVE_OPT)   /* 未挡住 */
                {
                    srd[AXSV].accel_count = -rdc.stepP1dgr;
                    srd[AXSV].run_state = DECEL;
                    valve.initStep = 1;
                }
                break;
            case 3:
                if(OPT_GAP == VALVE_OPT)   /* 未挡住 */
                    valve.initStep = 4;
                break;
            case 4:
                valve.initStep = 5; /* 初始化完成 */
                break;
            case 5:         /* 是否执行半通道 */
#ifdef IOCTRL
                if(OFF == bIoCtrl) // IOE IO不生效时才有半通道
                {
                    if(!MotionStatus[AXSV])
                    {
                        /* 半通道生效 复位密封 */
                        if(valve.bHalfSeal)
                        {
                            // 复位密封 半通道
                            ftemp = (float)rdc.stepRound/valveFix.fix.portCnt / 2;
                            AxisMoveRel(AXSV, -(int)ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                        }
                        valve.bNewInit = 1;
                    }
                }
#endif
                valve.initStep = 6;
                break;
            case 6:         /* 更新状态 */
                if(!MotionStatus[AXSV])
                {
                    valve.bReInit = 1;
                    valve.initStep = 0;
                    valve.portDes = 0;
#ifdef IOCTRL
                    if(bIoCtrl) // IO默认到A
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
                    valve.status &= ~VALVE_INITING; // 清除初始化标志
                    valve.status &= ~VALVE_RUNNING; // 清除运行标志
                    valve.status |= VALVE_RUN_END;  // 运行结束--空闲
                    VALVE_ENA = DISABLE;            // 停止电机
//                    ISET(I_05A);
                    // 清时间,保证不会连续复位转动
//                    timerPara.timeMilli = 0;
//                    printd("\r NO.%d", valveFix.fix.portCnt);
                    speed[AXSV] = 100;
                    accel[AXSV] = 100;
                    decel[AXSV] = 200;
                    speed[AXSV] *= (spdVx2);    // 恢复设定速度
                    speed[AXSV] *= (rdc.rate);
                    accel[AXSV] *= (spdVx2);
                    accel[AXSV] *= (rdc.rate);
                    decel[AXSV] *= (spdVx2);
                    decel[AXSV] *= (rdc.rate);
                    printd("\r\n Restore motion speed  (%d) spd%d acc%d dec%d",
                           spdVx2, speed[AXSV], accel[AXSV], decel[AXSV]);
                }
                break;
            default:
                break;
        }
    }
}

/*
    初始化完成后进行通道寻位,预给出两圈的行程,肯定会找到两次目标位置
    找到一个通道点,位置加1,直到找到目标位置,激活急停
    如果超过一圈没有找到目标位置,启动重新初始化,重新初始化的次数超过3次,报错退出
*/
void ProcessValve(void)
{
    float ftemp=0;
    if(!(valve.status&VALVE_INITING))
    {
        if(!MotionStatus[AXSV])
        {
            if(VALVE_RUN_END == valve.status)           /* 阀组空闲状态 */
            {
                if  (valve.portCur != valve.portDes &&
                    (POS_A == valve.portDes || POS_B ==valve.portDes))
                {
                    syspara.lastTime = 0;   /* 清空切换时间 */
                    ftemp = (float)rdc.stepRound / valveFix.fix.portCnt;    /* 单通道切换步数 */
                    if(valve.bNewInit)
                    {
                        if(valve.bHalfSeal)     /* 半通道 */
                            ftemp /= 2;
                    }
                    if(valve.portDes==POS_A)    /* B->A / Half->A */
                    {
                        ftemp *= 2;
                        valve.initStep = 4;
                    }
                    else                        /* A->B / Half->B */
                    {
                        ftemp += (float)valveFix.fix.dirGap*rdc.stepP01dgr; /* 方向补偿 */
                        ftemp *= -1;
                    }
                    VALVE_ENA = ENABLE;
                    if(valve.bNewInit)  /* 刚复位完成,用相对移动 */
                    {
                        valve.bNewInit = 0;
                        AxisMoveRel(AXSV, (int)ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                    }
                    else                /* 否则绝对移动 */
                    {
                        AxisMoveAbs(AXSV, (int)ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                    }
                    // 清空计数,避免数据暂留
                    valve.status &= ~VALVE_RUN_END;     /* 清除运行结束标志 */
                    valve.status |= VALVE_RUNNING;      /* 置位运行标志 */
                    valve.statusLast = VALVE_RUNNING;
                    syspara.protectTimeOut = 0;         /* 重新开始单次保护计时 */
                    printd("\r\n %s initstep%d (%d) ststus%02x",
                           __FUNCTION__, valve.initStep, syspara.protectTimeOut, valve.status);
                }
            }
            else if(VALVE_RUNNING == valve.statusLast)  /* 阀组上次运行状态 */
            {
                valve.statusLast = VALVE_NONE;      /* 清空状态 */
                if(POS_A == valve.portDes && (OPT_GAP == VALVE_OPT))
                {
                    valve.portDes = POS_N;
                    valve.status = VALVE_RUN_ERR;
                    valve.ErrBlinkTime = ERROR_BLINK;
                    VALVE_ENA = DISABLE;
                    printd("\r\n signal err");
                    return;
                }
                valve.portCur = valve.portDes;
                I2CPageWrite_Nbytes(ADDR_NOW_POS, LEN_NOW_POS, &valve.portCur);
                valve.portDes = POS_N;
                valve.retryTms = 0;
                valve.status = VALVE_RUN_END;
                ++syspara.totalCnt;     /* 切换次数+1 */
//                ISET(I_05A);
                VALVE_ENA = DISABLE;
//                EnableReceive();
            }
        }
    }
}


/*
 *  原点与端口光耦的信号激时,分别激活急停功能,确保停止的位置够精确
 */
void ValveOrg(void)
{
    if((OPT_BLOCKER == VALVE_OPT) && OPT_BLOCKER == valve.optLast)
    {
        valve.optLast = 0;
        position[AXSV] = - rdc.stepP1dgr*valveFix.fix.org;
        srd[AXSV].accel_count = - rdc.stepP1dgr*valveFix.fix.org;
        srd[AXSV].run_state = DECEL;
        if(valve.status & VALVE_INITING && 4 == valve.initStep)
        {
            valve.initStep = 5; /* 初始化完成,找到10号位原点 */
        }
        else if(POS_A == valve.portDes)
        {
            valve.initStep = 0;
        }
    }
    else if(OPT_GAP == VALVE_OPT)
    {
        valve.optLast = OPT_BLOCKER;
    }
}


/*
 *  设置地址为64号的时候,模块会自动启动烧机测试模式
 */
void TestBurn(void)
{
    static uint8 tmWait=0;
    if(BURN_ADDR == ModbusPara.mAddrs)
    {
        if(timerPara.timeWaitMill>SEC)
        {
            timerPara.timeWaitMill = 0;
            if(++tmWait>intCtrl)
            {
                // 30秒间隔,启动模块运转到下一个通道
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
