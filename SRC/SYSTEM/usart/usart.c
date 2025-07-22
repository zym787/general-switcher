#define _USART_H_GLOBALS_
#include "common.h"


//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 
  


//初始化IO 串口1
//pclk2:PCLK2时钟频率(Mhz)
//bound:波特率
//CHECK OK
void Usart1_Init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				            //得到整数部分
	fraction=(temp-mantissa)*16;            //得到小数部分	 
    mantissa<<=4;
	mantissa+=fraction; 
	
	RCC->APB2ENR |= (RCC_APB2Periph_USART1);            //使能串口1时钟
	RCC->APB2ENR |= (RCC_APB2Periph_GPIOA);             //使能PORTA口时钟  
	GPIOA->CRH &= (GPIO_Crh_P9 & GPIO_Crh_P10);        //IO状态设置 : USART1_TX   PA.9,USART1_RX	  PA.10
	GPIOA->CRH |= (GPIO_Mode_AF_PP_50MHz_P9 | GPIO_Mode_IN_PU_PD_P10);	//IO状态设置:TX---复用推挽输出,RX---下拉输入
		  
	RCC->APB2RSTR |= RCC_APB2Periph_USART1;                   //复位串口1
	RCC->APB2RSTR &= ~RCC_APB2Periph_USART1;                //停止复位	   	   

	//波特率设置
 	USART1->BRR=mantissa;                   //波特率设置	 
	USART1->CR1|=0x200C;                   // 1位停止,无校验位.
#if EN_UART1_RX		                        //如果使能了接收
	//使能接收中断
	USART1->CR1|=1<<8;                      //PE中断使能
	USART1->CR1|=1<<5;                      //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(3,3,USART1_IRQn,2);        //组2，最低优先级 
#endif
}

void DebugSend(unsigned char ch)
{
	while((USART1->SR&0x40)==0);//等待发送结束	
	USART1->DR=ch;
}

void USART1_IRQHandler(void)
{
	volatile u8 res;	
	res = res;
	if(USART1->SR&(1<<5))//接收到数据
	{	 
		res=USART1->DR; 
        getSerialData(rcvStr, res);
	}
}


//----------------------------------------------------------------------------------
//初始化IO 串口2
//pclk2:PCLK2时钟频率(Mhz)
//bound:波特率
//CHECK OK
void Usart2_Init(u32 pclk2,u32 bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	//   Tx / Rx  波特率 ＝  fck/(16*( USARTDIV))
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				    //得到整数部分 ----无符号整数 与 浮点数的运算
	fraction=(temp-mantissa)*16;    //得到小数部分	 浮点数 - 无符号整数 = 浮点数
    mantissa<<=4;                   // USARTDIV的整数部分 ---bit4~bit15
	mantissa+=fraction;             // USARTDIV的小数部分 ---bit0~bit3

	RCC->APB2ENR |= (RCC_APB2Periph_GPIOA);   //使能PORTA口时钟  
	RCC->APB1ENR |= (RCC_APB1Periph_USART2);  //使能串口2时钟
	GPIOA->CRL &= (GPIO_Crl_P2 & GPIO_Crl_P3);		//IO状态设置 : USART2_TX   PA.2,USART1_RX	  PA.3
	GPIOA->CRL |= (GPIO_Mode_AF_PP_50MHz_P2 | GPIO_Mode_IN_PU_PD_P3);	//IO状态设置:TX---复用推挽输出,RX---下拉输入
		  
	RCC->APB1RSTR |= RCC_APB1Periph_USART2;   //复位串口1
	RCC->APB1RSTR &= ~RCC_APB1Periph_USART2;//停止复位	   	   

	//波特率设置
 	USART2->BRR=mantissa;            // 波特率设置	 
	USART2->CR1 |= 0x200C;           //默认:一个起始位，8个数据位,1位停止,无校验位.
    #if EN_UART2_RX 		         //如果使能了接收
	//使能接收中断	
	USART2->CR1 |= 1<<8;              //PE中断使能,接收缓冲区非空中断使能	
	USART2->CR1 |= 1<<5;              //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(1,3,USART2_IRQn,2); //组2(4组抢占(0,1,2,3)，4组优先(0,1,2,3))，最低抢占级，最低优先级
    #endif
}


void Usart2_SendB(unsigned char ch)
{
	while((USART2->SR&0x40)==0);//等待发送结束	
	USART2->DR=ch;
}

//串口2发送字符串
void USART2_SendStr(char *s)
{
    while(*s != '\0') //检测字符串结束符
    {
        Usart2_SendB(*s++);//发送当前字符
    }
}

void USART2_IRQHandler(void)
{
	char res;	  
	res = res;
	if(USART2->SR&(1<<5))//接收到数据
	{	 
		res=USART2->DR;
        ModbusReceive(res);
    }
}


//----------------------------------------------------------------------------------
//初始化IO 串口3
//pclk2:PCLK2时钟频率(Mhz)
//bound:波特率
//CHECK OK
void Usart3_Init(u32 pclk2,u32 bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	//   Tx / Rx  波特率 ＝  fck/(16*( USARTDIV))
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				    //得到整数部分 ----无符号整数 与 浮点数的运算
	fraction=(temp-mantissa)*16;    //得到小数部分	 浮点数 - 无符号整数 = 浮点数
    mantissa<<=4;                   // USARTDIV的整数部分 ---bit4~bit15
	mantissa+=fraction;             // USARTDIV的小数部分 ---bit0~bit3

	RCC->APB2ENR |= (RCC_APB2Periph_GPIOB);     //使能PORTB口时钟  
	RCC->APB1ENR |= (RCC_APB1Periph_USART3);     //使能串口3时钟
	GPIOB->CRH &= (GPIO_Crh_P10 & GPIO_Crh_P11);		//IO状态设置 : USART3_TX   PC.11,USART3_RX	  PC.10
	GPIOB->CRH |= (GPIO_Mode_AF_PP_50MHz_P10 | GPIO_Mode_IN_PU_PD_P11);	//IO状态设置:TX---复用推挽输出,RX---下拉输入
		  
	RCC->APB1RSTR |= RCC_APB1Periph_USART3;     //复位串口1
	RCC->APB1RSTR &= ~RCC_APB1Periph_USART3;    //停止复位	   	   

	//波特率设置
 	USART3->BRR=mantissa;            // 波特率设置	 
	USART3->CR1 |= 0x200C;           //默认:一个起始位，8个数据位,1位停止,无校验位.
    #if EN_UART3_RX 		        //如果使能了接收
	//使能接收中断	
	USART3->CR1|= 1<<8;              //PE中断使能,接收缓冲区非空中断使能	
	USART3->CR1|= 1<<5;              //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(1,3,USART3_IRQn,2); //组2(4组抢占(0,1,2,3)，4组优先(0,1,2,3))，最低抢占级，最低优先级
    #endif
}


void Usart3_SendB(unsigned char ch)
{
	while((USART3->SR&0x40)==0);//等待发送结束	
	USART3->DR=ch;
}


//串口4发送字符串
void USART3_SendStr(char *s)
{
    while(*s != '\0') //检测字符串结束符
    {
        Usart3_SendB(*s++);//发送当前字符
    }
}


void USART3_IRQHandler(void)
{
	volatile u8 res;	
	res = res;
	if(USART3->SR&(1<<5))//接收到数据
	{	 
		res = USART3->DR; 
        ModbusReceive(res);
	}
}

#if STM32_RC_C8
//----------------------------------------------------------------------------------
//初始化IO 串口4
//pclk2:PCLK2时钟频率(Mhz)
//bound:波特率
//CHECK OK
void Uart4_Init(u32 pclk2,u32 bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	//   Tx / Rx  波特率 ＝  fck/(16*( USARTDIV))
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				    //得到整数部分 ----无符号整数 与 浮点数的运算
	fraction=(temp-mantissa)*16;    //得到小数部分	 浮点数 - 无符号整数 = 浮点数
    mantissa<<=4;                   // USARTDIV的整数部分 ---bit4~bit15
	mantissa+=fraction;             // USARTDIV的小数部分 ---bit0~bit3

	RCC->APB2ENR |= (RCC_APB2Periph_GPIOC);   //使能PORTA口时钟  
	RCC->APB1ENR |= (RCC_APB1Periph_UART4);   //使能串口4时钟
	GPIOC->CRH &= (GPIO_Crh_P10 & GPIO_Crh_P11);		//IO状态设置 : USART4_TX   PC.11,USART4_RX	  PC.10
	GPIOC->CRH |= (GPIO_Mode_IN_PU_PD_P11 | GPIO_Mode_AF_PP_50MHz_P10);	//IO状态设置:TX---复用推挽输出,RX---下拉输入
		  
	RCC->APB1RSTR |= RCC_APB1Periph_UART4;   //复位串口1
	RCC->APB1RSTR &= ~RCC_APB1Periph_UART4;//停止复位	   	   

	//波特率设置
 	UART4->BRR=mantissa;            // 波特率设置	 
	UART4->CR1 |= 0x200C;           //默认:一个起始位，8个数据位,1位停止,无校验位.
    #if EN_UART4_RX 		        //如果使能了接收
	//使能接收中断	
	UART4->CR1|= 1<<8;              //PE中断使能,接收缓冲区非空中断使能	
	UART4->CR1|= 1<<5;              //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(3,3,UART4_IRQn,2); //组2(4组抢占(0,1,2,3)，4组优先(0,1,2,3))，最低抢占级，最低优先级
    #endif
}


void Uart4_SendB(unsigned char ch)
{
	while((UART4->SR&0x40)==0);//等待发送结束	
	UART4->DR=ch;
}


//串口4发送字符串
void UART4_SendStr(char *s)
{
    while(*s != '\0') //检测字符串结束符
    {
        Uart4_SendB(*s++);//发送当前字符
    }
}


void UART4_IRQHandler(void)
{
	volatile u8 res;	
	res = res;
	if(UART4->SR&(1<<5))//接收到数据
	{	 
		res=UART4->DR; 
	}
}

#endif



