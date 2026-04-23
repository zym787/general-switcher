/* 包含头文件 ----------------------------------------------------------------*/
#include "common.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
ELAB_TAG("modbus"); /* elog 标签 */
/* 私有变量 ------------------------------------------------------------------*/
REG_VALUE R_value;
Modbus_T modbus;
uint8_t Rx_Buffer[MODBUS_LENGTH];
uint8_t Tx_Buffer[MODBUS_LENGTH];

__IO uint16_t g_mb_Holding[MODBUS_NUMBER] = {0}; /* 保持寄存器buf */

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
#if (SUPPORT_FUN_CODE_03H != 0)
static void mb_03H(void);
#endif
#if (SUPPORT_FUN_CODE_06H != 0)
static void mb_06H(void);
#endif
#if (SUPPORT_FUN_CODE_10H != 0)
static void mb_10H(void);
#endif
void mb_ReadHolding(uint16_t _regAddr);
void mb_ReadHoldingOnce(void);
void mb_WriteHolding(uint16_t _regAddr, uint16_t _value);
void Get_Chip_UID(uint32_t *pID);
/* 函数体 --------------------------------------------------------------------*/

/**
 * @brief     : 初始化Modbus协议
 */
void mb_Init(void)
{
        RX_EN(); /* 开机为接收模式 */
        I2CPageRead_Nbytes(ADDR_BAUD, LEN_BAUD, &syspara.baudrate);
        if (BAUD_38400 < syspara.baudrate) {
                syspara.baudrate = BAUD_9600; /* 9600 */
        }
        Usart2_Init(36, BaudRate_V[syspara.baudrate]); /* UART2 19200bps */
        delay_ms(100);
        Usart3_Init(36, BaudRate_V[syspara.baudrate]); /* UART3 19200bps */
        delay_ms(100);
        TIM3_Init(BaudRate_Time[syspara.baudrate], 71);  // 45us--0.45ms
        delay_ms(100);

        elog_debug("USART2/3 Init, baudrate: %d", BaudRate_V[syspara.baudrate]);

        /* 参数配置 */
        modbus.RunState = MB_IDLE;
        modbus.ErrorState = MB_ERROR_NONE;
        modbus.times = 0;
        modbus.ReciveCount = 0;
        modbus.Address = 1;

        memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
        memset(Tx_Buffer, 0, sizeof(Tx_Buffer));
        uint8_t i = 0;
        do {
                MB_SET_HOLDING(i, 0);
                i++;
        } while (i < MODBUS_NUMBER);

        mb_ReadHoldingOnce();
}

/**
 * @brief    在定时器中断中定时调用,用于处理超时检测
 */
void mb_TimesProcess(void)
{
        if (modbus.RunState & MB_BUSY) {
                /* 总线检测 */
                if (modbus.times < BUS_IDLE_TIME) {
                        modbus.times++;
                } else {
                        /* 总线进入空闲,帧结束或开始 */
                        if (modbus.RunState == MB_RECIVE_ERR) {
                                /* 接收过程中,有出现数据存储空间溢出或间隔时间超过T1.5 */
                                modbus.ErrorState = MB_ERROR_DEVICE;
                                modbus.RunState = MB_IDLE;
                        } else if (modbus.RunState == MB_NO_RESPONSE) {
                                modbus.RunState = MB_IDLE;
                        } else if (modbus.RunState == MB_RECIVE) {
                                modbus.RunState = MB_RECIVE_END;
                        }
                }
        }
}

/**
 * @brief     : 发送数据
 * @param    _length
 */
void mb_SendBuffer(uint8_t _length)
{
// #if (DEBUG_MODBUS != 0)
//         XF_LOG_BUFFER_HEX(Tx_Buffer, _length);
// #endif
        // elog_debug("%s(): L%d", __FUNCTION__, _length);
        TX_EN();
        if (_length) {
                modbus.RunState = MB_SEND;
                for (uint8_t cnt = 0; cnt < _length; cnt++) {
                        while ((USART3->SR & 0x40) == 0)
                                ; /* 等待发送结束 */
                        USART3->DR = Tx_Buffer[cnt];
                        modbus.times = 0; /* 重新计时 */
                }
        }
        while ((USART3->SR & 0x40) == 0)
                ; /* 等待发送结束 */
        RX_EN();

        if (_length) {
                modbus.RunState = MB_SEND;
                for (uint8_t cnt = 0; cnt < _length; cnt++) {
                        while ((USART2->SR & 0x40) == 0)
                                ; /* 等待发送结束 */
                        USART2->DR = Tx_Buffer[cnt];
                        modbus.times = 0; /* 重新计时 */
                }
        }
        while ((USART2->SR & 0x40) == 0)
                ; /* 等待发送结束 */
        modbus.RunState = MB_IDLE;
        modbus.ReciveCount = 0;
}

/**
 * @brief     : 接收数据
 * @param    _recStr
 */
void mb_Receive(uint8_t _recStr)
{
        modbus.times = 0; /* 重新计时 */
        if (modbus.RunState == MB_IDLE && !modbus.ReciveCount) {
                /* 空闲并且数据处理结束,可以进行新的接收 */
                if (modbus.Address == _recStr || MB_ALLSLAVEADDR == _recStr) {
                        modbus.RunState = MB_RECIVE;
                        modbus.ErrorState = MB_ERROR_NONE;
                        modbus.ReciveCount = 1;
                        Rx_Buffer[0] = _recStr;
                } else {
                        /* 非本设备地址,或非当前查询的设备,且非广播地址*/
                        modbus.RunState = MB_NO_RESPONSE;
                        modbus.ErrorState = MB_ERROR_DEVICE;
                        elog_error("%d Address Error", _recStr);
                }
        } else if (modbus.RunState == MB_RECIVE) {
                /* 如果溢出或者传输过程出现时间间隔超过T1.5,都不再接收*/
                if (modbus.ReciveCount < MODBUS_LENGTH && modbus.times < FRAME_ERR_TIME) {
                        Rx_Buffer[modbus.ReciveCount] = _recStr;
                } else {
                        modbus.RunState = MB_RECIVE_ERR;
                        elog_error("%d Frame Error", _recStr);
                }
                ++modbus.ReciveCount;
        }
}

/**
 * @brief    处理错误
 */
void mb_Error(void)
{
        uint16_t crc16;

        if (modbus.ErrorState == MB_ERROR_FUNC || modbus.ErrorState == MB_ERROR_ADDR ||
            modbus.ErrorState == MB_ERROR_DATA) {
                /* 从模式,发送异常响应 */
                Tx_Buffer[0] = Rx_Buffer[0];
                Tx_Buffer[1] = Rx_Buffer[1] | 0x80;
                Tx_Buffer[2] = modbus.ErrorState;
                crc16 = ModbusCRC16((uint8_t *)&Tx_Buffer, 3);
                Tx_Buffer[3] = crc16 >> 8;
                Tx_Buffer[4] = crc16 & 0xFF;
                if (Rx_Buffer[0] != MB_Broadcast_ADDR) {
                        mb_SendBuffer(5);
                }
                elog_error("Error %d", modbus.ErrorState);
        }
        modbus.ErrorState = MB_ERROR_NONE;
}

/**
 * @brief     : 03 读保持寄存器
 */
#if (SUPPORT_FUN_CODE_03H != 0)
static void mb_03H(void)
{
        /*
         从机地址为01H。保持寄存器的起始地址为0010H，结束地址为0011H。该次查询总共访问2个保持寄存器。
         主机发送:
                 01 从机地址
                 03 功能码
                 00 寄存器地址高字节
                 10 寄存器地址低字节
                 00 寄存器数量高字节
                 02 寄存器数量低字节
                 C5 CRC高字节
                 CE CRC低字节

         从机应答: 	保持寄存器的长度为2个字节。对于单个保持寄存器而言，寄存器高字节数据先被传输，
                                 低字节数据后被传输。保持寄存器之间，低地址寄存器先被传输，高地址寄存器后被传输。
                 01 从机地址
                 03 功能码
                 04 字节数
                 12 数据1高字节(0010H)
                 34 数据1低字节(0010H)
                 02 数据2高字节(0011H)
                 03 数据2低字节(0100H)
                 FF CRC高字节
                 F4 CRC低字节

         读一个保持寄存器例子:
                 发送：	01 03 00 10 00 01            85 CF ---- 读 0010H一个寄存器内容
                 返回：	01 03 02 12 34               B5 33 ---- 返回10H功能码写入的内容（10H功能码会介绍）
 */
        /* 解析 */
        uint8_t recAddr = Rx_Buffer[0];                             /* 设备地址 */
        uint16_t recRegAddr = ((Rx_Buffer[2] << 8) | Rx_Buffer[3]); /* 寄存器首地址 */
        uint16_t regNum = ((Rx_Buffer[4] << 8) | Rx_Buffer[5]);     /* 寄存器数量 */
#if (DEBUG_MODBUS != 0)
        elog_debug(" Addr %d, RegAddr %d, regNum %d", recAddr, recRegAddr, regNum);
#endif

        /* 响应 */
        Tx_Buffer[0] = recAddr;      /* 第1个字节 设备地址 */
        Tx_Buffer[1] = Rx_Buffer[1]; /* 第2个字节 功能码 */
        Tx_Buffer[2] = 2 * regNum;   /* 第3个字节 寄存器数量 */

        /* 确保读取的地址存在 */
        if (recRegAddr + regNum > MODBUS_NUMBER) {
                elog_error(" Reg Address Over");
                modbus.ErrorState = MB_ERROR_ADDR;
                mb_Error();
                return;
        }

        /* 返回保持寄存器内的数据 */
        for (uint8_t i = 0; i < regNum; i++) {
                mb_ReadHolding(recRegAddr + i); /* 更新保持寄存器 */
                uint16_t tempRead = MB_GET_HOLDING(recRegAddr + i);
                Tx_Buffer[2 * i + 3] = tempRead >> 8;
                Tx_Buffer[2 * i + 4] = tempRead & 0xFF;
#if (DEBUG_MODBUS != 0)
                elog_debug(" R%d|Read%02X(%d): %d", i, recRegAddr + i, recRegAddr + i, tempRead);
#endif
        }

        /* 拼接CRC */
        uint16_t crc16 = ModbusCRC16((uint8_t *)&Tx_Buffer[0], 2 * regNum + 3);

        Tx_Buffer[2 * regNum + 3] = crc16 >> 8;   /* 最后2个字节 CRC高字节 */
        Tx_Buffer[2 * regNum + 4] = crc16 & 0xFF; /* 最后1个字节 CRC低字节 */
#if (DEBUG_MODBUS != 0)
        elog_debug(" CRC:%04X  (%d)%02X, (%d)%02X L:%d", crc16, 2 * regNum + 3, Tx_Buffer[2 * regNum + 3],
                   2 * regNum + 4, Tx_Buffer[2 * regNum + 4]);
#endif
#if (DEBUG_MODBUS != 0)
        printd("\r\n TxBuffer:");
        for (uint8_t i = 0; i < 2 * regNum + 5; i++) {
                printd(" (%d)%02X", i, Tx_Buffer[i]);
        }
        printd("\r\n");
#endif

        /* 只有无报错时才回复 */
        if (modbus.ErrorState == MB_ERROR_NONE) {
                mb_SendBuffer(2 * regNum + 5);
#if (DEBUG_MODBUS != 0)
                // XF_LOG_BUFFER_HEX(Tx_Buffer, 2 * regNum + 5);
#endif
        }
}
#endif

/**
 * @brief     : 06 写单个保持寄存器
 */
#if (SUPPORT_FUN_CODE_06H != 0)
static void mb_06H(void)
{
        /*
                写保持寄存器。注意06指令只能操作单个保持寄存器，10H指令可以设置单个或多个保持寄存器
                主机发送:
                        01 从机地址
                        06 功能码
                        00 寄存器地址高字节
                        10 寄存器地址低字节
                        67 数据1高字节
                        4A 数据1低字节
                        23 CRC校验高字节
                        C8 CRC校验低字节

                从机响应:
                        01 从机地址
                        06 功能码
                        00 寄存器地址高字节
                        10 寄存器地址低字节
                        67 数据1高字节
                        4A 数据1低字节
                        23 CRC校验高字节
                        C8 CRC校验低字节

                例子:
                        发送：	01 06 00 10 67 4A  23 C8    ---- 将0010地址寄存器设置为67 4A
                        返回：	01 06 00 10 67 4A  23 C8    ---- 返回同样数据
*/
        /* 解析 */
        uint8_t recAddr = Rx_Buffer[0];                             /* 设备地址 */
        uint16_t recRegAddr = ((Rx_Buffer[2] << 8) | Rx_Buffer[3]); /* 寄存器首地址 */
        uint16_t regValve = ((Rx_Buffer[4] << 8) | Rx_Buffer[5]);   /* 寄存器值 */

        /* 响应 */
        Tx_Buffer[0] = recAddr;           /* 第1个字节 设备地址 */
        Tx_Buffer[1] = Rx_Buffer[1];      /* 第2个字节 功能码 */
        Tx_Buffer[2] = recRegAddr >> 8;   /* 第3个字节 寄存器地址高字节 */
        Tx_Buffer[3] = recRegAddr & 0xFF; /* 第4个字节 寄存器地址低字节 */

        /* 确保写入的地址存在 */
        if (recRegAddr > MODBUS_NUMBER) {
                elog_error(" Reg Address Over");
                modbus.ErrorState = MB_ERROR_ADDR;
                mb_Error();
                return;
        }

        /* 写入保持寄存器内的数据 */
        mb_WriteHolding(recRegAddr, regValve);

        /* 返回保持寄存器内的数据 */
        Tx_Buffer[4] = MB_GET_HOLDING(recRegAddr) >> 8; /* 第5个字节 寄存器值高字节 */
        Tx_Buffer[5] = MB_GET_HOLDING(recRegAddr) & 0xFF; /* 第6个字节 寄存器值低字节 */
#if (DEBUG_MODBUS != 0)
        elog_debug(" R%d|Write%02X(%d): %d", 0, recRegAddr, recRegAddr, regValve);
#endif

        /* 拼接CRC */
        uint16_t crc16 = ModbusCRC16((uint8_t *)&Tx_Buffer[0], 6);
        Tx_Buffer[6] = crc16 >> 8;
        Tx_Buffer[7] = crc16 & 0xFF;

#if (DEBUG_MODBUS != 0)
        elog_debug(" CRC:%04X  (%d)%02X, (%d)%02X L:%d", crc16, Tx_Buffer[6], Tx_Buffer[6], Tx_Buffer[7], Tx_Buffer[7],
                   8);
#endif
#if (DEBUG_MODBUS != 0)
        printd("\r\n TxBuffer:");
        for (uint8_t i = 0; i < 8; i++) {
                printd(" (%d)%02X", i, Tx_Buffer[i]);
        }
        printd("\r\n");
#endif

        /* 只有无报错时才回复 */
        if (modbus.ErrorState == MB_ERROR_NONE) {
                mb_SendBuffer(8);
#if (DEBUG_MODBUS != 0)
                // XF_LOG_BUFFER_HEX(Tx_Buffer, 8);
#endif
        }
}
#endif

/**
 * @brief     : 10 写多个保持寄存器
 */
#if (SUPPORT_FUN_CODE_10H != 0)
static void mb_10H(void)
{
        /*
        主机发送:
                01 从机地址
                10 功能码
                00 寄存器起始地址高字节
                10 寄存器起始地址低字节
                00 寄存器数量高字节
                02 寄存器数量低字节
                04 字节数
                12 数据1高字节
                34 数据1低字节
                02 数据2高字节
                03 数据2低字节
                F7 CRC校验高字节
                74 CRC校验低字节

        从机响应:
                01 从机地址
                10 功能码
                00 寄存器地址高字节
                10 寄存器地址低字节
                00 寄存器数量高字节
                02 寄存器数量低字节
                40 CRC校验高字节
                0D CRC校验低字节

        例子:
                发送：	01 10 00 10 00 02 04 12 34 02 03 F7 74    ----  向0010H~0011H写入12 34 02 03 四个字节数据
                返回：	01 10 00 10 00 02 40 0D                   ----  返回内容

*/
        uint8_t recAddr = Rx_Buffer[0];                             /* 设备地址 */
        uint16_t recRegAddr = ((Rx_Buffer[2] << 8) | Rx_Buffer[3]); /* 寄存器首地址 */
        uint16_t regNum = ((Rx_Buffer[4] << 8) | Rx_Buffer[5]);     /* 寄存器数量 */
        uint8_t byteNum = Rx_Buffer[6];                             /* 字节数 */

        /* 响应 */
        Tx_Buffer[0] = recAddr;           /* 第1个字节 设备地址 */
        Tx_Buffer[1] = Rx_Buffer[1];      /* 第2个字节 功能码 */
        Tx_Buffer[2] = recRegAddr >> 8;   /* 第3个字节 寄存器地址高字节 */
        Tx_Buffer[3] = recRegAddr & 0xFF; /* 第4个字节 寄存器地址低字节 */
        Tx_Buffer[4] = regNum >> 8;       /* 第5个字节 寄存器数量高字节 */
        Tx_Buffer[5] = regNum & 0xFF;     /* 第6个字节 寄存器数量低字节 */

        /* 确保写入的地址存在 */
        if (recRegAddr + regNum > MODBUS_NUMBER) {
                elog_error(" Reg Address Over");
                modbus.ErrorState = MB_ERROR_ADDR;
                mb_Error();
                return;
        }
        /* 确保写入字节数正确 */
        if (byteNum != regNum * 2) {
                modbus.ErrorState = MB_ERROR_DATA;
                mb_Error();
                return;
        }

        /* 写入保持寄存器数据 */
        for (uint8_t i = 0; i < regNum; i++) {
                uint16_t tempWrite = (uint16_t)(Rx_Buffer[2 * i + 7] << 8) | Rx_Buffer[2 * i + 8];
                MB_SET_HOLDING(recRegAddr + i, tempWrite);
#if (DEBUG_MODBUS != 0)
                elog_debug(" R%d|Write%02X(%d): %d", i, recRegAddr + i, recRegAddr + i, tempWrite);
#endif
        }

        /* 拼接CRC */
        uint16_t crc16 = ModbusCRC16((uint8_t *)&Tx_Buffer[0], 6);
        Tx_Buffer[6] = crc16 >> 8;
        Tx_Buffer[7] = crc16 & 0xFF;

#if (DEBUG_MODBUS != 0)
        elog_debug(" CRC:%04X  (%d)%02X, (%d)%02X L:%d", crc16, Tx_Buffer[6], Tx_Buffer[6], Tx_Buffer[7], Tx_Buffer[7],
                   8);
#endif
#if (DEBUG_MODBUS != 0)
        printd("\r\n TxBuffer:");
        for (uint8_t i = 0; i < 8; i++) {
                printd(" (%d)%02X", i, Tx_Buffer[i]);
        }
        printd("\r\n");
#endif

        /* 只有无报错时才回复 */
        if (modbus.ErrorState == MB_ERROR_NONE) {
                mb_SendBuffer(8);
#if (DEBUG_MODBUS != 0)
                // XF_LOG_BUFFER_HEX(Tx_Buffer, 8);
#endif
        }
}
#endif

void mb_Poll(void)
{
        if (MB_RECIVE_END == modbus.RunState) {
                if (LEAST_RCV_CNT < modbus.ReciveCount) {
                        LED_WORK = !LED_WORK;
// #if (DEBUG_MODBUS != 0)
//                         XF_LOG_BUFFER_HEX(Rx_Buffer, modbus.ReciveCount);
// #endif
                        if (0 == ModbusCRC16(&Rx_Buffer[0], modbus.ReciveCount)) {
                                /* 第2字节 功能码 */
                                /* 确认模块存在并且工作正常 */
                                switch (Rx_Buffer[1]) {
#if (SUPPORT_FUN_CODE_03H != 0)
                                        case FUN_CODE_03H:
                                                mb_03H();
                                                break;
#endif
#if (SUPPORT_FUN_CODE_06H != 0)
                                        case FUN_CODE_06H:
                                                mb_06H();
                                                break;
#endif
#if (SUPPORT_FUN_CODE_10H != 0)
                                        case FUN_CODE_10H:
                                                mb_10H();
                                                break;
#endif
                                        default:
                                                modbus.ErrorState = MB_ERROR_FUNC;
                                                break;
                                }
                        } else {
                                /* CRC校验失败 */
                                modbus.ErrorState = MB_ERROR_DEVICE;
                                elog_error("CRC Error %d", modbus.ErrorState);
                        }
                        modbus.ReciveCount = 0;
                        
                } else {
                        /* 数据长度不足 即无效数据 */
                        modbus.ReciveCount = 0;
                }
                if (MB_ERROR_NONE != modbus.ErrorState) {
                        mb_Error();
                }
                modbus.ErrorState = MB_ERROR_NONE;
                modbus.RunState = MB_ERROR_NONE;
        }
}

/**
 * @brief     : 更新保持寄存器
 * @detail    : 分区 根据寄存器地址,更新保持寄存器的值,以便在读保持寄存器时返回正确的数据
 */
void mb_ReadHolding(uint16_t _regAddr)
{
        /* 只读参数寄存器 STATUS */
        if (MB_R_STATUS_CHANNEL_CUR <= _regAddr && MB_R_STATUS_COUNT_2 >= _regAddr) {
                MB_SET_HOLDING(MB_R_STATUS_CHANNEL_CUR, valve.portCur);        /* 当前通道 */
                MB_SET_HOLDING(MB_R_STATUS_CONTROL_STATE, valve.status);       /* 当前状态 */
                MB_SET_HOLDING(MB_R_STATUS_MOVE_TIME, syspara.lastTime);       /* 上次运动耗时 */
                MB_SET_HOLDING(MB_R_STATUS_SW_CODE, (SOFT_VER_NUM >> 16));     /* 软件代码*/
                MB_SET_HOLDING(MB_R_STATUS_SW_VERSION, SOFT_REVISION);         /* 软件版本*/
                MB_SET_HOLDING(MB_R_STATUS_COUNT_1, syspara.burnCnt >> 16);    /* 老化次数1 */
                MB_SET_HOLDING(MB_R_STATUS_COUNT_2, syspara.burnCnt & 0xFFFF); /* 老化次数2 */
                return;
        }
        /* 运行参数1寄存器 OPERATE1 */
        else if (MB_RW_OPERATE1_ADDRESS <= _regAddr && MB_RW_OPERATE1_MOVE_COUNT_2 >= _regAddr) {
                MB_SET_HOLDING(MB_RW_OPERATE1_ADDRESS, ags_mbParam.mAddrs);             /* 地址 */
                MB_SET_HOLDING(MB_RW_OPERATE1_SPEED, valve.spd);                        /* 速度 */
                MB_SET_HOLDING(MB_RW_OPERATE1_DIRECTION, 0xFF);                         /* 方向始终为就近方向 */
                MB_SET_HOLDING(MB_RW_OPERATE1_BAUDRATE, syspara.baudrate);              /* 波特率 */
                MB_SET_HOLDING(MB_RW_OPERATE1_MOVE_COUNT_1, syspara.totalCnt >> 16);    /* 移动次数1 */
                MB_SET_HOLDING(MB_RW_OPERATE1_MOVE_COUNT_2, syspara.totalCnt & 0xFFFF); /* 移动次数2 */
                return;
        }
        /* 序列号寄存器 USER */
        else if (MB_RW_USER_SN_1 <= _regAddr && MB_RW_USER_SN_3 >= _regAddr) {
                MB_SET_HOLDING(MB_RW_USER_SN_1, valve.SnCode[0] << 8 | valve.SnCode[1]); /* 序列号 */
                MB_SET_HOLDING(MB_RW_USER_SN_2, valve.SnCode[2] << 8 | valve.SnCode[3]); /* 序列号 */
                MB_SET_HOLDING(MB_RW_USER_SN_3, valve.SnCode[4] << 8 | 0x00);                   /* 序列号 */
                return;
        }
        /* 出厂参数1寄存器 FACTORY1 */
        else if (MB_R_FACTORY1_UID_X0 <= _regAddr && MB_R_FACTORY1_UID_Z1 >= _regAddr) {
                return;
        }
        /* 出厂参数2寄存器 FACTORY2 */
        else if (MB_RW_FACTORY2_VALVE_TYPE <= _regAddr && MB_RW_FACTORY2_COMPEN_CCW >= _regAddr) {
                MB_SET_HOLDING(MB_RW_FACTORY2_CHANNEL_NUM, valveFix.fix.portCnt); /* 通道数 */
                MB_SET_HOLDING(MB_RW_FACTORY2_HALF_MODE, valve.bHalfSeal);        /* 半通道 */
                MB_SET_HOLDING(MB_RW_FACTORY2_COMPEN_ORG, valve.fixOrg);          /* 原点补偿 */
                MB_SET_HOLDING(MB_RW_FACTORY2_COMPEN_DIR, valveFix.fix.org);      /* 方向补偿 */
                return;
        } else {
                // MB_SET_HOLDING(_regAddr, 0); /* 其他寄存器暂时返回0 */
                return;
        }
}

void mb_ReadHoldingOnce(void)
{
        uint32_t uid[3];
        Get_Chip_UID(uid);
        /* 出厂参数1寄存器 FACTORY1 */
        MB_SET_HOLDING(MB_R_FACTORY1_UID_X0, uid[0] >> 16);    /* UID */
        MB_SET_HOLDING(MB_R_FACTORY1_UID_X1, uid[0] & 0xFFFF); /* UID */
        MB_SET_HOLDING(MB_R_FACTORY1_UID_Y0, uid[1] >> 16);    /* UID */
        MB_SET_HOLDING(MB_R_FACTORY1_UID_Y1, uid[1] & 0xFFFF); /* UID */
        MB_SET_HOLDING(MB_R_FACTORY1_UID_Z0, uid[2] >> 16);    /* UID */
        MB_SET_HOLDING(MB_R_FACTORY1_UID_Z1, uid[2] & 0xFFFF); /* UID */
}

void mb_WriteHolding(uint16_t _regAddr, uint16_t _value)
{
        MB_SET_HOLDING(_regAddr, _value);
#if (DEBUG_MODBUS != 0)
        elog_debug(" Write Holding %02X(%d): %d", _regAddr, _regAddr, _value);
#endif
        /* 控制指令寄存器 CTRL */
        // if (MB_RW_CTRL_SET_NORMAL <= _regAddr && MB_RW_CTRL_SET_GOD_MODE >= _regAddr) {
        if (MB_RW_CTRL_SET_GOD_MODE >= _regAddr) {
                switch (_regAddr) {
                        case MB_RW_CTRL_SET_NORMAL:
                                valve.portDes = _value; /* 更新目标通道 */
                                break;
                        // case MB_RW_CTRL_SET_CW:
                        //         valve.dir = 0x00;       /* CW切换 */
                        //         valve.portDes = _value; /* 更新目标通道 */
                        //         break;
                        // case MB_RW_CTRL_SET_CCW:
                        //         valve.dir = 0x01;       /* CCW切换 */
                        //         valve.portDes = _value; /* 更新目标通道 */
                        //         break;
                        case MB_RW_CTRL_SET_FREE:
                                valve.spd = MB_GET_HOLDING(MB_RW_OPERATE1_SPEED);     /* 速度 */
                                speed[AXSV] = accel[AXSV] = 100;
                                decel[AXSV] = 200;
                                speed[AXSV] *= (valve.spd);
                                speed[AXSV] *= (rdc.rate);
                                accel[AXSV] *= (valve.spd);
                                accel[AXSV] *= (rdc.rate);
                                decel[AXSV] *= (valve.spd);
                                decel[AXSV] *= (rdc.rate);
                                valve.portDes = _value; /* 更新目标通道 */
                                break;
                        case MB_RW_CTRL_SET_ZERO:
                                valve.status = VALVE_INITING;
                                valve.initStep = 0; /* 复位指令 */
                                valve.bNewInit = 0xff;
                                valve.ErrBlinkTime = RETRY_TIME_OUT;
                                break;
                        case MB_RW_CTRL_SET_GOD_MODE:
                                if (_value == AGING_CODE) {
                                        syspara.GodMode = GD_AGING; /* 进入老化模式 */
                                } else if (_value == SECURITY_CODE) {
                                        syspara.GodMode = GD_FACTORY; /* 进入工厂模式 */
                                } else if (_value == NORMAL_CODE) {
                                        syspara.GodMode = GD_NORMAL; /* 进入正常模式 */
                                        syspara.burnCnt = 0;         /* 老化次数清零 */
                                        I2CPageWrite_Nbytes(ADDR_BURN_CNT, LEN_BURN_CNT, (uint8_t *)&syspara.burnCnt);
                                } else {
                                        syspara.GodMode = GD_NORMAL;
                                }
                                I2CPageWrite_Nbytes(ADDR_GOD_MODE, LEN_GOD_MODE, &syspara.GodMode);
                                break;
                        default:
                                break;
                }
        }
        /* 运行参数1寄存器 OPERATE1 */
        if (MB_RW_OPERATE1_ADDRESS <= _regAddr && MB_RW_OPERATE1_MOVE_COUNT_2 >= _regAddr) {
                switch (_regAddr) {
                        case MB_RW_OPERATE1_ADDRESS:
                                ags_mbParam.mAddrs = _value;
                                I2CPageWrite_Nbytes(ADDR_MODULE_NUM, LEN_MODULE_NUM, &ags_mbParam.mAddrs);
                                break;
                        case MB_RW_OPERATE1_SPEED:
                                valve.spd = _value;
                                I2CPageWrite_Nbytes(ADDR_SPD, LEN_SPD, &valve.spd);
                                break;
                        // case MB_RW_OPERATE1_DIRECTION:
                        //         valve.dir = _value;
                        //         break;
                        case MB_RW_OPERATE1_BAUDRATE:
                                syspara.baudrate = (_value == 2 ? BAUD_19200 : (_value == 3 ? BAUD_38400 : BAUD_9600));
                                I2CPageWrite_Nbytes(ADDR_BAUD, LEN_BAUD, &syspara.baudrate);
                                break;
                        case MB_RW_OPERATE1_MOVE_COUNT_1:
                                syspara.totalCnt |= _value << 16; /* 移动次数1 */
                                I2CPageWrite_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, (uint8_t *)&syspara.totalCnt);
                                break;
                        case MB_RW_OPERATE1_MOVE_COUNT_2:
                                syspara.totalCnt |= _value; /* 移动次数2 */
                                I2CPageWrite_Nbytes(ADDR_TOTAL_CNT, LEN_TOTAL_CNT, (uint8_t *)&syspara.totalCnt);
                                break;
                        default:
                                break;
                }
        }
        /* 序列号寄存器 USER */
        if (MB_RW_USER_SN_1 <= _regAddr && MB_RW_USER_SN_3 >= _regAddr) {
                switch (_regAddr) {
                        case MB_RW_USER_SN_1:
                                valve.SnCode[0] = _value >> 8;
                                valve.SnCode[1] = _value & 0xFF;
                                I2CPageWrite_Nbytes(ADDR_SN, LEN_SN, valve.SnCode);
                                break;
                        case MB_RW_USER_SN_2:
                                valve.SnCode[2] = _value >> 8;
                                valve.SnCode[3] = _value & 0xFF;
                                I2CPageWrite_Nbytes(ADDR_SN, LEN_SN, valve.SnCode);
                                break;
                        case MB_RW_USER_SN_3:
                                valve.SnCode[4] = _value >> 8;
                                I2CPageWrite_Nbytes(ADDR_SN, LEN_SN, valve.SnCode);
                                break;
                        default:
                                break;
                }
        }

        /******************** 工厂模式寄存器 FACTORY *******************/
        if (GD_NORMAL == syspara.GodMode) {
                return;
        }
#if (DEBUG_MODBUS != 0)
        elog_debug(" Write Factory Holding %02X(%d): %d", _regAddr, _regAddr, _value);
#endif
        /* 出厂参数1寄存器 FACTORY1 */
        //  if (MB_R_FACTORY1_UID_X0 <= _regAddr && MB_R_FACTORY1_UID_Z1 >= _regAddr) {
        //          /* UID寄存器为只读,不处理写入 */
        //  }
        if (MB_RW_FACTORY1_DATA_01 <= _regAddr && MB_RW_FACTORY1_DATA_04 >= _regAddr) {
                switch (_regAddr) {
                        case MB_RW_FACTORY1_DATA_01:
                                break;
                        case MB_RW_FACTORY1_DATA_02:
                                break;
                        case MB_RW_FACTORY1_DATA_03:
                                break;
                        case MB_RW_FACTORY1_DATA_04:
                                break;
                        default:
                                break;
                }
        }
        /* 工厂模式安全码寄存器 FACTORY2_SECURE_CODE */
        // if (MB_RW_FACTORY2_SECURE_CODE == _regAddr) {
        //         if (_value == SECURITY_CODE) {
        //                 syspara.GodMode = GD_FACTORY; /* 进入工厂模式 */
        //         }
        // }

        /* 出厂参数2寄存器 FACTORY2 */
        if (MB_RW_FACTORY2_VALVE_TYPE <= _regAddr && MB_RW_FACTORY2_SECURE_CODE > _regAddr ||
            MB_RW_FACTORY2_SECURE_CODE < _regAddr && MB_RW_FACTORY2_COMPEN_CCW >= _regAddr) {
                switch (_regAddr) {
                        case MB_RW_FACTORY2_VALVE_TYPE:
                                break;
                        case MB_RW_FACTORY2_CTRL_MODE:
                                break;
                        case MB_RW_FACTORY2_CHANNEL_NUM:
                                valveFix.fix.portCnt = _value;
                                I2CPageWrite_Nbytes(ADDR_PORT_CNT, LEN_PORT_CNT, &valveFix.fix.portCnt);
                                break;
                        case MB_RW_FACTORY2_HALF_MODE:
                                valve.bHalfSeal = _value;
                                I2CPageWrite_Nbytes(ADDR_HALF_SEAL, LEN_HALF_SEAL, &valve.bHalfSeal);
                                break;
                        case MB_RW_FACTORY2_REPLY_MODE:
                                break;
                        case MB_RW_FACTORY2_COMPEN_ORG:
                                valve.fixOrg = _value;
                                I2CPageWrite_Nbytes(ADDR_VALVE_FIX, LEN_VALVE_FIX, &valve.fixOrg);
                                break;
                        case MB_RW_FACTORY2_COMPEN_DIR:
                                I2CPageWrite_Nbytes(ADDR_DIR_FIX, LEN_DIR_FIX, &valveFix.fix.dirGap);
                                break;
                        case MB_RW_FACTORY2_COMPEN_CW:
                                // valve.fDirCw = _value;
                                // _value <<= 8;
                                // _value |= valve.fDirCCw;
                                // I2CPageWrite_Nbytes(ADDR_DIR_SD, LEN_DIR_SD, (uint8_t *)&_value);
                                break;
                        case MB_RW_FACTORY2_COMPEN_CCW:
                                // valve.fDirCCw = _value;
                                // _value <<= 8;
                                // _value |= valve.fDirCw;
                                // I2CPageWrite_Nbytes(ADDR_DIR_SD, LEN_DIR_SD, (uint8_t *)&_value);
                                break;
                        default:
                                break;
                }
        }
}

#define UID_BASE_ADDRESS 0x1FFFF7E8

void Get_Chip_UID(uint32_t *pID)
{
        // 强制类型转换，将地址转换为 uint32_t 指针并取值
        pID[0] = *(__IO uint32_t *)(UID_BASE_ADDRESS);
        pID[1] = *(__IO uint32_t *)(UID_BASE_ADDRESS + 4);
        pID[2] = *(__IO uint32_t *)(UID_BASE_ADDRESS + 8);
}
