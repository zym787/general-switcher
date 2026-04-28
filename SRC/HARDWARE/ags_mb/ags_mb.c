#define _MODBUS_GLOBALS_
#include "common.h"

uint16_t BaudRate_V[BAUD_NUM] = {9600, 9600, 19200, 38400};
uint16_t BaudRate_Time[BAUD_NUM] = {520, 520, 260, 130};

void ags_mbInit(void)
{
        uint8_t cnt;

        RX_EN(); /* 开机为接收模式 */

        I2CPageRead_Nbytes(ADDR_BAUD, LEN_BAUD, &syspara.baudrate);
        // if (!syspara.baudrate || syspara.baudrate > 3)
        //         syspara.baudrate = 2;

        // if (syspara.baudrate == 1) {
        //         Usart2_Init(36, BAUD_RATE_9600);  // 串口初始化默认为9600
        //         delay_ms(100);
        //         Usart3_Init(36, BAUD_RATE_9600);  // 串口2 485初始化为9600
        //         delay_ms(100);
        //         TIM3_Init(MODBUS_TIME_9600, 71);  // 45us--0.45ms
        // } else if (syspara.baudrate == 2) {
        //         Usart2_Init(36, BAUD_RATE_19200);  // 串口初始化默认为19200
        //         delay_ms(100);
        //         Usart3_Init(36, BAUD_RATE_19200);  // 串口2 485初始化为19200
        //         delay_ms(100);
        //         TIM3_Init(MODBUS_TIME_19200, 71);  // 45us--0.45ms
        // } else if (syspara.baudrate == 3) {
        //         Usart2_Init(36, BAUD_RATE_38400);  // 串口初始化默认为38400
        //         delay_ms(100);
        //         Usart3_Init(36, BAUD_RATE_38400);  // 串口2 485初始化为38400
        //         delay_ms(100);
        //         TIM3_Init(MODBUS_TIME_38400, 71);  // 45us--0.45ms
        // }
        // delay_ms(100);

        if (BAUD_38400 < syspara.baudrate) {
                syspara.baudrate = BAUD_9600; /* 9600 */
        }
        Usart2_Init(36, BaudRate_V[syspara.baudrate]); /* UART2 19200bps */
        delay_ms(100);
        Usart3_Init(36, BaudRate_V[syspara.baudrate]); /* UART3 19200bps */
        delay_ms(100);
        TIM3_Init(BaudRate_Time[syspara.baudrate], 71);  // 45us--0.45ms
        delay_ms(100);

        printd("USART2/3 Init, baudrate: %d", BaudRate_V[syspara.baudrate]);

        // 参数配置
        ags_mbParam.sRUN = MB_IDLE;
        ags_mbParam.sERR = ERR_NOT;
        ags_mbParam.times = 0;
        ags_mbParam.rCnt = 0;
        ags_mbParam.HostRept = 0;
        ags_mbParam.mAddrs = MB_SALVE_DEF_ADDR;
        for (cnt = 0; cnt < LENGTH_MB_DATA; cnt++) {
                ags_mbParam.rBuf[cnt] = 0;
                ags_mbParam.tBuf[cnt] = 0;
        }

        for (cnt = 0; cnt < LENGTH_COIL_REG; cnt++)
                CoilBitPara[cnt] = 0;

        for (cnt = 0; cnt < LENGTH_DiscreteREG; cnt++)
                DiscreteBitPara[cnt] = 0;

        for (cnt = 0; cnt < LENGTH_HoldingREG; cnt++)
                HoldingREGPara[cnt] = 0;

        for (cnt = 0; cnt < LENGTH_InputREG; cnt++)
                InputREGPara[cnt] = 0;
}

void ags_mbTimesProcess(void)
{
        if (ags_mbParam.sRUN & MB_BUSY) {
                /* 总线检测 */
                if (ags_mbParam.times < BUS_IDLE_TIME) {
                        ags_mbParam.times++;
                } else {
                        // 总线进入空闲,帧结束或开始
                        if (ags_mbParam.sRUN == MB_RECIVE_ERR) {
                                // 接收过程中 有出现数据存储空间溢出或间隔时间超过T1.5
                                ags_mbParam.sERR = ERR_MB_DEVICE;
                                ags_mbParam.sRUN = MB_IDLE;
                        } else if (ags_mbParam.sRUN == MB_NO_RESPONSE) {
                                ags_mbParam.sRUN = MB_IDLE;
                        } else if (ags_mbParam.sRUN == MB_RECIVE) {
                                ags_mbParam.sRUN = MB_RECIVE_END;
                        }
                }
        }
}

void ags_mbSend(uint8_t length)
{
        uint8_t cnt;

        /* 485串口 */
        TX_EN();
        if (length) {
                ags_mbParam.sRUN = MB_SEND;
                for (cnt = 0; cnt < length; cnt++) {
                        while ((USART3->SR & 0X40) == 0)
                                ;  // 等待发送结束
                        USART3->DR = ags_mbParam.tBuf[cnt];
                        ags_mbParam.times = 0;  // 重新计时。
                }
        }
        while ((USART3->SR & 0X40) == 0)
                ;  // 等待发送结束
        RX_EN();

        /* 232串口 */
        if (length) {
                ags_mbParam.sRUN = MB_SEND;
                for (cnt = 0; cnt < length; cnt++) {
                        while ((USART2->SR & 0X40) == 0)
                                ;  // 等待发送结束
                        USART2->DR = ags_mbParam.tBuf[cnt];
                        ags_mbParam.times = 0;  // 重新计时。
                }
        }
        while ((USART2->SR & 0X40) == 0)
                ;  // 等待发送结束
        ags_mbParam.sRUN = MB_IDLE;
        ags_mbParam.rCnt = 0;
}

void ags_mbReceive(uint8_t res)
{
        ags_mbParam.times = 0;  // 重新计时
        if (ags_mbParam.sRUN == MB_IDLE && !ags_mbParam.rCnt) {
                // 空闲并且数据处理结束,可以进行新的接收
                if (ags_mbParam.mAddrs == res || MB_Broadcast_ADDR == res) {
                        // 开始接收数据
                        ags_mbParam.sRUN = MB_RECIVE;
                        ags_mbParam.sERR = ERR_NOT;
                        ags_mbParam.rCnt = 1;
                        ags_mbParam.rBuf[0] = res;
                } else {
                        // 非本设备地址,或非当前查询的设备,且非广播地址
                        ags_mbParam.sRUN = MB_NO_RESPONSE;
                        ags_mbParam.sERR = ERR_MB_DEVICE;
                }
        } else if (ags_mbParam.sRUN == MB_RECIVE) {
                // 如果溢出或者传输过程出现时间间隔超过T1.5,都不在接收
                if (ags_mbParam.rCnt < LENGTH_MB_DATA && ags_mbParam.times < FRAME_ERR_TIME) {
                        ags_mbParam.rBuf[ags_mbParam.rCnt] = res;
                } else {
                        ags_mbParam.sRUN = MB_RECIVE_ERR;
                }

                ++ags_mbParam.rCnt;
        }
}

void ags_mbError(void)
{
        uint16_t temp16;

        if (ags_mbParam.sERR == ERR_MB_FUN || ags_mbParam.sERR == ERR_MB_ADDR || ags_mbParam.sERR == ERR_MB_DATA) {
                /* 从模式,发送响应数据 */
                ags_mbParam.tBuf[0] = ags_mbParam.rBuf[0];         // 设备地址
                ags_mbParam.tBuf[1] = ags_mbParam.rBuf[1] | 0X80;  // 功能码| 0X80
                ags_mbParam.tBuf[2] = ags_mbParam.sERR;            // 错误代码
                temp16 = ModbusCRC16(&ags_mbParam.tBuf[0], 3);    // 获取CRC
                ags_mbParam.tBuf[3] = temp16 >> 8;
                ags_mbParam.tBuf[4] = temp16;
                if (ags_mbParam.tBuf[0] != MB_Broadcast_ADDR) {
                        ags_mbSend(5);
                }
#ifdef DEBUG_AGS_MB
                printd("\r\n [%02x]error reply Func:%02x", ags_mbParam.sERR, ags_mbParam.tBuf[1]);
#endif
        }
        ags_mbParam.sERR = ERR_NOT;
}

/* 功能码3 */
void ags_mbReadHoldingRegisters(void)
{
        unsigned short reg_num;
        uint8_t dvc_addr, op_addr, byteCount;

        dvc_addr = ags_mbParam.rBuf[0]; /* 模块地址 */
        op_addr = ags_mbParam.rBuf[2];  /* 操作码/操作地址 */
        /* 地址判断 */
        if (ags_mbParam.mAddrs == dvc_addr || MB_Broadcast_ADDR == dvc_addr) {
                ags_mbParam.tBuf[0] = ags_mbParam.rBuf[0]; /* 模块地址 */
                ags_mbParam.tBuf[1] = ags_mbParam.rBuf[1]; /* 功能码 */
                ags_mbParam.tBuf[2] = ags_mbParam.rBuf[2]; /* 操作码/操作地址 */
                if (0x00 == op_addr)                     /* 读状态 */
                {
                        ags_mbParam.tBuf[3] = valve.status;         /* 模块状态 */
                        ags_mbParam.tBuf[4] = valve.portCur;        /* 当前通道 */
                        ags_mbParam.tBuf[5] = ags_mbParam.mAddrs;    /* 模块地址 */
                        ags_mbParam.tBuf[6] = valveFix.fix.portCnt; /* 模块通道数 */
                        ags_mbParam.tBuf[7] = valveFix.fix.org;         /* 原点补偿值 */
                        ags_mbParam.tBuf[8] = valveFix.fix.dirGap;     /* 方向补偿值 */
                        ags_mbParam.tBuf[9] = valve.spd;               /* 速度 */
                        byteCount = 10;
                } else if (0x01 == op_addr) /* 读当前通道 */
                {
                        ags_mbParam.tBuf[3] = valve.portCur;  // 通道编号
                        byteCount = 4;
                } else if (0x02 == op_addr) /* 读地址 */
                {
                        ags_mbParam.tBuf[3] = ags_mbParam.mAddrs;  // 模块地址
                        byteCount = 4;
                } else if (0x03 == op_addr) /* 读版本 */
                {
                        ags_mbParam.tBuf[3] = (SOFT_VER >> 24) & 0xFF;
                        ags_mbParam.tBuf[4] = (SOFT_VER >> 16) & 0xFF;
                        ags_mbParam.tBuf[5] = (SOFT_VER >> 8) & 0xFF;
                        ags_mbParam.tBuf[6] = (SOFT_VER >> 0) & 0xFF;  // 模块版本号
                        byteCount = 7;
                } else if (0x07 == op_addr) /* 读波特率 */
                {
                        ags_mbParam.tBuf[3] = syspara.baudrate;  // 波特率
                        byteCount = 4;
                } else if (0x08 == op_addr) /* 读序列号 */
                {
                        I2CPageRead_Nbytes(ADDR_SN, LEN_SN, valve.SnCode);
                        ags_mbParam.tBuf[3] = valve.SnCode[0];
                        ags_mbParam.tBuf[4] = valve.SnCode[1];
                        ags_mbParam.tBuf[5] = valve.SnCode[2];
                        ags_mbParam.tBuf[6] = valve.SnCode[3];
                        ags_mbParam.tBuf[7] = valve.SnCode[4];
                        byteCount = 8;
                } else if (0x09 == op_addr) /* 读速度 */
                {
                        ags_mbParam.tBuf[3] = valve.spd; /* 转动速度 */
                        byteCount = 4;
                } else if (0x0A == op_addr) /* 读切换次数 */
                {
                        ags_mbParam.tBuf[3] = ((uint8*)&syspara.totalCnt)[3];
                        ags_mbParam.tBuf[4] = ((uint8*)&syspara.totalCnt)[2];
                        ags_mbParam.tBuf[5] = ((uint8*)&syspara.totalCnt)[1];
                        ags_mbParam.tBuf[6] = ((uint8*)&syspara.totalCnt)[0];
                        if (syspara.totalCnt != syspara.totalCntLst) {
                                I2CPageWrite_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, ((uint8*)&syspara.totalCnt));
                                syspara.totalCntLst = syspara.totalCnt;
                        }
                        byteCount = 7;
                } else if (0x0B == op_addr) /* 读回复方式 */
                {
                        I2CPageRead_Nbytes(ADDR_REPLY_MODE, LEN_REPLY_MODE, &syspara.replyMode);
                        ags_mbParam.tBuf[3] = syspara.replyMode;
                        byteCount = 4;
                } else if (0x0D == op_addr) /* 读半通道 */
                {
                        I2CPageRead_Nbytes(ADDR_HALF_SEAL, LEN_HALF_SEAL, &valve.bHalfSeal);
                        ags_mbParam.tBuf[3] = valve.bHalfSeal;
                        byteCount = 4;
                } else if (0x99 == op_addr) /* 读通道数 */
                {
                        I2CPageRead_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
                        ags_mbParam.tBuf[3] = valveFix.fix.portCnt;
                        byteCount = 4;
                } else {
                        ags_mbParam.sERR = ERR_MB_ADDR; /* 非法数据地址 */
                }
                /* 拼接 */
                reg_num = ModbusCRC16(&ags_mbParam.tBuf[0], byteCount); /* 获取CRC */
                ags_mbParam.tBuf[byteCount] = reg_num >> 8;
                byteCount++;
                ags_mbParam.tBuf[byteCount] = reg_num;
                byteCount++;
                /* 只有当地址不为广播地址且无报错时才回复 可以通过广播地址02查地址 */
                if (((MB_Broadcast_ADDR != ags_mbParam.tBuf[0]) ||
                     ((MB_Broadcast_ADDR == ags_mbParam.tBuf[0]) && 0x02 == op_addr)) &&
                    (ERR_NOT == ags_mbParam.sERR)) {
                        ags_mbSend(byteCount); /* 回复 */
                }
#ifdef DEBUG_AGS_MB
                printd("\r s:");
                for (uint8 i = 0; i < byteCount; i++)
                        printd(" %02x", ags_mbParam.tBuf[i]);
#endif
        } else {
                ags_mbParam.sERR = ERR_MB_DEVICE_ADDR; /* 非法从站设备地址 */
        }
}

/* 功能码6 */
void ags_mbPresetSingleHoldingRegister(void)
{
        uint16_t reg_num;
        uint8_t dvc_addr, op_addr, byteCount;

        dvc_addr = ags_mbParam.rBuf[0]; /* 第1字节 站号 模块地址 */
        op_addr = ags_mbParam.rBuf[2];  /* 第3字节 操作码 操作地址 */
        if ((dvc_addr == ags_mbParam.mAddrs) && BURN_ADDR != dvc_addr) {
                ags_mbParam.tBuf[0] = ags_mbParam.rBuf[0]; /* 设备地址 */
                ags_mbParam.tBuf[1] = ags_mbParam.rBuf[1]; /* 功能码 */
                ags_mbParam.tBuf[2] = ags_mbParam.rBuf[2]; /* 操作码/操作地址 */
                if (0x00 == op_addr)                     /* 写通道A */
                {
                        if ((POS_A == ags_mbParam.rBuf[3] || POS_B == ags_mbParam.rBuf[3]) && 6 == ags_mbParam.rCnt) {
                                if (valve.status == VALVE_RUN_END) {
                                        valve.portDes = ags_mbParam.rBuf[3];
                                        if (REPLYMODE_CUSTOM_1 == syspara.replyMode) {
                                                return;
                                        }
                                } else {
                                        ags_mbParam.sERR = ERR_MB_BUSY; /* 模块忙 */
                                }
                        } else {
                                ags_mbParam.sERR = ERR_MB_DATA; /* 操作数据无效 */
                        }
                } else if (0x01 == op_addr) /* 写地址 */
                {
                        // if (AGS_ADDR_MIN <= ags_mbParam.rBuf[3] && AGS_ADDR_MAX >= ags_mbParam.rBuf[3] &&
                        if (AGS_ADDR_MAX >= ags_mbParam.rBuf[3] &&
                            6 == ags_mbParam.rCnt) {
                                ags_mbParam.mAddrs = ags_mbParam.rBuf[3];
                                modbus.Address = ags_mbParam.mAddrs;
                                I2CPageWrite_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, &ags_mbParam.mAddrs);
                        } else {
                                ags_mbParam.sERR = ERR_MB_DATA; /* 操作数据无效 */
                        }
                } else if (0x06 == op_addr) /* 复位*/
                {
                        if (0x00 == ags_mbParam.rBuf[3] && 6 == ags_mbParam.rCnt) {
                                valve.status = VALVE_INITING;
                                valve.initStep = 0; /* 复位指令 */
                                valve.retryTms = 0;
                                valve.ErrBlinkTime = RETRY_TIME_OUT;
                        } else {
                                ags_mbParam.sERR = ERR_MB_DATA; /* 操作数据无效 */
                        }
                } else if (0x07 == op_addr) /* 写波特率 */
                {
                        if (BAUD_MIN <= ags_mbParam.rBuf[3] && BAUD_MAX >= ags_mbParam.rBuf[3] && 6 == ags_mbParam.rCnt) {
                                syspara.baudrate = (ags_mbParam.rBuf[3] == 2 ? BAUD_19200 : (ags_mbParam.rBuf[3] == 3 ? BAUD_38400 : BAUD_9600));
                                I2CPageWrite_Nbytes(ADDR_BAUD, LEN_BAUD, &syspara.baudrate);
                        } else {
                                ags_mbParam.sERR = ERR_MB_DATA; /* 操作数据无效 */
                        }
                } else if (0x08 == op_addr) /* 写序列号 */
                {
                        if (10 == ags_mbParam.rCnt) {
                                valve.SnCode[0] = ags_mbParam.rBuf[3];
                                valve.SnCode[1] = ags_mbParam.rBuf[4];
                                valve.SnCode[2] = ags_mbParam.rBuf[5];
                                valve.SnCode[3] = ags_mbParam.rBuf[6];
                                valve.SnCode[4] = ags_mbParam.rBuf[7];
                                I2CPageWrite_Nbytes(ADDR_SN, LEN_SN, valve.SnCode);
                        } else {
                                ags_mbParam.sERR = ERR_MB_DATA; /* 操作数据无效 */
                        }
                } else if (0x09 == op_addr) /* 写速度 */
                {
                        if (SPD_MIN <= ags_mbParam.rBuf[3] && SPD_MAX >= ags_mbParam.rBuf[3] && 6 == ags_mbParam.rCnt) {
                                valve.spd = ags_mbParam.rBuf[3];
                                I2CPageWrite_Nbytes(ADDR_SPD, LEN_SPD, &valve.spd);
                        } else {
                                ags_mbParam.sERR = ERR_MB_DATA; /* 操作数据无效 */
                        }
                } else if (0x0A == op_addr) /* 写切换次数 */
                {
                        if (9 == ags_mbParam.rCnt) {
                                ((uint8*)&syspara.totalCnt)[0] = ags_mbParam.rBuf[6];
                                ((uint8*)&syspara.totalCnt)[1] = ags_mbParam.rBuf[5];
                                ((uint8*)&syspara.totalCnt)[2] = ags_mbParam.rBuf[4];
                                ((uint8*)&syspara.totalCnt)[3] = ags_mbParam.rBuf[3];
                                I2CPageWrite_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, (uint8*)&syspara.totalCnt);
                        } else {
                                ags_mbParam.sERR = ERR_MB_DATA; /* 操作数据无效 */
                        }
                } else if (0x0B == op_addr) /* 设置通道状态指令回复方式 */
                {
                        // if (REPLYMODE_AGS <= ags_mbParam.rBuf[3] && REPLYMODE_CUSTOM_3 >= ags_mbParam.rBuf[3] &&
                        if (REPLYMODE_CUSTOM_3 >= ags_mbParam.rBuf[3] &&
                            6 == ags_mbParam.rCnt) {
                                syspara.replyMode = ags_mbParam.rBuf[3];
                                I2CPageWrite_Nbytes(ADDR_REPLY_MODE, LEN_REPLY_MODE, &syspara.replyMode);
                        } else {
                                ags_mbParam.sERR = ERR_MB_DATA; /* 操作数据无效 */
                        }
                } else if (0x0D == op_addr) /* 写半通道 */
                {
                        if ((OFF == ags_mbParam.rBuf[3] || ON == ags_mbParam.rBuf[3]) && 6 == ags_mbParam.rCnt) {
                                valve.bHalfSeal = ags_mbParam.rBuf[3];
                                I2CPageWrite_Nbytes(ADDR_HALF_SEAL, LEN_HALF_SEAL, &valve.bHalfSeal);
                        } else {
                                ags_mbParam.sERR = ERR_MB_DATA; /* 操作数据无效 */
                        }
                } else if (0x99 == op_addr) /* 写通道数 */
                {
                        if (CHANNEL_MIN <= ags_mbParam.rBuf[3] && CHANNEL_MAX >= ags_mbParam.rBuf[3] &&
                            6 == ags_mbParam.rCnt) {
                                valveFix.fix.portCnt = ags_mbParam.rBuf[3];
                                I2CPageWrite_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
                        } else {
                                ags_mbParam.sERR = ERR_MB_DATA; /* 操作数据无效 */
                        }
                }
                else {
                        ags_mbParam.sERR = ERR_MB_ADDR; /* 超出操作地址范围或者操作地址无效 */
                }
                ags_mbParam.tBuf[3] = ags_mbParam.rBuf[3];  // 端口编号
                byteCount = 4;
                /* 拼接 */
                reg_num = ModbusCRC16(&ags_mbParam.tBuf[0], byteCount); /* 获取CRC */
                ags_mbParam.tBuf[byteCount] = reg_num >> 8;
                byteCount++;
                ags_mbParam.tBuf[byteCount] = reg_num;
                byteCount++;
                /* 只有当地址不为广播地址且无报错时才回复 */
                if ((MB_Broadcast_ADDR != ags_mbParam.tBuf[0]) && (ERR_NOT == ags_mbParam.sERR)) {
                        ags_mbSend(byteCount); /* 回复 */
                }
#ifdef DEBUG_AGS_MB
                printd("\r s:");
                for (uint8 i = 0; i < byteCount; i++)
                        printd(" %02x", ags_mbParam.tBuf[i]);
#endif
        } else {
                ags_mbParam.sERR = ERR_MB_DEVICE_ADDR; /* 非法从站设备地址 */
        }
}

/* Modbus主线程 处理CRC 功能码 报错 */
void ags_mbProcess(void)
{
        if (MB_RECIVE_END == ags_mbParam.sRUN) {
                if (LEAST_RCV_CNT < ags_mbParam.rCnt) {
                        LED_WORK = !LED_WORK;
                        if (0 == ModbusCRC16(&ags_mbParam.rBuf[0], ags_mbParam.rCnt)) {
#ifdef DEBUG_AGS_MB
                                printd("\r r:");
                                for (uint8_t i = 0; i < ags_mbParam.rCnt; i++)
                                        printd(" %02x", ags_mbParam.rBuf[i]);
#endif
                                /* 确认模块存在并且工作正常 */
                                switch (ags_mbParam.rBuf[1]) {
                                        case GET_HOLDING_REGT: /* 功能码03 */
#if 0
                                                if (ags_mbParam.rCnt == 5) {
                                                        ags_mbReadHoldingRegisters();
                                                }
#else
                                                /* 读指令长度不匹配 重新接收 */
                                                if (ags_mbParam.rCnt != 5) {
                                                        ags_mbParam.rCnt = 0;
                                                        ags_mbParam.sRUN = MB_IDLE;
                                                        return;
                                                }
                                                ags_mbReadHoldingRegisters();
#endif
                                                break;
                                        case PRESET_HOLDING_sREGT: /* 功能码06 */
                                                ags_mbPresetSingleHoldingRegister();
                                                break;
                                        default:
                                                ags_mbParam.sERR = ERR_MB_FUN;
                                                break;
                                }
                        } else {
                                ags_mbParam.sERR = ERR_MB_DEVICE;
                        }
                        ags_mbParam.rCnt = 0;
                        ags_mbError();
                        ags_mbParam.sRUN = MB_IDLE;
                } else {
                        ags_mbParam.rCnt = 0;
                        ags_mbError();
                        ags_mbParam.sRUN = MB_IDLE;
                }
        }
}
