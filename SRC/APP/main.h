#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef _MAIN_GLOBALS_
#define PEXT
#else
#define PEXT extern
#endif

#define SOFT_VER        130
#define SOFT_VER_C      "V1.3.0r1"
// V1.2.9r7     2024.09.26  原点补偿做减速区间
// V1.3.0r0     2025.01.14  修改通信丢包
// V1.3.0r1     2025.03.05  修改序列号地址重复

#ifdef A12_901
#define IO_OUT          PAout(8)
#define IO_IN           PBin(3)
#endif
#ifdef A12_909
#define IO_OUT          PBout(13)
#define IO_IN           PBin(5)
#endif
#ifdef A12_906
#define IO_OUT          PBout(13)
#define IO_IN           PBin(14)
#endif

//----EEPROM存储地址分配---//
//------------------------------------------------------------------------------------------------------------
#define ADDR_BOARD_ID           0
#define LEN_BOARD_ID            2

#define ADDR_MODULE_NUM         (ADDR_BOARD_ID+LEN_BOARD_ID)
#define LEN_MODULE_NUM          1

#define ADDR_VALVE_FIX          (ADDR_MODULE_NUM+LEN_MODULE_NUM)
#define LEN_VALVE_FIX           1

#define ADDR_DIR_FIX            (ADDR_VALVE_FIX+LEN_VALVE_FIX)
#define LEN_DIR_FIX             1

#define ADDR_PORT_CNT           (ADDR_DIR_FIX+LEN_DIR_FIX)
#define LEN_PORT_CNT            1

#define ADDR_BAUD               (ADDR_PORT_CNT+LEN_PORT_CNT)
#define LEN_BAUD                1

#define ADDR_SPD                (ADDR_BAUD+LEN_BAUD)
#define LEN_SPD                 2

#define ADDR_NOW_POS            (ADDR_SPD+LEN_SPD)
#define LEN_NOW_POS             1

#define ADDR_IO_CTRL            (ADDR_NOW_POS+LEN_NOW_POS)
#define LEN_IO_CTRL             1

#define ADDR_INTVL              (ADDR_IO_CTRL+LEN_IO_CTRL)
#define LEN_INTVL               1

#define ADDR_ISET               (ADDR_INTVL+LEN_INTVL)
#define LEN_ISET                1

#define ADDR_SN                 (ADDR_ISET+LEN_ISET)
#define LEN_SN                  5

#define ADDR_RDC_RATE			(ADDR_SN+LEN_SN)
#define LEN_RDC_RATE			1

#define ADDR_HALF_SEAL			(ADDR_RDC_RATE+LEN_RDC_RATE)
#define LEN_HALF_SEAL			1

#define ADDR_TOTAL_CNT        	(ADDR_HALF_SEAL+LEN_HALF_SEAL)
#define LEN_TOTAL_CNT           4

#define NORMAL_BLINK            2500       //正常运行的闪烁间隔
#define RETRY_TIME_OUT          500        //异常运行的闪烁间隔

PEXT uint8 bdrate, bIoCtrl, intCtrl;
PEXT uint16 spdVx2;

typedef struct
{
    uint16  totalCnt;
    uint16  totalCntLst;
    uint16  lastTime;
    uint32  protectTimeOut;
}_SYS_T;
PEXT _SYS_T syspara;



PEXT int main(void);
PEXT void DebugOut(void);
PEXT void dlyInTimer(void);
PEXT unsigned char IntDly(unsigned short intMs);
PEXT void EnableReceive(void);
PEXT void DisableReceive(void);



#undef PEXT
#endif

