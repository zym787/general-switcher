#ifndef _USFUNC_H_
#define _USFUNC_H_

#ifdef _USFUNC_GLOBALS_
#define PEXT 
#else
#define PEXT extern
#endif

//用于批量注释掉调试输出语句
//"..."代表一个可以变化的参数表。使用保留名"__VA_ARGS__"把参数传递给宏。
//当宏的调用展开时,实际的参数就传递给 printf()了。
#define FSW_DBG_PRINT       1           //输出信息开关
#define FSW_WORK_PRINT      1           //输出信息开关

#if FSW_DBG_PRINT
#define LV_NA                   0
#define LV_INFO                 1
#define LV_DBG                  2
#define	printd(fmt, ...)		printf(fmt, ##__VA_ARGS__)
#define prInfo(lv,fmt, ...)     do{if(lv>=LV_INFO) printf(fmt, ##__VA_ARGS__);}while(0)
#define prDbg(lv,fmt, ...)      do{if(lv>=LV_DBG) printf(fmt, ##__VA_ARGS__);}while(0)
#else
#define printd(...)
#endif

#if FSW_WORK_PRINT
#define	printw(fmt, ... )		printf(fmt, ##__VA_ARGS__)
#else
#define printw(...)
#endif

                   
#define S_LIST_SH   "--------------------Sub command Line-------------------"
#define S_LIST_SE   "-------------------------------------------------------"

#define READ_ACT            0
#define WRITE_ACT           1

#define FLASH_CHK           1                       //IIC-1 flash-0

PEXT char rcvStr[USART_RCV_LEN];
PEXT void TermList(char rw);
PEXT void TermVR(char rw);
PEXT void TermMap(char rw);
PEXT void TermIIC(char rw);
PEXT void TermReset(char rw);
PEXT void TermMotorX(char rw);
PEXT void UsrCmdInit(void);

#undef PEXT
#endif


