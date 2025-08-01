#ifndef _USART_H_
#define _USART_H_

#ifdef _USART_H_GLOBALS_
#define PEXT
#else
#define PEXT extern
#endif


#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_UART1_RX 			1		//使能（1）/禁止（0）串口1接收
#define EN_UART2_RX 			1		//使能（1）/禁止（0）串口2接收
#define EN_UART3_RX 			1		//使能（1）/禁止（0）串口3接收
#define EN_UART4_RX 			1		//使能（1）/禁止（0）串口4接收
	  	
PEXT unsigned char  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
PEXT unsigned short USART_RX_STA;         		//接收状态标记	


//如果想串口中断接收,请不要注释以下宏定义
PEXT void Usart1_Init(u32 pclk2,u32 bound);
PEXT void USART1_IRQHandler(void);
PEXT void Usart2_Init(u32 pclk2,u32 bound);
PEXT void USART2_IRQHandler(void);
PEXT void Usart2_SendB(unsigned char ch);
PEXT void USART2_SendStr(char *s);
PEXT void Usart3_Init(u32 pclk2,u32 bound);
PEXT void USART3_IRQHandler(void);
PEXT void Usart3_SendB(unsigned char ch);
PEXT void USART3_SendStr(char *s);
#if STM32RC_C8
PEXT void Uart4_Init(u32 pclk2,u32 bound);
PEXT void UART4_IRQHandler(void);
PEXT void Uart4_SendB(unsigned char ch);
PEXT void UART4_SendStr(char *s);
#endif


#undef PEXT
#endif















