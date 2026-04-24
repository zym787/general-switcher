#define _MOTOR_GLOBALS_
#include "common.h"

void bsp_ValveGpioInit(void)
{
        RCC->APB2ENR |= (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC);

#if (defined A12_901) || (defined A12_909)
        /*  LED  PC15 */
        GPIOC->CRH &= (GPIO_Crh_P15);
        GPIOC->CRH |= (GPIO_Mode_Out_PP_50MHz_P15);
        /*  光感  PA15 */
        GPIOA->CRH &= (GPIO_Crh_P15);
        GPIOA->CRH |= (GPIO_Mode_IN_PU_PD_P15);
        GPIOA->ODR |= (GPIO_Pin_15);
        /*  驱动信号  PA4 PA6 PA7*/
        GPIOA->CRL &= (GPIO_Crl_P4 & GPIO_Crl_P6 & GPIO_Crl_P7);
        GPIOA->CRL |= (GPIO_Mode_Out_PP_50MHz_P4 | GPIO_Mode_Out_PP_50MHz_P6 | GPIO_Mode_Out_PP_50MHz_P7);
#ifdef A12_909
        /* ISET PB0 PB12 PA11 */
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
#endif

#ifdef A12_906
        /*  LED  PC14 */
        GPIOC->CRH &= (GPIO_Crh_P14);
        GPIOC->CRH |= (GPIO_Mode_Out_PP_50MHz_P14);
        /*  光感  PC15 */
        GPIOC->CRH &= (GPIO_Crh_P15);
        GPIOC->CRH |= (GPIO_Mode_IN_PU_PD_P15);
        GPIOC->ODR |= (GPIO_Pin_15);
        /*  驱动信号  PA4 PA5 PA6 PA7 */
        GPIOA->CRL &= (GPIO_Crl_P4 & GPIO_Crl_P5 & GPIO_Crl_P6 & GPIO_Crl_P7);
        GPIOA->CRL |= (GPIO_Mode_Out_PP_50MHz_P4 | GPIO_Mode_Out_PP_50MHz_P5 | GPIO_Mode_Out_PP_50MHz_P6 |
                       GPIO_Mode_Out_PP_50MHz_P7);
        /* ISET PB12 PB0 PA11 */
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

        /* 驱动库配置 */
        srd[AXXN].signalDIR = &VALVE_DIR;
        srd[AXXN].signalCLK = &VALVE_CLK;
        srd[AXXN].signalCCR1 = &TIM4->CCR1;
        srd[AXXN].signalARR = &TIM4->ARR;
        srd[AXXN].signalCR1 = &TIM4->CR1;
        srd[AXXN].SearchOrg = bsp_ValveOrigin;
        srd[AXXN].bEmgStop = &valve.bEmgStopV;

        /* 电流设置 906/909  0 最大 */
#ifndef A12_901
        ISET(I_26A);
#endif
}

/*
 * 阀门初始化 - 状态机各步骤处理函数
 * 将各步骤拆分为独立静态函数，消除重复代码，提升可维护性
 */

/* 方向系数宏：根据DIRECTION_SWITCH统一处理电机方向正负 */
#if DIRECTION_SWITCH == 1
#define DIR_ADJ(steps) (steps) /* DIRECTION_SWITCH=1时，直接使用原始方向 */
#else
#define DIR_ADJ(steps) (-(steps)) /* DIRECTION_SWITCH=0时，方向整体取反 */
#endif

/**
 * @brief 计算半通道密封移动步数
 * @return 半通道对应的步数值（float）
 */
static void _calcHalfSealSteps(float *_ftemp) { *_ftemp = (float)rdc.stepRound / valveFix.fix.portCnt / 2.0f; }

/**
 * @brief 判断当前是否应跳过半通道逻辑
 * @return true-跳过半通道（IO控制生效时）; false-执行半通道
 */
static bool _shouldSkipHalfSeal(void)
{
#ifdef IOCTRL
        return (ON == syspara.ioCtrl);
#else
        return false;
#endif
}

/**
 * @brief 恢复电机运行速度为设定值
 */
void bsp_ValveUpdateSpeed(uint8_t _spd)
{
        speed[AXSV] = 100 * _spd * rdc.rate;
        accel[AXSV] = 100 * _spd * rdc.rate;
        decel[AXSV] = 200 * _spd * rdc.rate;
        printd(" 更新速度到%dRPM  spd%d acc%d dec%d", _spd, speed[AXSV], accel[AXSV], decel[AXSV]);
}

/**
 * @brief 初始化步骤0：启动重试逻辑
 * @details 使能电机，增加重试计数，读取上次位置，进入步骤1
 */
static void _initStep0_Retry(void)
{
        if (RETRY_TIMES <= valve.retryTms) {
                return;
        }
        VALVE_ENA = ENABLE;
        ++valve.retryTms;
        I2CPageRead_Nbytes(ADDR_NOW_POS, LEN_NOW_POS, &valve.portLast);
        valve.initStep = 1;
        if (1 == valve.retryTms) {
                printd("\r Init ");
        } else {
                valve.ErrBlinkTime = RETRY_TIME_OUT;
                printd("\r RETRY %d %d", valve.retryTms, RETRY_TIME_OUT);
        }
}

/**
 * @brief 初始化步骤1：根据光感信号决定电机移动方向
 * @details 光感未挡住时正向转一圈寻找缺口；挡住时反向转半通道脱离挡光片
 */
static void _initStep1_DirectionDecision(void)
{
        if (MotionStatus[AXSV]) {
                return; /* 电机仍在运动中，等待完成 */
        }

        if (OPT_GAP == VALVE_OPT) {
                /* 未挡住：正向转一圈寻找挡光片/缺口 */
                AxisMoveRel(AXSV, DIR_ADJ(-rdc.stepRound), accel[AXSV], decel[AXSV], speed[AXSV]);
                dbg_printf("\r MoveGap (%d) step%d", valve.initStep, DIR_ADJ(-rdc.stepRound));
                valve.initStep = 3;
        } else {
                /* 挡住：反向转半通道脱离 */
                float ftemp = (float)rdc.stepRound / valveFix.fix.portCnt / 2.0f;
                AxisMoveRel(AXSV, DIR_ADJ(ftemp), accel[AXSV], decel[AXSV], speed[AXSV]);
                dbg_printf("\r MoveBack (%d) step%d", valve.initStep, DIR_ADJ(ftemp));
                valve.initStep = 2;
        }
}

/**
 * @brief 初始化步骤2：调整减速或确认位置
 * @details 未挡住时触发减速；仍挡住则标记为B位置
 */
static void _initStep2_AdjustDecel(void)
{
        if (OPT_GAP == VALVE_OPT) {
                /* 未挡住：立即减速 */
                srd[AXSV].accel_count = -rdc.stepP1dgr;
                srd[AXSV].run_state = DECEL;
                valve.initStep = 1;
                dbg_printf("\r Decel (%d)", valve.initStep);
        } else {
                /* 回转后仍触发光耦：标记当前为B位置 */
                valve.portLast = POS_B;
                valve.initStep = 1;
                dbg_printf("\r Pos B (%d)", valve.initStep);
        }
}

/**
 * @brief 初始化步骤3：等待脱离光耦触发区
 */
static void _initStep3_WaitGap(void)
{
        if (OPT_GAP == VALVE_OPT) {
                valve.initStep = 4;
        }
}

/**
 * @brief 初始化步骤5：执行半通道密封（如启用）
 * @details IO控制模式下跳过半通道；否则在电机停止后执行半通道移动
 */
static void _initStep5_HalfSeal(void)
{
        if (_shouldSkipHalfSeal()) {
                valve.initStep = 6;
                return;
        }

        if (MotionStatus[AXSV]) {
                return;
        }

        if (ON == valve.bHalfSeal) {
                float ftemp = 0;
                _calcHalfSealSteps(&ftemp);

                AxisMoveRel(AXSV, (int)DIR_ADJ(ftemp), accel[AXSV], decel[AXSV], speed[AXSV]);
                dbg_printf("\r Half Seal (%d) step%d", valve.initStep, (int)DIR_ADJ(ftemp));
        }
        valve.bNewInit = 1;
        valve.initStep = 6;
}

/**
 * @brief 初始化步骤6：更新当前位置状态
 * @details 根据IO控制和半通道配置确定最终位置，写入EEPROM
 */
static void _initStep6_UpdatePosition(void)
{
        if (MotionStatus[AXSV]) {
                return;
        }

        valve.portDes = 0;

        if (_shouldSkipHalfSeal()) {
                valve.portCur = POS_A;
        } else if (ON == valve.bHalfSeal) {
                valve.portCur = POS_M;
        } else {
                valve.portCur = POS_A;
        }

        if (!_shouldSkipHalfSeal()) {
                I2CPageWrite_Nbytes(ADDR_NOW_POS, LEN_NOW_POS, &valve.portCur);
        }

        valve.initStep = 7;
}

/**
 * @brief 初始化步骤7：完成初始化，恢复工作状态
 */
static void _initStep7_Complete(void)
{
        if (MotionStatus[AXSV]) {
                printd("\r\n Fatal Error!");
                return;
        }

        /* 清除状态标志 */
        valve.status &= ~VALVE_INITING;
        valve.status &= ~VALVE_RUNNING;
        valve.status |= VALVE_RUN_END;
        VALVE_ENA = DISABLE;
        valve.initStep = 0;

        /* 输出当前位置 */
        char posChar = (POS_A == valve.portCur)   ? 'A'
                       : (POS_B == valve.portCur) ? 'B'
                       : (POS_M == valve.portCur) ? 'M'
                                                  : 'X';
        printd("\r POS %c", posChar);

        /* 恢复电机速度到设定值 */
        printd("\r\n 恢复电机速度");
        bsp_ValveUpdateSpeed(valve.spd);
}

/*
 * 阀门初始化 - 主状态机入口
 */
void bsp_ValveInit(void)
{
        if (!(valve.status & VALVE_INITING)) {
                return;
        }

        switch (valve.initStep) {
                        /* 初始化重试逻辑 */
                case 0:
                        _initStep0_Retry();
                        break;
                        /* 根据光感信号决定移动方向 */
                case 1:
                        _initStep1_DirectionDecision();
                        break;
                        /* 未挡住时调整减速 */
                case 2:
                        _initStep2_AdjustDecel();
                        break;
                case 3:
                        _initStep3_WaitGap();
                        break;
                        /* 是否执行半通道 */
                case 5:
                        _initStep5_HalfSeal();
                        break;
                        /* 更新状态 */
                case 6:
                        _initStep6_UpdatePosition();
                        break;
                case 7:
                        _initStep7_Complete();
                        break;
                default:
                        break;
        }
}

/*
    初始化完成后进行通道寻位,预给出两圈的行程,肯定会找到两次目标位置
    找到一个通道点,位置加1,直到找到目标位置,激活急停
    如果超过一圈没有找到目标位置,启动重新初始化,重新初始化的次数超过3次,报错退出
*/
/* 提前计算移动步数 */
static void _calcMoveSteps(float *_ftemp)
{
        *_ftemp = (float)rdc.stepRound / valveFix.fix.portCnt; /* 单通道切换步数 */

        if (valve.bNewInit && valve.bHalfSeal) /* 半通道补偿 */
                *_ftemp /= 2;

        if (valve.portDes == POS_A) /* B->A / Half->A */
        {
                *_ftemp *= 2;
                valve.initStep = 4;
        } else /* A->B / Half->B */
        {
                *_ftemp += (float)valveFix.fix.dirGap * rdc.stepP01dgr; /* 方向补偿 */
                *_ftemp *= -1;
        }
}

/* 执行电机移动 */
static void _doAxisMove(float _ftemp)
{
        if (valve.bNewInit) /* 刚复位完成,用相对移动 */
        {
                valve.bNewInit = 0;
#if DIRECTION_SWITCH == 1
                AxisMoveRel(AXSV, -(int)_ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                dbg_printf("\r New Init Rel (%d) step%d", valve.initStep, -(int)_ftemp);
#else
                AxisMoveRel(AXSV, (int)_ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                dbg_printf("\r New Init Rel (%d) step%d", valve.initStep, (int)_ftemp);
#endif
        } else /* 否则绝对移动 */
        {
#if DIRECTION_SWITCH == 1
                AxisMoveAbs(AXSV, -(int)_ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                dbg_printf("\r No New Init Abs (%d) step%d", valve.initStep, -(int)_ftemp);
#else
                AxisMoveAbs(AXSV, (int)_ftemp, accel[AXSV], decel[AXSV], speed[AXSV]);
                dbg_printf("\r No New Init Abs (%d) step%d", valve.initStep, (int)_ftemp);
#endif
        }
}

/**
 * @brief    阀组空闲状态处理
 */
static void _handleValveIdle(void)
{
        float ftemp = 0;

        /* 目标位置无效或未变化,跳过 */
        if (valve.portCur == valve.portDes || (POS_A != valve.portDes && POS_B != valve.portDes)) {
                return;
        }

        syspara.lastTime = 0;          /* 清空切换时间 */
        syspara.bCountLastTime = true; /* 开始运行计时 */

        _calcMoveSteps(&ftemp);

        VALVE_ENA = ENABLE;
        _doAxisMove(ftemp);

        /* 清空计数,避免数据暂留 */
        valve.status &= ~VALVE_RUN_END; /* 清除运行结束标志 */
        valve.status |= VALVE_RUNNING;  /* 置位运行标志 */
        valve.statusLast = VALVE_RUNNING;
        syspara.protectTimeOut = 0; /* 重新开始单次保护计时 */
        dbg_printf("\r\n %s initstep%d (%d) ststus%02x", __FUNCTION__, valve.initStep, syspara.protectTimeOut,
                   valve.status);
}

/**
 * @brief    阀组运行结束处理
 */
static void _handleValveRunDone(void)
{
        valve.statusLast = VALVE_NONE; /* 清空状态 */

        /* 光耦信号异常检测 */
        if (POS_A == valve.portDes && (OPT_GAP == VALVE_OPT)) {
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
        ++syspara.totalCnt;             /* 切换次数+1 */
        syspara.bCountLastTime = false; /* 暂停切换时间计时 */
        VALVE_ENA = DISABLE;
        printd("\r\n 移动耗时: %dms", syspara.lastTime);
}

/**
 * @brief    阀运行
 */
void bsp_ValveProcess(void)
{
        /* 正在初始化或电机运动中,直接返回 */
        if ((valve.status & VALVE_INITING) || MotionStatus[AXSV]) {
                return;
        }

        if (VALVE_RUN_END == valve.status) {
                _handleValveIdle(); /* 阀组空闲状态 */
        } else if (VALVE_RUNNING == valve.statusLast) {
                _handleValveRunDone(); /* 阀组上次运行状态 */
        }
}

/*
 *  原点与端口光耦的信号激时,分别激活急停功能,确保停止的位置够精确
 */
void bsp_ValveOrigin(void)
{
        if ((OPT_BLOCKER == VALVE_OPT) && OPT_BLOCKER == valve.optLast) {
                valve.optLast = 0;
                position[AXSV] = -rdc.stepP1dgr * valveFix.fix.org;
                srd[AXSV].accel_count = -rdc.stepP1dgr * valveFix.fix.org;
                srd[AXSV].run_state = DECEL;
                if (valve.status & VALVE_INITING && 4 == valve.initStep) {
                        valve.initStep = 5; /* 初始化完成,找到原点 */
                } else if (POS_A == valve.portDes) {
                        valve.initStep = 0;
                }
        } else if (OPT_GAP == VALVE_OPT) {
                valve.optLast = OPT_BLOCKER;
        }
}

/*
 *  设置地址为64号的时候,模块会自动启动烧机测试模式
 */
void bsp_ValveAgingMode(void)
{
        static uint8_t tmWait = 0; /* 老化间隔记时器 */
        static uint8_t bFirstEnter = 0;
#ifndef AGING_MODE
        /* AGS协议 */
        if (syspara.protocol_type == AGS_MODBUS && BURN_ADDR == ags_mbParam.mAddrs) {
        }
        /* Modbus */
        else if (syspara.protocol_type == MODBUS && GD_AGING == syspara.GodMode) {
        } else {
                return;
        }
        if (0 == bFirstEnter) {
                printd("\r\n 进入烧机测试模式 开始循环切换!");
                bFirstEnter = 1;
        }
        if (timerPara.timeWaitMill < SEC) {
                return;
        }

        timerPara.timeWaitMill = 0;
        if (++tmWait > syspara.agingInterval) {
                // 30秒间隔,启动模块运转到下一个通道
                tmWait = 0;
                if (valve.status == VALVE_RUN_END) {
                        if (valve.portCur != POS_A) {
                                valve.initStep = 4;
                                valve.portDes = POS_A;
                        } else
                                valve.portDes = POS_B;
                        syspara.burnCnt++;
                        if (0 == syspara.burnCnt % 10) {
                                printd("  老化次数:%d(断电保存)", syspara.burnCnt);
                                I2CPageWrite_Nbytes(ADDR_BURN_CNT, LEN_BURN_CNT, (uint8_t *)&syspara.burnCnt);
                        }
                }
        }
#else
        static uint8_t dirFlag = 0; /* 方向标志位,0:正转,1:反转 */
        if (MOTOR_AGING_ADDR != ags_mbParam.mAddrs) {
                return;
        }
        if (timerPara.timeWaitMill < SEC) {
                return;
        }
        timerPara.timeWaitMill = 0;

        /* 更新电机速度到设定值 */
        bsp_ValveUpdateSpeed(valve.spd);

        if (++tmWait > syspara.agingInterval) {
                // n秒间隔,启动模块运转到下一个通道
                tmWait = 0;
                ++syspara.totalCnt;
                if (0 == dirFlag) {
                        /* 正转一圈 */
                        dirFlag = 1;
#if DIRECTION_SWITCH == 1
                        AxisMoveRel(AXSV, -rdc.stepRound, accel[AXSV], decel[AXSV], speed[AXSV]); /* 电机相对移动一圈 */
                        printd("\r\n - 反转1圈  CCW step%d", -rdc.stepRound);
#else
                        AxisMoveRel(AXSV, rdc.stepRound, accel[AXSV], decel[AXSV], speed[AXSV]); /* 电机相对移动一圈 */
                        printd("\r\n + 正转1圈  CW step%d", rdc.stepRound);
#endif
                        VALVE_ENA = ENABLE;
                } else if (1 == dirFlag) {
                        /* 反转一圈 */
                        dirFlag = 0;
#if DIRECTION_SWITCH == 1
                        AxisMoveRel(AXSV, rdc.stepRound, accel[AXSV], decel[AXSV], speed[AXSV]);
                        printd("\r\n + 正转1圈  CW step%d", rdc.stepRound);
#else
                        AxisMoveRel(AXSV, -rdc.stepRound, accel[AXSV], decel[AXSV], speed[AXSV]);
                        printd("\r\n - 反转1圈  CCW step%d", -rdc.stepRound);
#endif
                        VALVE_ENA = ENABLE;
                }
                printd("  切换次数:%d", syspara.totalCnt);
                I2CPageWrite_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, (uint8 *)&syspara.totalCnt);
        }
#endif
}
