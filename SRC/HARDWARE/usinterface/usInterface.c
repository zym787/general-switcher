#define _USINTERFACE_GLOBALS_

#include "usfunc.h"
#include "usinterface.h"

//----------使用方法，把相关的几个接口函数放置于相应的位置，使用printf进行调试输出即可----------//
//----------串口调试助手必须设置为：以回车发送，即发送的字符以0x0d 0x0a结束，建议使用HTerm------//

_STR_T str;
/*
    接口函数
    串口数据接收程序
    串口接收中断内调用
*/
void getSerialData(char sdata)
{
    #if CR_LF
	if((str.UsRxSta&STA_LF)==0)         //接收未完成
	{
		if(str.UsRxSta&STA_CR)          //接收到了0x0d
		{
			if(sdata!=ASCII_LF)
                str.UsRxSta |= STA_ERR; //接收出错
			else 
                str.UsRxSta |= STA_LF;  //接收完成了 
		}
		else if(sdata)
		{	
			if(sdata==ASCII_CR)
			{
                if(strlen(str.rcvStr))
                {//只有接收到有效命令后才能接收结束符
                    str.UsRxSta |= STA_CR;
                }
			}
            else if(sdata!=ASCII_CR && sdata!=ASCII_LF)
			{
                if(str.pRcvStr<&str.rcvStr[W_LEN-1])
                {//避免越界
                    *str.pRcvStr = sdata;
                    ++str.pRcvStr;
                }
                else
                    str.UsRxSta |= STA_ERR; //接收出错
			}		 
		}
	} 
    #else
	if((str.UsRxSta&STA_CR)==0)         //接收未完成
	{
		if(sdata==ASCII_CR)
		{
            if(strlen(str.rcvStr))
            {//只有接收到有效命令后才能接收结束符
                str.UsRxSta |= STA_CR;
            }
		}
		else if(sdata)
		{
            if(str.pRcvStr<&str.rcvStr[W_LEN-1])
            {//避免越界
                *str.pRcvStr = sdata;
                ++str.pRcvStr;
            }
            else
                str.UsRxSta |= STA_ERR; //接收出错
		}		 
	}
    #endif
    
}


/*
    接口函数
    用于主函数调用
    
*/
void StrProc(void)
{    
    #if CR_LF
    if(str.UsRxSta&STA_LF||str.UsRxSta&STA_ERR)
    #else
    if(str.UsRxSta&STA_CR||str.UsRxSta&STA_ERR)
    #endif
    {
        if(str.UsRxSta&STA_CR)
        {
            if(strlen(str.rcvStr))
            {
                str.ChRunFunc(str.rcvStr);
            }
            if(str.fUsIRF)
                printd("\r RCV:\"%s\"", str.rcvStr);
        }
        else
        {
            printd("\r Receive Err");
        }
        //清空str接收到的字符，并重定向指针
        memset(str.rcvStr, '\0', sizeof(str.rcvStr));
        str.pRcvStr = str.rcvStr;
        str.UsRxSta = 0;
    }
    TimeOutInt();
}


void TimeOutInt(void)
{
    #if CR_LF
    if(strlen(str.rcvStr) && !(str.UsRxSta&STA_LF))
    #else
    if(strlen(str.rcvStr) && !(str.UsRxSta&STA_CR))
    #endif
    {
        if(++str.TimeOut>RCV_TIME_OUT)
        {
            str.TimeOut = 0;
            if(str.fUsIRF)
                printd("\r TimeOut:\"%s\"", str.rcvStr);
            memset(str.rcvStr, '\0', sizeof(str.rcvStr));
            str.pRcvStr = str.rcvStr;
            str.UsRxSta = 0;
        }
    }
    else
    {
        str.TimeOut = 0;
    }
}

/*
    启动界面
*/
void BootInterface(void)
{//指向指针
    str.pRcvStr = str.rcvStr;
    str.ChRunFunc = ChRUN;
    printd("\r %s", BOOT_STR);
}

//-------------------------字符与内存处理函数-------------------------//

/*
    数字转为字符串
*/
void int2str(int n, char *s)
{
    char buf[10]="";
    int i=0, len=0;
    int temp=(n<0)?(-n):(n);      // temp为n的绝对值
    if(s==NULL)
        return;
    
    while(temp)
    {
        buf[i++] = (temp%10)+'0';   //把temp的每一位上的数存入buf
        temp = temp/10;
    }

    len = (n<0)?(++i):(i);              //如果n是负数，则多需要一位来存储负号
    s[i] = 0;                           //末尾是结束符0
    while(1)
    {
        i--;
        if(buf[len-i-1]==0)
            break;
        
        s[i] = buf[len-i-1];            //把buf数组里的字符拷到字符串
    }
    if(i==0)
        s[i] = '-';                     //如果是负数，添加一个负号
}


/*
    字符串转为数字
    输入必须为数字字符
*/
int str2int(const char *s)
{
    int temp=0;
    const char *ptr=s;                    //ptr保存str字符串开头
    
    if (*s=='-' || *s=='+')             //如果第一个字符是正负号则移到下一个字符
        s++;
    
    while(*s!=0)
    {
        if((*s<'0') || (*s>'9'))       //如果当前字符不是数字
            break;
        
        temp = temp*10+(*s-'0');      //如果当前字符是数字则计算数值
        s++;                                //移到下一个字符
    }   
    if (*ptr=='-')                        //如果字符串是以“-”开头，则转换成其相反数
        temp = -temp;
    
    return temp;
}


/*
    功能:清除指定数组内的值
    para: 数组指针
    para: 填充值
    para: 数组大小，使用sizeof()    
*/
void* memset(void* s, int c, size_t n)
{
    unsigned char* p = (unsigned char*)s;
    while (n > 0)
    {
        *p++ = (unsigned char)c;
        --n;
    }
    return s;
}

char tolower(char chr)
{	
    if ((chr >= 'A') && (chr <= 'Z'))		
        return chr + ('a' - 'A');	
    
    return chr;
} 

char toupper(char chr)
{	
    if ((chr >= 'a') && (chr <= 'z'))		
        return chr + ('A' - 'a');	
    
    return chr;
}


int strtohex(char *s)
{
    int intPara=0;
    while(*s!='\0')
    {
        *s = toupper(*s);
        intPara <<= 4;
        if(*s>='A' && *s<='F')
            intPara |= (10 + (*s-'A'));
        else
            intPara |= (*s-'0');
        
        s++;
    }
    return intPara;
}


char* myStrncpy(char *des, char *src, size_t n)
{
    strncpy(des, src, n-1);
    *(des+n-1) = '\0';
    return des;
}


/*
    @cmdlen     命令长度
    @paracnt    命令个数   值为0时不对个数进行检查
    @*para      参数
    @返回值     返回错误代码
    1: 命令与参数间隔符异常输出
    2: 参数长度与定义不符
    3: 参数个数不对
*/
unsigned char FetchInt(unsigned char cmdlen, unsigned char paracnt, char *srcStr, int *para)
{
    char chSep, *p, cpyBuf[W_LEN+1], rngBuf[P_LEN+1];
    unsigned short cnt=0, bfCnt=0, strCnt=0;
    strncpy(&chSep, (srcStr+cmdlen), 1);                      //取命令与参数的分隔符
    if(chSep==C_EQUAL)
    {
        strncpy(cpyBuf, (srcStr+cmdlen+1), W_LEN);            //取参数字符串
        *(cpyBuf+W_LEN) = C_TERM;                           //手动添加结束符
        p = cpyBuf;
        for(cnt=0; cnt<=W_LEN; cnt++)
        {//截取参数
            if(*p==C_SEP_1 || *p==C_SEP_2)
            {//判断分隔符
                if((cnt-bfCnt) && (cnt-bfCnt)<=P_LEN)         // paralen个字符
                {//判定参数长度
                    strncpy(rngBuf, cpyBuf+bfCnt, cnt-bfCnt);  //取参数字符串
                    rngBuf[cnt-bfCnt] = C_TERM;
                    bfCnt = cnt+1;
                    if(paracnt)
                    {
                        if(strCnt<paracnt)
                            *(para+strCnt) = str2int(rngBuf);
                        else
                            return 3;
                    }
                    else
                    {
                        *(para+strCnt) = str2int(rngBuf);
                    }
                    ++strCnt;       //第几个参数
                }
                else//参数长度与定义不符
                    return 2;
            }
            if(*p++==C_TERM)
            {//判断结束符
             //命令参数需要输入完全，不然报错，注释掉可以只输入前面几个
                if((cnt-bfCnt) && (cnt-bfCnt)<=P_LEN)    // paralen个字符
                {//判定参数长度,参数输出
                    strncpy(rngBuf, cpyBuf+bfCnt, cnt-bfCnt);
                    rngBuf[cnt-bfCnt] = C_TERM;
                    if(paracnt)
                    {
                        if(strCnt<paracnt)
                        {
                            *(para+strCnt) = str2int(rngBuf);
                        }
        				++strCnt;       //第几个参数
        				if(strCnt!=paracnt)
        				{//参数个数与定义不符
                            return 3;
        				}
                    }
                    else
                    {
                        *(para+strCnt) = str2int(rngBuf);
                    }
                    break;
                }
                else//参数长度与定义不符
                    return 2;
            }
        }
    }
    else    //命令与参数间隔符异常输出
    {
        return 1;
    }
    return 0;
}


/*
    @cmdlen     命令长度
    @paracnt    命令个数   值为0时不对个数进行检查
    @*para      输出数组
    @返回值     返回错误代码
    1: 命令与参数间隔符异常输出
    2: 参数长度与定义不符
    3: 参数个数不对
*/
unsigned char FetchChar(unsigned char cmdlen, unsigned char paracnt, char *srcStr, char *para)
{//strncpy函数进行字符串截取的时候，不会在最后加入结束符，需要手动添加，所以声明存储的时候需要多一个字节
    char chSep, *p, cpyBuf[W_LEN+1], rngBuf[P_LEN+1];
    unsigned short cnt=0, bfCnt=0, strCnt=0;
    strncpy(&chSep, (srcStr+cmdlen), 1);                //取命令与参数的分隔符
    if(chSep==C_EQUAL)
    {
        strncpy(cpyBuf, (srcStr+cmdlen+1), W_LEN);        //取参数字符串
        *(cpyBuf+W_LEN) = C_TERM;                           //手动添加结束符
        p = cpyBuf;
        for(cnt=0; cnt<=W_LEN; cnt++)
        {//截取参数
            if(*p==C_SEP_1 || *p==C_SEP_2)
            {//判断分隔符
                if((cnt-bfCnt) && (cnt-bfCnt)<=P_LEN)         // paralen个字符
                {//判定参数长度
                    strncpy(rngBuf, cpyBuf+bfCnt, cnt-bfCnt);   //取参数字符串
                    rngBuf[cnt-bfCnt] = C_TERM;
                    bfCnt = cnt+1;
                    if(paracnt)
                    {
                        if(strCnt<paracnt)
                            strcpy((para+strCnt*(P_LEN+1)), rngBuf);
                        else
                            return 3;
                    }
                    else
                    {
                        strcpy((para+strCnt*(P_LEN+1)), rngBuf);
                    }
                    ++strCnt;                                   //第几个参数
                }
                else    //参数长度与定义不符
                    return 2;
            }
            if(*p++==C_TERM)
            {//判断结束符
            //命令参数需要输入完全，不然报错，注释掉可以只输入前面几个
                if((cnt-bfCnt) && (cnt-bfCnt)<=P_LEN)      // paralen-1个字符
                {//判定参数长度,参数输出
                    strncpy(rngBuf, cpyBuf+bfCnt, cnt-bfCnt);
                    rngBuf[cnt-bfCnt] = C_TERM;
                    if(paracnt)
                    {
                        if(strCnt<paracnt)
                        {
                            strcpy((para+strCnt*(P_LEN+1)), rngBuf);
                        }
        				++strCnt;       //第几个参数
        				if(strCnt!=paracnt)
        				{//参数个数与定义不符
                            return 3;
        				}
                    }
                    else
                    {
                        strcpy((para+strCnt*(P_LEN+1)), rngBuf);
                    }
                    break;
                }
                else    //参数长度与定义不符
                    return 2;
            }
        }
    }
    else    //命令与参数间隔符异常输出
    {
        return 1;
    }
    return 0;
}

/*
    @cmdlen     命令长度
    @paracnt    命令个数   值为0时不对个数进行检查
    @*para      参数
    @返回值     返回错误代码
    1: 命令与参数间隔符异常输出
    2: 参数长度与定义不符
    3: 参数个数不对
*/
unsigned char UnEqFetchInt(unsigned char cmdlen, unsigned char paracnt, char *srcStr, int *para)
{
    char *p, cpyBuf[W_LEN+1], rngBuf[P_LEN+1];
    unsigned short cnt=0, bfCnt=0, strCnt=0;
	strncpy(cpyBuf, (srcStr+cmdlen), W_LEN);        //取参数字符串
	*(cpyBuf+W_LEN) = C_TERM;                           //手动添加结束符
	p = cpyBuf;
	for(cnt=0; cnt<=W_LEN; cnt++)
	{//截取参数
        if(*p==C_SEP_1 || *p==C_SEP_2)
		{//判断分隔符
			if((cnt-bfCnt) && (cnt-bfCnt)<=P_LEN)         // paralen个字符
			{//判定参数长度
				strncpy(rngBuf, cpyBuf+bfCnt, cnt-bfCnt);   //取参数字符串
				rngBuf[cnt-bfCnt] = C_TERM;
				bfCnt = cnt+1;
                if(paracnt)
                {
                    if(strCnt<paracnt)
                        *(para+strCnt) = str2int(rngBuf);
                    else
                        return 3;
                }
                else
                {
                    *(para+strCnt) = str2int(rngBuf);
                }
				++strCnt;       //第几个参数
			}
			else//参数长度与定义不符
				return 2;
		}
		if(*p++==C_TERM)
		{//判断结束符
		//命令参数需要输入完全，不然报错，注释掉可以只输入前面几个
			if((cnt-bfCnt) && (cnt-bfCnt)<=P_LEN)    // paralen个字符
			{//判定参数长度,参数输出
				strncpy(rngBuf, cpyBuf+bfCnt, cnt-bfCnt);
				rngBuf[cnt-bfCnt] = C_TERM;
                if(paracnt)
                {
                    if(strCnt<paracnt)
                    {
                        *(para+strCnt) = str2int(rngBuf);
                    }
    				++strCnt;       //第几个参数
    				if(strCnt!=paracnt)
    				{//参数个数与定义不符
                        return 3;
    				}
                }
                else
                {
                    *(para+strCnt) = str2int(rngBuf);
                }
				break;
			}
			else//参数长度与定义不符
				return 2;
		}
	}
    return 0;
}


/*
    @cmdlen     命令长度
    @paracnt    命令个数   值为0时不对个数进行检查
    @*para      输出数组
    @返回值     返回错误代码
    1: 命令与参数间隔符异常输出
    2: 参数长度与定义不符
    3: 参数个数不对
*/
unsigned char UnEqFetchChar(unsigned char cmdlen, unsigned char paracnt, char *srcStr, char *para)
{//strncpy函数进行字符串截取的时候，不会在最后加入结束符，需要手动添加，所以声明存储的时候需要多一个字节
    char *p, cpyBuf[W_LEN+1], rngBuf[P_LEN+1];
    unsigned short cnt=0, bfCnt=0, strCnt=0;
	strncpy(cpyBuf, (srcStr+cmdlen), W_LEN);              //取参数字符串
	*(cpyBuf+W_LEN) = C_TERM;                           //手动添加结束符
	p = cpyBuf;
	for(cnt=0; cnt<=W_LEN; cnt++)
	{//截取参数
        if(*p==C_SEP_1 || *p==C_SEP_2)
		{//判断分隔符
			if((cnt-bfCnt) && (cnt-bfCnt)<=P_LEN)           // paralen个字符
			{//判定参数长度
				strncpy(rngBuf, cpyBuf+bfCnt, cnt-bfCnt);   //取参数字符串
				rngBuf[cnt-bfCnt] = C_TERM;
				strcpy((para+strCnt*(P_LEN+1)), rngBuf);
				bfCnt = cnt+1;
                if(paracnt)
                {
                    if(strCnt<paracnt)
                        strcpy((para+strCnt*(P_LEN+1)), rngBuf);
                    else
                        return 3;
                }
                else
                {
                    strcpy((para+strCnt*(P_LEN+1)), rngBuf);
                }
				++strCnt;                                   //第几个参数
			}
			else    //参数长度与定义不符
				return 2;
		}
		if(*p++==C_TERM)
		{//判断结束符
    		//命令参数需要输入完全，不然报错，注释掉可以只输入前面几个
			if((cnt-bfCnt) && (cnt-bfCnt)<=P_LEN)           // paralen-1个字符
			{//判定参数长度,参数输出
				strncpy(rngBuf, cpyBuf+bfCnt, cnt-bfCnt);
				rngBuf[cnt-bfCnt] = C_TERM;
                if(paracnt)
                {
                    if(strCnt<paracnt)
                    {
                        strcpy((para+strCnt*(P_LEN+1)), rngBuf);
                    }
    				++strCnt;       //第几个参数
    				if(strCnt!=paracnt)
    				{//参数个数与定义不符
                        return 3;
    				}
                }
                else
                {
                    strcpy((para+strCnt*(P_LEN+1)), rngBuf);
                }
				break;
			}
			else    //参数长度与定义不符
				return 2;
		}
	}
    return 0;
}



