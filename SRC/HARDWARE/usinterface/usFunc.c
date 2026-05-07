#define _USFUNC_GLOBALS_
#include "common.h"

/*

*/
void TermVR(char rw)
{
    int getInt;
    if(rw==READ_ACT)
    {
        printd("\r\n %s", SOFT_VER_C);
    }
    else
    {
        unsigned char ret = FetchInt(2, 0, RxBuf, &getInt);
        if(ret)
        {
            return;
        }
    }
}

/**
 * @brief     : 任意地址读写接口
 * @param     : rw
 */
// void TermMap(char rw)
// {
//     char getChar[2][P_LEN+1];
//     int getInt=0;
//     int *getAddr=NULL;

//     memset(getChar, 0, sizeof(getChar));
//     if(rw==READ_ACT)
//     {
//         printd("\r\n No para");
//     }
//     else
//     {
//         unsigned char ret = FetchChar(3, 0, RxBuf, *getChar);
//         if(ret)
//         {
//             printd("\r\n Err code %d", ret);
//             return;
//         }
//         getAddr = (int *)strtohex(*getChar);
//         if(**(getChar+1))
//         {
//             //判断是否有参数写入
//             getInt = str2int(*(getChar+1));
//             *getAddr = getInt;
//             printd("\r\n Wr 0x%08x to 0x%08x", getInt, getAddr);
//         }
//         else
//         {
//             //无参数,取指
//             getInt = (int)(*getAddr);
//             printd("\r\n Rd 0x[%02x %02x %02x %02x] from 0x%08x",
//                    (char)(getInt>>24), (char)(getInt>>16), (char)(getInt>>8), (char)(getInt>>0), getAddr);
//         }
//     }
// }


/**
 * @brief     : 擦除EEPROM内数据
 * @param     : rw
 */
void TermIIC(char rw)
{
    unsigned char rwBuf[4]= {0, 0, 0, 0};
    int getInt=0;
    if(rw==READ_ACT)
    {
        printd("\r\n No para");
    }
    else
    {

        unsigned char ret = FetchInt(3, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r\n ERR%d", ret);
            return;
        }
        if(getInt)
        {
            *(rwBuf+0) = getInt>>8;
            *(rwBuf+1) = getInt;
            I2CPageWrite_Nbytes(0, 2, rwBuf);
            printd("\r\n I2C write 0x%02x 0x%02x", *(rwBuf+0), *(rwBuf+1));
            *(rwBuf+2) = 0;
            *(rwBuf+3) = 0;
            I2CPageRead_Nbytes(0, 2, rwBuf+2);
            printd("\r\n I2C read 0x%02x 0x%02x", *(rwBuf+2), *(rwBuf+3));
            if(*(rwBuf+0)==*(rwBuf+2) && *(rwBuf+1)==*(rwBuf+3))
                printd("\r\n I2C R/W Succeed!");
            else
                printd("\r\n I2C R/W Fail!");
        }
        else
        {
            *(rwBuf+0) = 0;
            *(rwBuf+1) = 0;
            I2CPageWrite_Nbytes(ADDR_BOARD_ID, LEN_BOARD_ID, rwBuf);
            printd("\r\n Clear store id!");
        }
    }
}


/*

*/
void TermReset(char rw)
{
    valve.status = VALVE_INITING;
    valve.initStep = 0;     /* 复位指令 */
    valve.retryTms = 0;
    valve.ErrBlinkTime = RETRY_TIME_OUT;
    printd("\r\n RESET!");
}


/*

*/
void TermMotorX(char rw)
{
    int getInt;
    if(rw == WRITE_ACT)
    {
        unsigned char ret = FetchInt(3, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r\n Err code %d", ret);
            return;
        }
        printd("\r MOT %d", getInt);
        if(!MotionStatus[AXSV])
        {
            AxisMoveRel(AXSV, rdc.stepP1dgr*getInt, accel[AXSV]/RT_INIT, decel[AXSV]/RT_INIT, speed[AXSV]);
        }
    }
}





/*

*/
void TermFixO(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &valveFix.fix.org);
        printd("\r 读出原点补偿:%d", valveFix.fix.org);
    }
    else
    {
        unsigned char ret = FetchInt(4, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        valveFix.fix.org = getInt;
        printd("\r 写入原点补偿:%d", valveFix.fix.org);
        I2CPageWrite_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &valveFix.fix.org);
    }
}



/*

*/
void TermFixG(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_DIR_FIX, LEN_DIR_FIX, &valveFix.fix.dirGap);
        printd("\r 读出方向补偿:%d", valveFix.fix.dirGap);
    }
    else
    {
        unsigned char ret = FetchInt(4, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        valveFix.fix.dirGap = getInt;
        printd("\r 写入方向补偿:%d", valveFix.fix.dirGap);
        I2CPageWrite_Nbytes(ADDR_DIR_FIX, LEN_DIR_FIX, &valveFix.fix.dirGap);
    }
}


/*

*/
void TermAddr(char rw)
{
    int getInt=0;
    if(rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, (uint8*)&getInt);
        printd("\r Addr:%d", getInt);
    }
    else
    {
        unsigned char ret = FetchInt(4, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if (AGS_ADDR_MIN <= getInt && BURN_ADDR >= getInt || MOTOR_AGING_ADDR == getInt)
        {
            ags_mbParam.mAddrs = getInt;
            printd("\r\n Set Addr to %d", ags_mbParam.mAddrs);
        }
        else
        {
            printd("\r\n %d Address out of range (%d-%d)", getInt, AGS_ADDR_MIN, BURN_ADDR);
            ags_mbParam.mAddrs = AGS_ADDR_DEF;
            printd("\r\n Use default Address %d", ags_mbParam.mAddrs);
        }
        modbus.Address = ags_mbParam.mAddrs;
        I2CPageWrite_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, &ags_mbParam.mAddrs);
    }
}


/*

*/
void TermCnt(char rw)
{
    int getInt=0;
    if(rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
        (CHANNEL_MIN > valveFix.fix.portCnt || CHANNEL_MAX < valveFix.fix.portCnt) ? 
            (valveFix.fix.portCnt = CHANNEL_DEF) : (valveFix.fix.portCnt);
        printd("\r Port Cnt:%d", valveFix.fix.portCnt);
    }
    else
    {
        unsigned char ret = FetchInt(3, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if(CHANNEL_MIN <= getInt && CHANNEL_MAX >= getInt)
        {
            valveFix.fix.portCnt = getInt;
            
            printd("\r\n set port cnt to %d", valveFix.fix.portCnt);
        }
        else
        {
            printd("\r\n %d Channel out of range (%d-%d)", getInt, CHANNEL_MIN, CHANNEL_MAX);
            valveFix.fix.portCnt = CHANNEL_DEF;
            printd("\r\n Use default Channel %d", valveFix.fix.portCnt);
        }
        I2CPageWrite_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
    }
}

/*

*/
void TermPos(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
    }
    else
    {
        unsigned char ret = FetchInt(3, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if(POS_A == getInt || POS_B == getInt)
        {
            if(valve.status==VALVE_RUN_END)
            {
                valve.portDes = getInt;
                printd("\r %d ==> %d",valve.portCur, valve.portDes);
            }
            else
            {
                printd("\r\n Busy, Ignore the request!");
            }
        }
        else
        {
            printd("\r\n pos overflow, switcher only two states");
            return;
        }
    }
}

/*

*/
void TermBaud(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_BAUD, LEN_BAUD, &syspara.baudrate);
        printd("\r 波特率:%d %dbps", syspara.baudrate, BaudRate_V[syspara.baudrate]);
    }
    else
    {
        unsigned char ret = FetchInt(3, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if(getInt%9600)
        {
            printd("\r\n baud rate must be an integer multiple of 9600, %d not exist", getInt);
            return;
        }
        if(9600 == getInt)
        {
            syspara.baudrate = BAUD_9600;
        }
        else if(19200 == getInt)
        {
            syspara.baudrate = BAUD_19200;
        }
        else if(38400 == getInt)
        {
            syspara.baudrate = BAUD_38400;
        }
        else
        {
            printd("\r\n baud rate overflow");
            return;
        }
        printd("\r set baud rate to %d %dbps", syspara.baudrate, getInt);
        I2CPageWrite_Nbytes(ADDR_BAUD, LEN_BAUD, &syspara.baudrate);
    }
}

/*

*/
void TermSpd(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
            I2CPageRead_Nbytes(ADDR_SPD, LEN_SPD, &valve.spd);
            printd("\r\n Speed %d", valve.spd);
    }
    else
    {
        unsigned char ret = FetchInt(3, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
#ifdef AGING_MODE
        if(0 <= getInt && 255 >= getInt)
        {
            valve.spd = getInt;
            printd("\r\n set speed to %d", valve.spd);
        }
        else
        {
            printd("\r\n %d Speed out of range (%dRDCR: %d-%d)", 
                getInt, rdc.rate, 0, 255);
            valve.spd = 30;
            printd("\r\n Use default Speed %d", valve.spd);
        }
#else
        if(RDC20 == rdc.rate)
        {
            if(SPD_MIN_RDCR20 <= getInt && SPD_MAX_RDCR20 >= getInt)
            {
                valve.spd = getInt;
                printd("\r\n set speed to %d", valve.spd);
            }
            else
            {
                printd("\r\n %d Speed out of range (%dRDCR: %d-%d)", 
                    getInt, rdc.rate, SPD_MIN_RDCR20, SPD_MAX_RDCR20);
                valve.spd = INIT_SPD / 2;
                printd("\r\n Use default Speed %d", valve.spd);
            }
        }
        else
        {
            if(SPD_MIN <= getInt && SPD_MAX >= getInt)
            {
                valve.spd = getInt;
                printd("\r\n set speed to %d", valve.spd);
            }
            else
            {
                printd("\r\n %d Speed out of range (%dRDCR: %d-%d)", 
                    getInt, rdc.rate, SPD_MIN, SPD_MAX);
                valve.spd = INIT_SPD;
                printd("\r\n Use default Speed %d", valve.spd);
            }
        }
#endif
        I2CPageWrite_Nbytes(ADDR_SPD, LEN_SPD, &valve.spd);
    }
}

/**
 * @brief     : IO控制开关
 * @param     : rw
 */
void TermIO(char rw)
{
        int getInt = 0;
        /* 无参数时直接切换IO状态 */
        if (rw == READ_ACT) {
                syspara.ioCtrl = !syspara.ioCtrl;
        } else {
                unsigned char ret = FetchInt(2, 0, RxBuf, &getInt);
                if (ret) {
                        printd("\r Err code %d", ret);
                        return;
                }
                if (0 == getInt || 1 == getInt) {
                        syspara.ioCtrl = getInt;
                } else {
                        printd("\r\n IO control overflow");
                        return;
                }
        }
        printd("\r set IO to %d %s", syspara.ioCtrl, (0 == syspara.ioCtrl ? "关" : "开"));
        I2CPageWrite_Nbytes(ADDR_IO_CTRL, LEN_IO_CTRL, (uint8_t *)&syspara.ioCtrl);
}

/**
 * @brief     : 老化间隔
 * @param     : rw
 */
void TermInterval(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
            I2CPageRead_Nbytes(ADDR_INTVL, LEN_INTVL, &syspara.agingInterval);
            printd("Interval %d Sec", syspara.agingInterval);
    }
    else
    {
        unsigned char ret = FetchInt(3, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if(BYTE_RANGE_MIN <= getInt && BYTE_RANGE_MAX >= getInt)
        {
            syspara.agingInterval = getInt;
            printd("\r\n set interval to %d Sec", syspara.agingInterval);
            I2CPageWrite_Nbytes(ADDR_INTVL, LEN_INTVL, &syspara.agingInterval);
        }
        else
        {
            printd("\r\n interval overflow");
            return;
        }
    }
}

/**
 * @brief     : 电流设置
 * @param     : rw
 */
void TermISet(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_ISET, LEN_ISET, &valve.iSet);
        printd("\r Current:%d %sA ", valve.iSet, 
            (0 == valve.iSet ? "2.6" :
                (1 == valve.iSet ? "2.2" :
                    (2 == valve.iSet ? "1.8" : 
                        (3 == valve.iSet ? "1.6" : "0.5")))));
    }
    else
    {
        unsigned char ret = FetchInt(4, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if(0 <= getInt && 4 >= getInt)
        {
            valve.iSet = getInt;
            printd("\r set current to:%d %sA ", valve.iSet, 
            (0 == valve.iSet ? "2.6" :
                (1 == valve.iSet ? "2.2" :
                    (2 == valve.iSet ? "1.8" : 
                        (3 == valve.iSet ? "1.6" : "0.5")))));
            I2CPageWrite_Nbytes(ADDR_ISET, LEN_ISET, &valve.iSet);
#ifndef A12_901
            ISET(valve.iSet);
#endif
        }
        else
        {
            printd("\r\n current overflow");
            return;
        }
    }
}

/*

*/
void TermRDCR(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_RDC_RATE, LEN_RDC_RATE, &rdc.rate);
        printd("\r\n Rate %d", rdc.rate);
    }
    else
    {
        unsigned char ret = FetchInt(4, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if (RDC01 == getInt || RDC04 == getInt || RDC10 == getInt || 
            RDC16 == getInt || RDC20 == getInt)
        {
            rdc.rate = getInt;
            printd("\r\n set Rate to %d", rdc.rate);
        }
        else
        {
            rdc.rate = RDC04;
            printd("\r\n %d Rate out of range. Use default Rate %d", getInt, rdc.rate);
        }
        I2CPageWrite_Nbytes(ADDR_RDC_RATE, LEN_RDC_RATE, &rdc.rate);
    }
}

/*

*/
void TermHalf(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_HALF_SEAL, LEN_HALF_SEAL, &valve.bHalfSeal);
        printd("\r\n Half seal %d", valve.bHalfSeal);
    }
    else
    {
        unsigned char ret = FetchInt(4, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if (ON == getInt || OFF == getInt)
        {
            valve.bHalfSeal = getInt;
            printd("\r\n set half seal to %d", valve.bHalfSeal);
        }
        else
        {
            valve.bHalfSeal = OFF;
            printd("\r\n %d half seal overflow. Use default half %d", getInt, valve.bHalfSeal);
        }
        I2CPageWrite_Nbytes(ADDR_HALF_SEAL, LEN_HALF_SEAL, &valve.bHalfSeal);
    }
}

/*

*/
void TermOut(char rw)
{
//    int getInt = 0;
    if(rw == READ_ACT)
    {
        IO_OUT = !IO_OUT;
        printd("\r\n OUT");
    }
}

/*
 * 切换次数
 */
void TermMovesCnt(char rw)
{
    int getInt = 0;
    printd("\r\n func %s", __func__);
    if(rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, ((uint8*)&syspara.totalCnt));
        printd("\r 切换次数:%d", syspara.totalCnt);
    }
    else
    {
        unsigned char ret = FetchInt(5, 0, RxBuf, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        syspara.totalCnt = getInt;
        printd("\r 写入切换次数:%d", syspara.totalCnt);
        I2CPageWrite_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, (uint8*)&syspara.totalCnt);
    }
}


/*
 * 点检模式：打印出所有关键参数
 */
void TermInspection(char rw)
{
        printd("\r\n ------------------------------------------");
        printd("\r\n                 请 注 意 !                ");
        printd("\r\n      未 使 用 变 量 请 勿 自 行 识 别      ");
        printd("\r\n ------------------------------------------\r\n");
        printd("\r\n ***************< 点检模式 >***************");
        /* 点检参数 */
        printd("\r\n 版本       (VR)   : %s", SOFT_VER_C);                                               /* 版本号 */
        printd("\r\n 电路板     (PCB)  : %s", PCB_VR);                                                   /* PCB版本号 */
        printd("\r\n 编译时间   (TIME) : %s %s", __DATE__, __TIME__);                                    /* 时间 */
        printd("\r\n 地址       (ADDR) : %d", ags_mbParam.mAddrs);                                        /* 地址 */
        printd("\r\n 通道数     (CNT)  : %d", valveFix.fix.portCnt);                                     /* 通道数 */
        printd("\r\n 波特率     (BAUD) : %d %dbps", syspara.baudrate, BaudRate_V[syspara.baudrate]);     /* 波特率 */
        printd("\r\n 速度       (SPD)  : %d RPM", valve.spd);                                            /* 速度 */
        printd("\r\n 减速比     (RDCR) : %d", rdc.rate);                                                 /* 减速比 */
        printd("\r\n 半通道     (HALF) : %d %s", valve.bHalfSeal, (0 == valve.bHalfSeal ? "关" : "开")); /* 半通道 */
        printd("\r\n 原点补偿   (FIXO) : %d (1度)", valveFix.fix.org);                                   /* 原点补偿 */
        printd("\r\n 方向补偿   (FIXG) : %d (0.1度)", valveFix.fix.dirGap);                              /* 方向补偿 */
        printd("\r\n IO控制     (IOE)  : %d %s", syspara.ioCtrl, (0 == syspara.ioCtrl ? "关" : "开"));                 /* IO */
        printd("\r\n 电流       (ISET) : %d", valve.iSet);                                               /* 电流设置 */
        printd("\r\n 切换次数   (MOVES): %d", syspara.totalCnt);                                         /* 切换次数 */
        printd("\r\n 回复方式   (REPLY): %d", syspara.replyMode);                                        /* 回复方式 */
        /* 序列号 */
        printd("\r\n 序列号     (SN)   : %02X %02X %02X %02X %02X", valve.SnCode[0], valve.SnCode[1], valve.SnCode[2],
               valve.SnCode[3], valve.SnCode[4]);
        printd("\r\n ***************< 点检模式 >***************\r\n");
}

/*
 * 回复模式：AGS协议栈回复模式 0,AGS标准 不回复  1,定制 运动指令不回复
 */
void TermReply(char rw)
{
    int getInt = 0;
    printd("\r\n func %s", __func__);
    if (rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_REPLY_MODE, LEN_REPLY_MODE, &syspara.replyMode);
        printd("\r 回复方式:%d", syspara.replyMode);
    }
    else
    {
        unsigned char ret = FetchInt(5, 0, RxBuf, &getInt);
        if (ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if (REPLYMODE_AGS <= getInt && REPLYMODE_CUSTOM_3 >= getInt)
        {
            syspara.replyMode = getInt;
            printd("\r 写入回复方式:%d", syspara.replyMode);
        }
        else
        {
            syspara.replyMode = REPLYMODE_AGS;
            printd("\r\n %d 回复方式不存在 使用AGS标准 (范围%d-%d)", getInt, REPLYMODE_AGS, REPLYMODE_CUSTOM_3);
        }
        I2CPageWrite_Nbytes(ADDR_REPLY_MODE, LEN_REPLY_MODE, &syspara.replyMode);
    }
}

void TermProtocal(char rw)
{
        int getInt = 0;
        if (rw == READ_ACT) {
                I2CPageRead_Nbytes(ADDR_PROTOCOL, LEN_PROTOCOL, &syspara.protocol_type);
                printd("\r\n now protocol is");
                if (AGS_MODBUS == syspara.protocol_type) {
                        printd(" AGS");
                } else if (EXT_COMM == syspara.protocol_type) {
                        printd(" HX");
                } else if (MODBUS == syspara.protocol_type) {
                        printd(" MODBUS");
                } else {
                        printd(" wrong type");
                }
        } else {
                unsigned char ret = FetchInt(5, 0, RxBuf, &getInt);
                if (ret) {
                        printd("\r\n Err code %d", ret);
                        return;
                }

                switch (getInt) {
                        default:
                                printd("\r\n wrong type set default AGS");
                        case AGS_MODBUS:
                                syspara.protocol_type = AGS_MODBUS;
                                printd("\r\n set protocal to AGS");
                                break;
                        case EXT_COMM:
                                syspara.protocol_type = EXT_COMM;
                                printd("\r\n set protocal to HX");
                                break;
                        case MODBUS:
                                syspara.protocol_type = MODBUS;
                                printd("\r\n set protocal to MODBUS");
                                break;
                }
                I2CPageWrite_Nbytes(ADDR_PROTOCOL, LEN_PROTOCOL, &syspara.protocol_type);
        }
}

/*
 * 老化次数
 */
void TermTestCnt(char rw)
{
    int getInt = 0;
    uint32_t saveCnt = 0;
    if (rw == READ_ACT)
    {
        I2CPageRead_Nbytes(ADDR_BURN_CNT, LEN_BURN_CNT, (uint8_t *)&saveCnt);
        if(saveCnt <= syspara.burnCnt)
        {
            I2CPageWrite_Nbytes(ADDR_BURN_CNT, LEN_BURN_CNT, (uint8_t *)&syspara.burnCnt);
            dbg_printf("Saved");
        }
        printd("\r\n 老化次数 %d", syspara.burnCnt);
    }
    else
    {
        unsigned char ret = FetchInt(5, 0, RxBuf, &getInt);
        if (ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        printd("\r\n 设置老化次数 %d", getInt);
        syspara.burnCnt = getInt;
        I2CPageWrite_Nbytes(ADDR_BURN_CNT, LEN_BURN_CNT, (uint8_t *)&syspara.burnCnt);
    }
}

void TermList(char rw);

// clang-format off
/* 填充命令结构体数组 */
static _CMD_T cmds[] =
{
    {"/?",      2,  TermList},
    {"VR",      2,  TermVR},
    // {"MAP",     3,  TermMap},
    {"IIC",     3,  TermIIC},
    {"RST",     3,  TermReset},
    // {"MOT",     3,  TermMotorX},
    {"FIXO",    4,  TermFixO},
    {"FIXG",    4,  TermFixG},
    {"ADDR",    4,  TermAddr},
    {"CNT",     3,  TermCnt},
    {"POS",     3,  TermPos},
    {"BDR",     3,  TermBaud},
    {"SPD",     3,  TermSpd},
    {"IOE",     3,  TermIO},
    {"INT",     3,  TermInterval},
    {"ISET",    4,  TermISet},
    {"OUT",     3,  TermOut},
    {"RDCR",    4,  TermRDCR},
    {"HALF",    4,  TermHalf},
    {"MOVES",   5,  TermMovesCnt},
    {"INSP",    4,  TermInspection},
    {"REPLY",   5,  TermReply},
    {"PRTCL",   5,  TermProtocal},
    {"TESTC",   5,  TermTestCnt},
};

static char* comment[] =
{
    "显示所有指令",
    "显示版本",
    "擦除EEPROM",
    "设置/显示原点补偿",
    "设置/显示方向补偿",
    "设置/显示地址",
    "设置/显示通道数",
    "移动到指定位置",
    "设置/显示波特率",
    "设置/显示速度",
    "设置IO",
    "设置/显示间隔",
    "设置/显示电流",
    "IO输出引脚翻转",
    "设置/显示减速比",
    "设置/显示半通道",
    "设置/显示切换次数",
    "点检模式",
    "设置/显示回复模式",
    "设置/显示控制协议",
};
// clang-format on

/*
 * 显示所有指令
 */
void TermList(char rw)
{
    if(rw==READ_ACT)
    {
        printd("\r\n %s", S_LIST_SH);
        for(int i=0; i<ARRAY_SIZE(cmds); i++)
        {
            if(i<ARRAY_SIZE(comment))
            {
                printd("\r\n|-%s", cmds[i].Name);
                int j = 10-cmds[i].RealLen;
                while(--j)
                {
                    printd(" ");
                }
                printd("%s", comment[i]);
            }
        }
        printd("\r\n %s", S_LIST_SE);
    }
}

void UsrCmdInit(void)
{
    RegisterCmds(cmds, ARRAY_SIZE(cmds));
}
