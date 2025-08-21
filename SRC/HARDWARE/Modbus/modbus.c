#define _MODBUS_GLOBALS_
#include "common.h"

#define RX_EN()     (PBout(1)=0)
#define TX_EN()     (PBout(1)=1)

void ModbusInit(void)
{
    unsigned char cnt;

    RCC->APB2ENR |= (RCC_APB2Periph_GPIOB);
    GPIOB->CRL &= (GPIO_Crl_P1);
    GPIOB->CRL |= (GPIO_Mode_Out_PP_50MHz_P1);

    RX_EN();        /* 开机为接收模式 */

    I2CPageRead_Nbytes(ADDR_BAUD, LEN_BAUD, &bdrate);
    printd("\r baud:%d", bdrate);
    if(!bdrate||bdrate>3)
        bdrate = 2;

    if(bdrate==1)
    {
        Usart2_Init(36, BAUD_RATE_9600);				//串口初始化默认为9600
        delay_ms(100);
        Usart3_Init(36, BAUD_RATE_9600);	        // 串口2 485初始化为9600
        delay_ms(100);
        TIM3_Init(MODBUS_TIME_9600,71);	                // 45us--0.45ms
    }
    else if(bdrate==2)
    {
        Usart2_Init(36, BAUD_RATE_19200);				//串口初始化默认为19200
        delay_ms(100);
        Usart3_Init(36, BAUD_RATE_19200);	            // 串口2 485初始化为19200
        delay_ms(100);
        TIM3_Init(MODBUS_TIME_19200,71);	                // 45us--0.45ms
    }
    else if(bdrate==3)
    {
        Usart2_Init(36, BAUD_RATE_38400);				//串口初始化默认为38400
        delay_ms(100);
        Usart3_Init(36, BAUD_RATE_38400);	        // 串口2 485初始化为38400
        delay_ms(100);
        TIM3_Init(MODBUS_TIME_38400,71);	                // 45us--0.45ms
    }
    delay_ms(100);

    // 参数配置
    ModbusPara.sRUN =  MB_IDEL;
    ModbusPara.sERR = ERR_NOT;
    ModbusPara.times = 0;
    ModbusPara.rCnt = 0;
    ModbusPara.HostRept = 0;
    ModbusPara.mAddrs = MB_SALVE_DEF_ADDR;
    for(cnt = 0; cnt < LENGTH_MB_DATA; cnt++)
    {
        ModbusPara.rBuf[cnt] = 0;
        ModbusPara.tBuf[cnt] = 0;
    }

    for(cnt = 0; cnt < LENGTH_COIL_REG; cnt++)
        CoilBitPara[cnt] = 0;

    for(cnt = 0; cnt < LENGTH_DiscreteREG; cnt++)
        DiscreteBitPara[cnt] = 0;

    for(cnt = 0; cnt < LENGTH_HoldingREG; cnt++)
        HoldingREGPara[cnt] = 0;

    for(cnt = 0; cnt < LENGTH_InputREG; cnt++)
        InputREGPara[cnt] = 0;
}


void ModbusTimesProcess(void)
{
    if(ModbusPara.sRUN&MB_BUSY)
    {
        /* 总线检测 */
        if(ModbusPara.times < BUS_IDLE_TIME)
        {
            ModbusPara.times++;
        }
        else
        {
            //总线进入空闲,帧结束或开始
            if(ModbusPara.sRUN == MB_RECIVE_ERR)
            {
                // 接收过程中 有出现数据存储空间溢出或间隔时间超过T1.5
                ModbusPara.sERR = ERR_MB_DEVICE;
                ModbusPara.sRUN =  MB_IDEL;
            }
            else if(ModbusPara.sRUN == MB_NO_RESPONSE)
            {
                ModbusPara.sRUN = MB_IDEL;
            }
            else if(ModbusPara.sRUN == MB_RECIVE)
            {
                ModbusPara.sRUN =  MB_RECIVE_END;
            }
        }
    }
}


void ModbusSend(unsigned char length)
{
    unsigned char cnt;

    TX_EN();
    if(length)
    {
        ModbusPara.sRUN = MB_SEND;
        for(cnt=0; cnt < length; cnt++)
        {
            while((USART3->SR&0X40)==0);        //等待发送结束
            USART3->DR = ModbusPara.tBuf[cnt];
            ModbusPara.times = 0;               //重新计时。
        }
    }
    while((USART3->SR&0X40)==0);                //等待发送结束
    RX_EN();

    if(length)
    {
        ModbusPara.sRUN = MB_SEND;
        for(cnt=0; cnt < length; cnt++)
        {
            while((USART2->SR&0X40)==0);        //等待发送结束
            USART2->DR = ModbusPara.tBuf[cnt];
            ModbusPara.times = 0;               //重新计时。
        }
    }
    while((USART2->SR&0X40)==0);                //等待发送结束
    ModbusPara.sRUN = MB_IDEL;
    ModbusPara.rCnt = 0;
}

void ModbusReceive(unsigned char res)
{
    ModbusPara.times = 0;  //重新计时
    if(ModbusPara.sRUN==MB_IDEL && !ModbusPara.rCnt)
    {
        // 空闲并且数据处理结束,可以进行新的接收
        if(ModbusPara.mAddrs==res || res==MB_Broadcast_ADDR)
        {
            // 开始接收数据
            ModbusPara.sRUN = MB_RECIVE;
            ModbusPara.sERR = ERR_NOT;
            ModbusPara.rCnt = 1;
            ModbusPara.rBuf[0] = res;
        }
        else
        {
            // 非本设备地址,或非当前查询的设备,且非广播地址
            ModbusPara.sRUN = MB_NO_RESPONSE;
            ModbusPara.sERR = ERR_MB_DEVICE;
        }
    }
    else if(ModbusPara.sRUN==MB_RECIVE)
    {
        // 如果溢出或者传输过程出现时间间隔超过T1.5,都不在接收
        if(ModbusPara.rCnt < LENGTH_MB_DATA && ModbusPara.times < FRAME_ERR_TIME)
        {
            ModbusPara.rBuf[ModbusPara.rCnt] = res;
        }
        else
        {
            ModbusPara.sRUN = MB_RECIVE_ERR;
        }

        ++ModbusPara.rCnt;
    }
}

void Modbus_ERROR(void)
{
    uint16_t temp16;

    if(ModbusPara.sERR==ERR_MB_FUN || ModbusPara.sERR==ERR_MB_ADDR || ModbusPara.sERR==ERR_MB_DATA)
    {
        /* 从模式,发送响应数据 */
        ModbusPara.tBuf[0] = ModbusPara.rBuf[0];			// 设备地址
        ModbusPara.tBuf[1] = ModbusPara.rBuf[1]|0X80;	    // 功能码| 0X80
        ModbusPara.tBuf[2] = ModbusPara.sERR;			    // 错误代码
        temp16 = ModbusCRC16(&ModbusPara.tBuf[0], 3);	    // 获取CRC
        ModbusPara.tBuf[3] = temp16 >> 8;
        ModbusPara.tBuf[4] = temp16 ;
        if(ModbusPara.tBuf[0] != MB_Broadcast_ADDR)
        {
            ModbusSend(5);
        }
#ifdef DEBUG_MODBUS
        printd("\r\n [%02x]error reply Func:%02x",
               ModbusPara.sERR, ModbusPara.tBuf[1]);
#endif
    }
    ModbusPara.sERR = ERR_NOT;
}

/* 功能码3 */
void MB_ReadHoldingRegisters(void)
{
    unsigned short reg_num;
    unsigned char dvc_addr, op_addr, byteCount;

    dvc_addr = ModbusPara.rBuf[0];      /* 模块地址 */
    op_addr = ModbusPara.rBuf[2];       /* 操作码/操作地址 */
    /* 地址判断 */
    if(ModbusPara.mAddrs == dvc_addr || MB_Broadcast_ADDR == dvc_addr)
    {
        ModbusPara.tBuf[0] = ModbusPara.rBuf[0];    /* 模块地址 */
        ModbusPara.tBuf[1] = ModbusPara.rBuf[1];    /* 功能码 */
        ModbusPara.tBuf[2] = ModbusPara.rBuf[2];    /* 操作码/操作地址 */
        if(0x00 == op_addr)                /* 读状态 */
        {
            ModbusPara.tBuf[3] = valve.status;          /* 模块状态 */
            ModbusPara.tBuf[4] = valve.portCur;         /* 当前通道 */
            ModbusPara.tBuf[5] = ModbusPara.mAddrs;     /* 模块地址 */
            ModbusPara.tBuf[6] = valveFix.fix.portCnt;  /* 模块通道数 */
            ModbusPara.tBuf[7] = valve.fixOrg;          /* 原点补偿值 */
            ModbusPara.tBuf[8] = valveFix.fix.org;      /* 方向补偿值 */
            ModbusPara.tBuf[9] = spdVx2;                /* 速度 */
            byteCount = 10;
        }
        else if(0x01 == op_addr)       /* 读当前通道 */
        {
            ModbusPara.tBuf[3] = valve.portCur;         // 通道编号
            byteCount = 4;
        }
        else if(0x02 == op_addr)       /* 读地址 */
        {
            ModbusPara.tBuf[3] = ModbusPara.mAddrs;         // 模块地址
            byteCount = 4;
        }
        else if(0x03 == op_addr)       /* 读版本 */
        {
            ModbusPara.tBuf[3] = (SOFT_VER >> 24) & 0xFF;
            ModbusPara.tBuf[4] = (SOFT_VER >> 16) & 0xFF;
            ModbusPara.tBuf[5] = (SOFT_VER >> 8) & 0xFF;
            ModbusPara.tBuf[6] = (SOFT_VER >> 0) & 0xFF;   // 模块版本号
            byteCount = 7;
        }
        else if(0x07 == op_addr)       /* 读波特率 */
        {
            ModbusPara.tBuf[3] = bdrate;     // 波特率
            byteCount = 4;
        }
        else if(0x08 == op_addr)       /* 读序列号 */
        {
            I2CPageRead_Nbytes(ADDR_SN, LEN_SN, valve.SnCode);
            ModbusPara.tBuf[3] = valve.SnCode[0];
            ModbusPara.tBuf[4] = valve.SnCode[1];
            ModbusPara.tBuf[5] = valve.SnCode[2];
            ModbusPara.tBuf[6] = valve.SnCode[3];
            ModbusPara.tBuf[7] = valve.SnCode[4];
            byteCount = 8;
        }
        else if(0x09 == op_addr)       /* 读速度 */
        {
            ModbusPara.tBuf[3] = spdVx2;     /* 转动速度 */
            byteCount = 4;
        }
        else if(0x0A == op_addr)       /* 读切换次数 */
        {
            ModbusPara.tBuf[3] = ((uint8*)&syspara.totalCnt)[3];
            ModbusPara.tBuf[4] = ((uint8*)&syspara.totalCnt)[2];
            ModbusPara.tBuf[5] = ((uint8*)&syspara.totalCnt)[1];
            ModbusPara.tBuf[6] = ((uint8*)&syspara.totalCnt)[0];
            if(syspara.totalCnt != syspara.totalCntLst)
            {
                I2CPageWrite_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, ((uint8*)&syspara.totalCnt));
            }
            byteCount = 7;
        }
        else if(0x99 == op_addr)        /* 读通道数 */
        {
            I2CPageRead_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
            ModbusPara.tBuf[3] = valveFix.fix.portCnt;
            byteCount = 4;
        }
        else
        {
            ModbusPara.sERR = ERR_MB_ADDR;  /* 非法数据地址 */
        }
        /* 拼接 */
        reg_num = ModbusCRC16(&ModbusPara.tBuf[0], byteCount);  /* 获取CRC */
        ModbusPara.tBuf[byteCount] = reg_num >> 8;
        byteCount++;
        ModbusPara.tBuf[byteCount] = reg_num ;
        byteCount++;
        /* 只有当地址不为广播地址且无报错时才回复 可以通过广播地址02查地址 */
        if(((MB_Broadcast_ADDR != ModbusPara.tBuf[0]) ||
                (MB_Broadcast_ADDR == ModbusPara.tBuf[0]) &&
                0x02 == op_addr) && (ERR_NOT == ModbusPara.sERR))
        {
            ModbusSend(byteCount);   /* 回复 */
        }
#ifdef DEBUG_MODBUS
        printd("\r s:");
        for(uint8 i = 0; i < byteCount; i++)
            printd(" %02x", ModbusPara.tBuf[i]);
#endif
    }
    else
    {
        ModbusPara.sERR = ERR_MB_DEVICE_ADDR;   /* 非法从站设备地址 */
    }
}

/* 功能码6 */
void MB_PresetSingleHoldingRegister(void)
{
    uint16_t reg_num;
    uint8_t dvc_addr, op_addr, byteCount;

    dvc_addr = ModbusPara.rBuf[0];  /* 第1字节 站号 模块地址 */
    op_addr = ModbusPara.rBuf[2];   /* 第3字节 操作码 操作地址 */
    if((dvc_addr == ModbusPara.mAddrs) && BURN_ADDR != dvc_addr)
    {
        ModbusPara.tBuf[0] = ModbusPara.rBuf[0];    /* 设备地址 */
        ModbusPara.tBuf[1] = ModbusPara.rBuf[1];    /* 功能码 */
        ModbusPara.tBuf[2] = ModbusPara.rBuf[2];    /* 操作码/操作地址 */
        if(0x00 == op_addr)             /* 写通道A */
        {
            if((POS_A == ModbusPara.rBuf[3] || POS_B == ModbusPara.rBuf[3]) &&
                    6 == ModbusPara.rCnt)
            {
                if(valve.status==VALVE_RUN_END)
                {
                    valve.portDes = ModbusPara.rBuf[3];
                }
                else
                {
                    ModbusPara.sERR = ERR_MB_BUSY;  /* 模块忙 */
                }
            }
            else
            {
                ModbusPara.sERR = ERR_MB_DATA;  /* 操作数据无效 */
            }
        }
        else if(0x01 == op_addr)        /* 写地址 */
        {
            if(AGS_ADDR_MIN <= ModbusPara.rBuf[3] && AGS_ADDR_MAX >= ModbusPara.rBuf[3] &&
                    6 == ModbusPara.rCnt)
            {
                ModbusPara.mAddrs = ModbusPara.rBuf[3];
                I2CPageWrite_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, &ModbusPara.mAddrs);
            }
            else
            {
                ModbusPara.sERR = ERR_MB_DATA;  /* 操作数据无效 */
            }
        }
        else if(0x06 == op_addr)        /* 复位*/
        {
            if (0x00 == ModbusPara.rBuf[3] && 6 == ModbusPara.rCnt)
            {
                valve.status = VALVE_INITING;
                valve.initStep = 0;     /* 复位指令 */
                valve.retryTms = 0;
                valve.ErrBlinkTime = RETRY_TIME_OUT;
            }
            else
            {
                ModbusPara.sERR = ERR_MB_DATA;  /* 操作数据无效 */
            }
        }
        else if(0x07 == op_addr)        /* 写波特率 */
        {
            if(BAUD_MIN <= ModbusPara.rBuf[3] && BAUD_MAX >= ModbusPara.rBuf[3] &&
                    6 == ModbusPara.rCnt)
            {
                bdrate = ModbusPara.rBuf[3];
                I2CPageWrite_Nbytes(ADDR_BAUD, LEN_BAUD, &bdrate);
            }
            else
            {
                ModbusPara.sERR = ERR_MB_DATA;  /* 操作数据无效 */
            }
        }
        else if(0x08 == op_addr)        /* 写序列号 */
        {
            if (10 == ModbusPara.rCnt)
            {
                valve.SnCode[0] = ModbusPara.rBuf[3];
                valve.SnCode[1] = ModbusPara.rBuf[4];
                valve.SnCode[2] = ModbusPara.rBuf[5];
                valve.SnCode[3] = ModbusPara.rBuf[6];
                valve.SnCode[4] = ModbusPara.rBuf[7];
                I2CPageWrite_Nbytes(ADDR_SN, LEN_SN, valve.SnCode);
            }
            else
            {
                ModbusPara.sERR = ERR_MB_DATA;   /* 操作数据无效 */
            }
        }
        else if(0x09 == op_addr)        /* 写速度 */
        {
            if(SPD_MIN <= ModbusPara.rBuf[3] && SPD_MAX >= ModbusPara.rBuf[3] &&
                    6 == ModbusPara.rCnt)
            {
                spdVx2 = ModbusPara.rBuf[3];
                I2CPageWrite_Nbytes(ADDR_SPD, LEN_SPD, &spdVx2);
            }
            else
            {
                ModbusPara.sERR = ERR_MB_DATA;  /* 操作数据无效 */
            }
        }
        else if(0x0A == op_addr)        /* 写切换次数 */
        {
            if (9 == ModbusPara.rCnt)
            {
                ((uint8*)&syspara.totalCnt)[0] = ModbusPara.rBuf[6];
                ((uint8*)&syspara.totalCnt)[1] = ModbusPara.rBuf[5];
                ((uint8*)&syspara.totalCnt)[2] = ModbusPara.rBuf[4];
                ((uint8*)&syspara.totalCnt)[3] = ModbusPara.rBuf[3];
                I2CPageWrite_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, (uint8*)&syspara.totalCnt);
            }
            else
            {
                ModbusPara.sERR = ERR_MB_DATA;   /* 操作数据无效 */
            }
        }
        else if(0x99 == op_addr)        /* 写通道数 */
        {
            if(CHANNEL_MIN <= ModbusPara.rBuf[3] && CHANNEL_MAX >= ModbusPara.rBuf[3] &&
                    6 == ModbusPara.rCnt)
            {
                valveFix.fix.portCnt = ModbusPara.rBuf[3];
                I2CPageWrite_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
            }
            else
            {
                ModbusPara.sERR = ERR_MB_DATA;  /* 操作数据无效 */
            }
        }
//        else if(0xFF == op_addr)        /* 高级写入 */
//        {
//            printd("\r\n rcv para: %02x %02x %02x %02x %02x %02x %02x %02x",
//                ModbusPara.rBuf[3], ModbusPara.rBuf[4], ModbusPara.rBuf[5], ModbusPara.rBuf[6],
//                ModbusPara.rBuf[7], ModbusPara.rBuf[8], ModbusPara.rBuf[9], ModbusPara.rBuf[10]);

//            I2CPageWrite_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, &ModbusPara.rBuf[3]);
//            I2CPageWrite_Nbytes(ADDR_BAUD, LEN_BAUD, &ModbusPara.rBuf[4]);
//            I2CPageWrite_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &ModbusPara.rBuf[5]);
//            I2CPageWrite_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &ModbusPara.rBuf[6]);
//            I2CPageWrite_Nbytes(ADDR_DIR_FIX, LEN_DIR_FIX, &ModbusPara.rBuf[7]);

//            uint8 ReadBuf[2]={0,0};
//            ReadBuf[0] = 0x00;
//            ReadBuf[1] = ModbusPara.rBuf[8];
//            I2CPageWrite_Nbytes(ADDR_SPD, LEN_SPD, ReadBuf);
//            I2CPageWrite_Nbytes(ADDR_IO_CTRL, LEN_IO_CTRL, &ModbusPara.rBuf[9]);
//            I2CPageWrite_Nbytes(ADDR_INTVL, LEN_INTVL, &ModbusPara.rBuf[10]);
//        }
        else
        {
            ModbusPara.sERR = ERR_MB_ADDR;  /* 超出操作地址范围或者操作地址无效 */
        }
        ModbusPara.tBuf[3] = ModbusPara.rBuf[3];  			// 端口编号
        byteCount = 4;
        /* 拼接 */
        reg_num = ModbusCRC16(&ModbusPara.tBuf[0], byteCount);  /* 获取CRC */
        ModbusPara.tBuf[byteCount] = reg_num >> 8;
        byteCount++;
        ModbusPara.tBuf[byteCount] = reg_num ;
        byteCount++;
        /* 只有当地址不为广播地址且无报错时才回复 */
        if((MB_Broadcast_ADDR != ModbusPara.tBuf[0]) &&
            (ERR_NOT == ModbusPara.sERR))
        {
            ModbusSend(byteCount);   /* 回复 */
        }
#ifdef DEBUG_MODBUS
        printd("\r s:");
        for(uint8 i = 0; i < byteCount; i++)
            printd(" %02x", ModbusPara.tBuf[i]);
#endif
    }
    else
    {
        ModbusPara.sERR = ERR_MB_DEVICE_ADDR;   /* 非法从站设备地址 */
    }
}

/* Modbus主线程 处理CRC 功能码 报错 */
void ModbusProces(void)
{
    uint32_t i = 0;
    if(MB_RECIVE_END == ModbusPara.sRUN)
    {
        if(LEAST_RCV_CNT < ModbusPara.rCnt)
        {
            LED_WORK = !LED_WORK;
            if(0 == ModbusCRC16(&ModbusPara.rBuf[0], ModbusPara.rCnt))
            {
#ifdef DEBUG_MODBUS
                printd("\r r:");
                for(i = 0; i < ModbusPara.rCnt; i++)
                    printd(" %02x", ModbusPara.rBuf[i]);
#endif
                /* 确认模块存在并且工作正常 */
                switch(ModbusPara.rBuf[1])
                {
                    case GET_HOLDING_REGT:          /* 功能码03 */
                        MB_ReadHoldingRegisters();
                        break;
                    case PRESET_HOLDING_sREGT:      /* 功能码06 */
                        MB_PresetSingleHoldingRegister();
                        break;
                    default:
                        ModbusPara.sERR = ERR_MB_FUN;
                        break;
                }
            }
            else
            {
                ModbusPara.sERR = ERR_MB_DEVICE;
            }
            ModbusPara.rCnt = 0;
            Modbus_ERROR();
            ModbusPara.sRUN = MB_IDEL;
        }
        else
        {
            ModbusPara.rCnt = 0;
            Modbus_ERROR();
            ModbusPara.sRUN = MB_IDEL;
        }
    }
}
