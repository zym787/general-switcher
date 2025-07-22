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
        {//判断是否有参数写入
            getInt = str2int(*(getChar+1));
            *getAddr = getInt;
            printd("\r\n Wr 0x%08x to 0x%08x", getInt, getAddr);
        }
        else
        {//无参数，取指
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
    unsigned char rwBuf[4]={0, 0, 0, 0};
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
    valve.initStep = 0;
    printd("\r\n RESET");
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
        I2CPageWrite_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &valveFix.fix.org);
        printd("\r 写入原点补偿:%d", valveFix.fix.org);
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
        I2CPageWrite_Nbytes(ADDR_DIR_FIX, LEN_DIR_FIX, &valveFix.fix.dirGap);
        printd("\r 写入方向补偿:%d", valveFix.fix.dirGap);
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
        if(getInt<=64)
        {
            I2CPageWrite_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, (uint8*)&getInt);
            ModbusPara.mAddrs = getInt;
        }
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
        (valveFix.fix.portCnt&&valveFix.fix.portCnt>32)?(valveFix.fix.portCnt=10):(valveFix.fix.portCnt);
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
        if(getInt<=32)
        {
            I2CPageWrite_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, (uint8*)&getInt);
            valveFix.fix.portCnt = getInt;
        }
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
        if(valve.status==VALVE_RUN_END)
        {
            printd("\r ==>%d", getInt);
            valve.portDes = getInt;
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
            printd("\r\n baud rate %d not exist", getInt);
            return;
        }
        if(getInt==9600)
        {
            printd("\r\n set baud rate to %d", getInt);
            bdrate = 1;
        }
        else if(getInt==19200)
        {
            printd("\r\n set baud rate to %d", getInt);
            bdrate = 2;
        }
        else if(getInt==38400)
        {
            printd("\r\n set baud rate to %d", getInt);
            bdrate = 3;
        }
        I2CPageWrite_Nbytes(ADDR_BAUD, LEN_BAUD, &bdrate);
    }
}

/*

*/
void TermSpd(char rw)
{
    uint8 rdbuff[2]={0};
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
        if(getInt)
        {
            printd("\r\n set spd to %d", getInt);
            rdbuff[0] = getInt>>8;
            rdbuff[1] = getInt;
            I2CPageWrite_Nbytes(ADDR_SPD, LEN_SPD, rdbuff);
        }
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
    }
    else
    {
        unsigned char ret = FetchInt(3, 0, rcvStr, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if(getInt&&getInt<=255)
        {
            printd("\r\n set INT to %d", getInt);
            intCtrl = getInt;
            I2CPageWrite_Nbytes(ADDR_INTVL, LEN_INTVL, &intCtrl);
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
        printd("\r\n now current %d", valve.iSet);
    }
    else
    {
        unsigned char ret = FetchInt(4, 0, rcvStr, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        if(getInt<=3)
        {
            printd("\r\n set I to %d", getInt);
            valve.iSet = getInt;
            I2CPageWrite_Nbytes(ADDR_ISET, LEN_ISET, &valve.iSet);
			#ifndef A12_901
            ISET(valve.iSet);
			#endif
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
        printd("\r\n read rate %d", rdc.rate);
    }
    else
    {
        unsigned char ret = FetchInt(4, 0, rcvStr, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        printd("\r\n set rate %d", getInt);
        rdc.rate = getInt;
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
        printd("\r\n read half %d", valve.bHalfSeal);
    }
    else
    {
        unsigned char ret = FetchInt(4, 0, rcvStr, &getInt);
        if(ret)
        {
            printd("\r Err code %d", ret);
            return;
        }
        printd("\r\n set half %d", getInt);
        valve.bHalfSeal = getInt;
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

void TermList(char rw);
/* 填充命令结构体数组 */
static _CMD_T cmds[] = {
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
};

static char* comment[] = {
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


