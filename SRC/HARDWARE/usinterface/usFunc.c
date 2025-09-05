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
        unsigned char ret = FetchInt(2, 0, rcvStr, &getInt);
        if(ret)
        {
            return;
        }
    }
}


/*

*/
void TermMap(char rw)
{
    char getChar[2][P_LEN+1];
    int getInt=0;
    int *getAddr=NULL;

    memset(getChar, 0, sizeof(getChar));
    if(rw==READ_ACT)
    {
        printd("\r\n No para");
    }
    else
    {
        unsigned char ret = FetchChar(3, 0, rcvStr, *getChar);
        if(ret)
        {
            printd("\r\n Err code %d", ret);
            return;
        }
        getAddr = (int *)strtohex(*getChar);
        if(**(getChar+1))
        {
            //判断是否有参数写入
            getInt = str2int(*(getChar+1));
            *getAddr = getInt;
            printd("\r\n Wr 0x%08x to 0x%08x", getInt, getAddr);
        }
        else
        {
            //无参数,取指
            getInt = (int)(*getAddr);
            printd("\r\n Rd 0x[%02x %02x %02x %02x] from 0x%08x",
                   (char)(getInt>>24), (char)(getInt>>16), (char)(getInt>>8), (char)(getInt>>0), getAddr);
        }
    }
}


/*

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

        unsigned char ret = FetchInt(3, 0, rcvStr, &getInt);
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
        unsigned char ret = FetchInt(3, 0, rcvStr, &getInt);
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
        unsigned char ret = FetchInt(4, 0, rcvStr, &getInt);
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
        unsigned char ret = FetchInt(4, 0, rcvStr, &getInt);
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
        unsigned char ret = FetchInt(4, 0, rcvStr, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if(AGS_ADDR_MIN <= getInt && BURN_ADDR >= getInt)
        {
            ModbusPara.mAddrs = getInt;
            printd("\r\n Set Addr to %d", ModbusPara.mAddrs);
        }
        else
        {
            printd("\r\n %d Address out of range (%d-%d)", getInt, AGS_ADDR_MIN, BURN_ADDR);
            ModbusPara.mAddrs = AGS_ADDR_DEF;
            printd("\r\n Use default Address %d", ModbusPara.mAddrs);
        }
        I2CPageWrite_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, &ModbusPara.mAddrs);
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
        (CHANNEL_MIN > valveFix.fix.portCnt && CHANNEL_MAX < valveFix.fix.portCnt) ? 
            (valveFix.fix.portCnt = CHANNEL_DEF) : (valveFix.fix.portCnt);
        printd("\r Port Cnt:%d", valveFix.fix.portCnt);
    }
    else
    {
        unsigned char ret = FetchInt(3, 0, rcvStr, &getInt);
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
        unsigned char ret = FetchInt(3, 0, rcvStr, &getInt);
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
        I2CPageRead_Nbytes(ADDR_BAUD, LEN_BAUD, &bdrate);
        printd("\r\n Baud:%d", bdrate);
    }
    else
    {
        unsigned char ret = FetchInt(3, 0, rcvStr, &getInt);
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
            bdrate = 1;
        }
        else if(19200 == getInt)
        {
            bdrate = 2;
        }
        else if(38400 == getInt)
        {
            bdrate = 3;
        }
        else
        {
            printd("\r\n baud rate overflow");
            return;
        }
        printd("\r set baud rate to %d %dbps", bdrate, getInt);
        I2CPageWrite_Nbytes(ADDR_BAUD, LEN_BAUD, &bdrate);
    }
}

/*

*/
void TermSpd(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
        I2CPageWrite_Nbytes(ADDR_SPD, LEN_SPD, &spdVx2);
        printd("\r\n Speed %d", spdVx2);
    }
    else
    {
        unsigned char ret = FetchInt(3, 0, rcvStr, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if(RDC20 == rdc.rate)
        {
            if(SPD_MIN_RDCR20 <= getInt && SPD_MAX_RDCR20 >= getInt)
            {
                spdVx2 = getInt;
                printd("\r\n set speed to %d", spdVx2);
            }
            else
            {
                printd("\r\n %d Speed out of range (%dRDCR: %d-%d)", 
                    getInt, rdc.rate, SPD_MIN_RDCR20, SPD_MAX_RDCR20);
                spdVx2 = INIT_SPD / 2;
                printd("\r\n Use default Speed %d", spdVx2);
            }
        }
        else
        {
            if(SPD_MIN <= getInt && SPD_MAX >= getInt)
            {
                spdVx2 = getInt;
                printd("\r\n set speed to %d", spdVx2);
            }
            else
            {
                printd("\r\n %d Speed out of range (%dRDCR: %d-%d)", 
                    getInt, rdc.rate, SPD_MIN, SPD_MAX);
                spdVx2 = INIT_SPD;
                printd("\r\n Use default Speed %d", spdVx2);
            }
        }
        I2CPageWrite_Nbytes(ADDR_SPD, LEN_SPD, &spdVx2);
    }
}

/*

*/
void TermIO(char rw)
{
    if(rw == READ_ACT)
    {
        bIoCtrl = !bIoCtrl;
        I2CPageWrite_Nbytes(ADDR_IO_CTRL, LEN_IO_CTRL, &bIoCtrl);
        printd("\r IO:%d %s", bIoCtrl, (0 == bIoCtrl ? "OFF" : "ON"));
    }
}


/*

*/
void TermInterval(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
        I2CPageWrite_Nbytes(ADDR_INTVL, LEN_INTVL, &intCtrl);
        printd("Interval %d Sec", intCtrl);
    }
    else
    {
        unsigned char ret = FetchInt(3, 0, rcvStr, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if(BYTE_RANGE_MIN <= getInt && BYTE_RANGE_MAX >= getInt)
        {
            intCtrl = getInt;
            printd("\r\n set interval to %d Sec", intCtrl);
            I2CPageWrite_Nbytes(ADDR_INTVL, LEN_INTVL, &intCtrl);
        }
        else
        {
            printd("\r\n interval overflow");
            return;
        }
    }
}

/*

*/
void TermISet(char rw)
{
    int getInt = 0;
    if(rw == READ_ACT)
    {
        I2CPageWrite_Nbytes(ADDR_ISET, LEN_ISET, &valve.iSet);
        printd("\r Current:%d %sA ", valve.iSet, 
            (0 == valve.iSet ? "2.6" :
                (1 == valve.iSet ? "2.2" :
                    (2 == valve.iSet ? "1.8" : 
                        (3 == valve.iSet ? "1.6" : "0.5")))));
    }
    else
    {
        unsigned char ret = FetchInt(4, 0, rcvStr, &getInt);
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
        unsigned char ret = FetchInt(4, 0, rcvStr, &getInt);
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
        unsigned char ret = FetchInt(4, 0, rcvStr, &getInt);
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
        unsigned char ret = FetchInt(5, 0, rcvStr, &getInt);
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
    printd("\r\n");
    /* 点检参数： 版本号 地址 通道数 波特率 速度 减速比 半通道 IO */
    printd("\r\n VR   : %s", SOFT_VER_C);   /* 版本号 */
    printd("\r\n TIME : %s %s", __DATE__, __TIME__);   /* 版本号 */
    printd("\r\n ADDR : %d", ModbusPara.mAddrs);      /* 地址 */
    printd("\r\n CNT  : %d", valveFix.fix.portCnt);    /* 通道数 */
    printd("\r\n BAUD : %d", bdrate);                 /* 波特率 */
    printd("\r\n SPD  : %d", spdVx2);                  /* 速度 */
    printd("\r\n IOE  : %d", bIoCtrl);                 /* IO */
    printd("\r\n ISET : %d", valve.iSet);             /* 电流设置 */
    printd("\r\n RDCR : %d", rdc.rate);               /* 减速比 */
    printd("\r\n HALF : %d", valve.bHalfSeal);        /* 半通道 */
    printd("\r\n MOVES: %d", syspara.totalCnt);      /* 切换次数 */
    printd("\r\n FIXO : %d", valveFix.fix.org);       /* 原点补偿 */
    printd("\r\n FIXG : %d", valveFix.fix.dirGap);    /* 方向补偿 */
    /* 序列号 */
    printd("\r\n SN   : %02X %02X %02X %02X %02X", 
        valve.SnCode[0], valve.SnCode[1], valve.SnCode[2], valve.SnCode[3], valve.SnCode[4]);
    printd("\r\n");
}

void TermList(char rw);
/* 填充命令结构体数组 */
static _CMD_T cmds[] =
{
    {"/?",      2,  TermList},
    {"VR",      2,  TermVR},
    {"MAP",     3,  TermMap},
    {"IIC",     3,  TermIIC},
    {"RST",     3,  TermReset},
    {"MOT",     3,  TermMotorX},
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
};

static char* comment[] =
{
    "display all commands",
    "display version",
    "write value into specify address",
    "operate storage like eeprom or factory set",
    "MOTORX",

};

/*

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


