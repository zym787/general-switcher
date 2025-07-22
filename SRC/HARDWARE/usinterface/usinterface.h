/*
    作者: zhiyuanTian
    时期: 2018-10-21
    变更:
    1、增加超接收超时清除缓存判断
    2、新增一个分隔符兼容
    时期: 2018-11-22
    变更:
    1、增加接收完成与超时显示字符串
    2、新增1条的VR开关，置1使能
    时期: 2018-11-22
    变更:
    1、增加接收时字符判断，避免首个字符为0的情况发生，
    因首字符为0的话后面的字符全部无效
    时期: 2019-02-13
    变更:
    1、增加接收参数个数判断
    时期: 2019-05-01
    变更:
    1、增加接收个数出错判断，避免越界死机
    时期: 2019-11-28
    变更:
    1、修改命令注册解析模式
*/
#include "stdio.h"
#include "string.h"

#ifndef _USINTERFACE_H_
#define _USINTERFACE_H_

#ifdef _USINTERFACE_GLOBALS_
#define PEXT 
#else
#define PEXT extern
#endif

//参数长度与数量
#define P_CNT               8                       //参数个数
#define P_LEN               16                      //参数单位长度
#define W_LEN               (P_LEN*P_CNT+P_CNT-1)
//接收缓冲区大小:命令+参数分隔符+参数个数*单位参数长度，20+16+8*16
#define USART_RCV_LEN       (164)

typedef struct
{
    unsigned short  TimeOut;
    unsigned short  UsRxSta;
    unsigned char   RxCnt;                      // 接收字符个数
}_CMD_LINE_T;

//----------------------------------------------------------------------------------------------------------------------------------------------------
#define ARRAY_SIZE(x)       (sizeof(x) / (sizeof((x)[0])))

#define MAX_CMD_NAME_LEN    20	        // 最大命令名长度
#define MAX_CMDS_CNT        64	        // 最大命令数

/* 命令结构体类型 */
typedef struct
{
 	char    Name[MAX_CMD_NAME_LEN];             // 命令名
 	char    RealLen;                            // 命令实际长度
    void    (*Operate)(char);                   // 命令操作函数
} _CMD_T;

/* 命令列表结构体类型 */
typedef struct
{
 	_CMD_T  Cmds[MAX_CMDS_CNT];                 // 列表内容
	int     Num;	                            // 列表长度
} _CMDS_T;

//----------------------------------------------------------------------------------------------------------------------------------------------------
//主要外部接口函数
void BootInterface(void);
void getSerialData(char* str, char sdata);
void RegisterCmds(_CMD_T regCmds[], int length);
void UsrCmdAnalyse(char* str);
void TimeOutInt(char* str);
void int2str(int n, char *s);
int str2int(const char *s);
char tolower(char chr);
char toupper(char chr);
int strtohex(char *s);
void* memset(void* s, int c, size_t n);
char* myStrncpy(char *des, char *src, size_t n);
unsigned char FetchChar(unsigned char cmdlen, unsigned char paracnt, char *srcStr, char *para);
unsigned char FetchInt(unsigned char cmdlen, unsigned char paracnt, char *srcStr, int *para);
unsigned char UnEqFetchChar(unsigned char cmdlen, unsigned char paracnt, char *srcStr, char *para);
unsigned char UnEqFetchInt(unsigned char cmdlen, unsigned char paracnt, char *srcStr, int *para);

#undef PEXT
#endif


