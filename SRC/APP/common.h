#ifndef _COMMON_H_
#define _COMMON_H_

typedef unsigned char  uint8;                    /* Unsigned  8 bit quantity                           */
typedef unsigned short uint16;                   /* Unsigned 16 bit quantity                           */
typedef unsigned int   uint32;                   /* Unsigned 32 bit quantity                           */

#define STM32_RC_C8             0       // 1-使用RC 0使用C8
#define IOCTRL                          // 使用IO控制
#define IO_RS             1   // IO_IN 和 IO_OUT是否取反
                              // 1 A (232/485/IO)   IO_IN高 IO_OUT低 (取反)
                              // 0 B (IO)           IO_IN高 IO_OUT高 (同电平)

//#define A12_901
//#define A12_906
#define A12_909     // 909需要注意输出反向

#include <stm32f10x.h>
#include "usinterface.h"
#include "stmflash.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "usFunc.h"
#include "smotor.h"
#include "EEP24serial.h"
#include "main.h"
#include "modbus.h"
#include "motor.h"


//STM32F10X_HD,USE_STDPERIPH_DRIVER

#define OFF 0
#define ON  1

#define FALSE 0
#define TRUE  1

//-------------------------rcc----------------------------------
#define RCC_APB1Periph_TIM2              ((vu32)0x00000001)
#define RCC_APB1Periph_TIM3              ((vu32)0x00000002)
#define RCC_APB1Periph_TIM4              ((vu32)0x00000004)
#define RCC_APB1Periph_TIM5              ((vu32)0x00000008)
#define RCC_APB1Periph_TIM6              ((vu32)0x00000010)
#define RCC_APB1Periph_TIM7              ((vu32)0x00000020)
#define RCC_APB1Periph_TIM12             ((vu32)0x00000040)
#define RCC_APB1Periph_TIM13             ((vu32)0x00000080)
#define RCC_APB1Periph_TIM14             ((vu32)0x00000100)
#define RCC_APB1Periph_WWDG              ((vu32)0x00000800)
#define RCC_APB1Periph_SPI2              ((vu32)0x00004000)
#define RCC_APB1Periph_SPI3              ((vu32)0x00008000)
#define RCC_APB1Periph_USART2            ((vu32)0x00020000)
#define RCC_APB1Periph_USART3            ((vu32)0x00040000)
#define RCC_APB1Periph_UART4             ((vu32)0x00080000)
#define RCC_APB1Periph_UART5             ((vu32)0x00100000)
#define RCC_APB1Periph_I2C1              ((vu32)0x00200000)
#define RCC_APB1Periph_I2C2              ((vu32)0x00400000)
#define RCC_APB1Periph_USB               ((vu32)0x00800000)
#define RCC_APB1Periph_CAN1              ((vu32)0x02000000)
#define RCC_APB1Periph_CAN2              ((vu32)0x04000000)
#define RCC_APB1Periph_BKP               ((vu32)0x08000000)
#define RCC_APB1Periph_PWR               ((vu32)0x10000000)
#define RCC_APB1Periph_DAC               ((vu32)0x20000000)
#define RCC_APB1Periph_CEC               ((vu32)0x40000000)

#define RCC_APB2Periph_AFIO              ((vu32)0x00000001)
#define RCC_APB2Periph_GPIOA             ((vu32)0x00000004)
#define RCC_APB2Periph_GPIOB             ((vu32)0x00000008)
#define RCC_APB2Periph_GPIOC             ((vu32)0x00000010)
#define RCC_APB2Periph_GPIOD             ((vu32)0x00000020)
#define RCC_APB2Periph_GPIOE             ((vu32)0x00000040)
#define RCC_APB2Periph_GPIOF             ((vu32)0x00000080)
#define RCC_APB2Periph_GPIOG             ((vu32)0x00000100)
#define RCC_APB2Periph_ADC1              ((vu32)0x00000200)
#define RCC_APB2Periph_ADC2              ((vu32)0x00000400)
#define RCC_APB2Periph_TIM1              ((vu32)0x00000800)
#define RCC_APB2Periph_SPI1              ((vu32)0x00001000)
#define RCC_APB2Periph_TIM8              ((vu32)0x00002000)
#define RCC_APB2Periph_USART1            ((vu32)0x00004000)
#define RCC_APB2Periph_ADC3              ((vu32)0x00008000)
#define RCC_APB2Periph_TIM15             ((vu32)0x00010000)
#define RCC_APB2Periph_TIM16             ((vu32)0x00020000)
#define RCC_APB2Periph_TIM17             ((vu32)0x00040000)
#define RCC_APB2Periph_TIM9              ((vu32)0x00080000)
#define RCC_APB2Periph_TIM10             ((vu32)0x00100000)
#define RCC_APB2Periph_TIM11             ((vu32)0x00200000)

//----GPIO----
#define GPIO_Pin_0                      ((vu16)0x0001) 		//???????-?蜿?3???
#define GPIO_Pin_1                      ((vu16)0x0002)
#define GPIO_Pin_2                      ((vu16)0x0004)
#define GPIO_Pin_3                      ((vu16)0x0008)
#define GPIO_Pin_4                      ((vu16)0x0010)
#define GPIO_Pin_5                      ((vu16)0x0020)
#define GPIO_Pin_6                      ((vu16)0x0040)
#define GPIO_Pin_7                      ((vu16)0x0080)
#define GPIO_Pin_8                      ((vu16)0x0100)
#define GPIO_Pin_9                      ((vu16)0x0200)
#define GPIO_Pin_10                     ((vu16)0x0400)
#define GPIO_Pin_11                     ((vu16)0x0800)
#define GPIO_Pin_12                     ((vu16)0x1000)
#define GPIO_Pin_13                     ((vu16)0x2000)
#define GPIO_Pin_14                     ((vu16)0x4000)
#define GPIO_Pin_15                     ((vu16)0x8000)
#define GPIO_Pin_All                    ((vu16)0xFFFF)

#define GPIO_Crl_P0                     ((vu32)0XFFFFFFF0)
#define GPIO_Crl_P1                     ((vu32)0XFFFFFF0f)
#define GPIO_Crl_P2                     ((vu32)0XFFFFF0ff)
#define GPIO_Crl_P3                     ((vu32)0XFFFF0fff)
#define GPIO_Crl_P4                     ((vu32)0XFFF0ffff)
#define GPIO_Crl_P5                     ((vu32)0XFF0fffff)
#define GPIO_Crl_P6                     ((vu32)0XF0ffffff)
#define GPIO_Crl_P7                     ((vu32)0X0fffffff)
#define GPIO_Crh_P8                     GPIO_Crl_P0
#define GPIO_Crh_P9                     GPIO_Crl_P1
#define GPIO_Crh_P10                    GPIO_Crl_P2
#define GPIO_Crh_P11                    GPIO_Crl_P3
#define GPIO_Crh_P12                    GPIO_Crl_P4
#define GPIO_Crh_P13                    GPIO_Crl_P5
#define GPIO_Crh_P14                    GPIO_Crl_P6
#define GPIO_Crh_P15                    GPIO_Crl_P7

//???a????????
#define	GPIO_Mode_IN_AIN						((vu32)0x00000000)
//????????????---?'??o蟮??'??
#define	GPIO_Mode_IN_FLOATING_P0				((vu32)0x00000004)
#define	GPIO_Mode_IN_FLOATING_P1				((vu32)0x00000040)
#define	GPIO_Mode_IN_FLOATING_P2				((vu32)0x00000400)
#define	GPIO_Mode_IN_FLOATING_P3				((vu32)0x00004000)
#define	GPIO_Mode_IN_FLOATING_P4				((vu32)0x00040000)
#define	GPIO_Mode_IN_FLOATING_P5				((vu32)0x00400000)
#define	GPIO_Mode_IN_FLOATING_P6				((vu32)0x04000000)
#define	GPIO_Mode_IN_FLOATING_P7				((vu32)0x40000000)
#define	GPIO_Mode_IN_FLOATING_P8				((vu32)0x00000004)
#define	GPIO_Mode_IN_FLOATING_P9				((vu32)0x00000040)
#define	GPIO_Mode_IN_FLOATING_P10				((vu32)0x00000400)
#define	GPIO_Mode_IN_FLOATING_P11				((vu32)0x00004000)
#define	GPIO_Mode_IN_FLOATING_P12				((vu32)0x00040000)
#define	GPIO_Mode_IN_FLOATING_P13				((vu32)0x00400000)
#define	GPIO_Mode_IN_FLOATING_P14				((vu32)0x04000000)
#define	GPIO_Mode_IN_FLOATING_P15				((vu32)0x40000000)
//???-/ ???-????????
#define	GPIO_Mode_IN_PU_PD_P0					((vu32)0x00000008)
#define	GPIO_Mode_IN_PU_PD_P1					((vu32)0x00000080)
#define	GPIO_Mode_IN_PU_PD_P2					((vu32)0x00000800)
#define	GPIO_Mode_IN_PU_PD_P3					((vu32)0x00008000)
#define	GPIO_Mode_IN_PU_PD_P4					((vu32)0x00080000)
#define	GPIO_Mode_IN_PU_PD_P5					((vu32)0x00800000)
#define	GPIO_Mode_IN_PU_PD_P6					((vu32)0x08000000)
#define	GPIO_Mode_IN_PU_PD_P7					((vu32)0x80000000)
#define	GPIO_Mode_IN_PU_PD_P8					((vu32)0x00000008)
#define	GPIO_Mode_IN_PU_PD_P9					((vu32)0x00000080)
#define	GPIO_Mode_IN_PU_PD_P10					((vu32)0x00000800)
#define	GPIO_Mode_IN_PU_PD_P11					((vu32)0x00008000)
#define	GPIO_Mode_IN_PU_PD_P12					((vu32)0x00080000)
#define	GPIO_Mode_IN_PU_PD_P13					((vu32)0x00800000)
#define	GPIO_Mode_IN_PU_PD_P14					((vu32)0x08000000)
#define	GPIO_Mode_IN_PU_PD_P15					((vu32)0x80000000)
//悫????盱??3?????
#define	GPIO_Mode_Out_PP_10MHz_P0				((vu32)0x00000001)
#define	GPIO_Mode_Out_PP_10MHz_P1				((vu32)0x00000010)
#define	GPIO_Mode_Out_PP_10MHz_P2				((vu32)0x00000100)
#define	GPIO_Mode_Out_PP_10MHz_P3				((vu32)0x00001000)
#define	GPIO_Mode_Out_PP_10MHz_P4				((vu32)0x00010000)
#define	GPIO_Mode_Out_PP_10MHz_P5				((vu32)0x00100000)
#define	GPIO_Mode_Out_PP_10MHz_P6				((vu32)0x01000000)
#define	GPIO_Mode_Out_PP_10MHz_P7				((vu32)0x10000000)
#define	GPIO_Mode_Out_PP_10MHz_P8				((vu32)0x00000001)
#define	GPIO_Mode_Out_PP_10MHz_P9				((vu32)0x00000010)
#define	GPIO_Mode_Out_PP_10MHz_P10				((vu32)0x00000100)
#define	GPIO_Mode_Out_PP_10MHz_P11				((vu32)0x00001000)
#define	GPIO_Mode_Out_PP_10MHz_P12				((vu32)0x00010000)
#define	GPIO_Mode_Out_PP_10MHz_P13				((vu32)0x00100000)
#define	GPIO_Mode_Out_PP_10MHz_P14				((vu32)0x01000000)
#define	GPIO_Mode_Out_PP_10MHz_P15				((vu32)0x10000000)

#define	GPIO_Mode_Out_PP_2MHz_P0				((vu32)0x00000002)
#define	GPIO_Mode_Out_PP_2MHz_P1				((vu32)0x00000020)
#define	GPIO_Mode_Out_PP_2MHz_P2				((vu32)0x00000200)
#define	GPIO_Mode_Out_PP_2MHz_P3				((vu32)0x00002000)
#define	GPIO_Mode_Out_PP_2MHz_P4				((vu32)0x00020000)
#define	GPIO_Mode_Out_PP_2MHz_P5				((vu32)0x00200000)
#define	GPIO_Mode_Out_PP_2MHz_P6				((vu32)0x02000000)
#define	GPIO_Mode_Out_PP_2MHz_P7				((vu32)0x20000000)
#define	GPIO_Mode_Out_PP_2MHz_P8				((vu32)0x00000002)
#define	GPIO_Mode_Out_PP_2MHz_P9				((vu32)0x00000020)
#define	GPIO_Mode_Out_PP_2MHz_P10				((vu32)0x00000200)
#define	GPIO_Mode_Out_PP_2MHz_P11				((vu32)0x00002000)
#define	GPIO_Mode_Out_PP_2MHz_P12				((vu32)0x00020000)
#define	GPIO_Mode_Out_PP_2MHz_P13				((vu32)0x00200000)
#define	GPIO_Mode_Out_PP_2MHz_P14				((vu32)0x02000000)
#define	GPIO_Mode_Out_PP_2MHz_P15				((vu32)0x20000000)

#define	GPIO_Mode_Out_PP_50MHz_P0				((vu32)0x00000003)
#define	GPIO_Mode_Out_PP_50MHz_P1				((vu32)0x00000030)
#define	GPIO_Mode_Out_PP_50MHz_P2				((vu32)0x00000300)
#define	GPIO_Mode_Out_PP_50MHz_P3				((vu32)0x00003000)
#define	GPIO_Mode_Out_PP_50MHz_P4				((vu32)0x00030000)
#define	GPIO_Mode_Out_PP_50MHz_P5				((vu32)0x00300000)
#define	GPIO_Mode_Out_PP_50MHz_P6				((vu32)0x03000000)
#define	GPIO_Mode_Out_PP_50MHz_P7				((vu32)0x30000000)
#define	GPIO_Mode_Out_PP_50MHz_P8				((vu32)0x00000003)
#define	GPIO_Mode_Out_PP_50MHz_P9				((vu32)0x00000030)
#define	GPIO_Mode_Out_PP_50MHz_P10				((vu32)0x00000300)
#define	GPIO_Mode_Out_PP_50MHz_P11				((vu32)0x00003000)
#define	GPIO_Mode_Out_PP_50MHz_P12				((vu32)0x00030000)
#define	GPIO_Mode_Out_PP_50MHz_P13				((vu32)0x00300000)
#define	GPIO_Mode_Out_PP_50MHz_P14				((vu32)0x03000000)
#define	GPIO_Mode_Out_PP_50MHz_P15				((vu32)0x30000000)

//?'??1|????盱??3?????
#define	GPIO_Mode_AF_PP_10MHz_P0				((vu32)0x00000009)
#define	GPIO_Mode_AF_PP_10MHz_P1				((vu32)0x00000090)
#define	GPIO_Mode_AF_PP_10MHz_P2				((vu32)0x00000900)
#define	GPIO_Mode_AF_PP_10MHz_P3				((vu32)0x00009000)
#define	GPIO_Mode_AF_PP_10MHz_P4				((vu32)0x00090000)
#define	GPIO_Mode_AF_PP_10MHz_P5				((vu32)0x00900000)
#define	GPIO_Mode_AF_PP_10MHz_P6				((vu32)0x09000000)
#define	GPIO_Mode_AF_PP_10MHz_P7				((vu32)0x90000000)
#define	GPIO_Mode_AF_PP_10MHz_P8				((vu32)0x00000009)
#define	GPIO_Mode_AF_PP_10MHz_P9				((vu32)0x00000090)
#define	GPIO_Mode_AF_PP_10MHz_P10				((vu32)0x00000900)
#define	GPIO_Mode_AF_PP_10MHz_P11				((vu32)0x00009000)
#define	GPIO_Mode_AF_PP_10MHz_P12				((vu32)0x00090000)
#define	GPIO_Mode_AF_PP_10MHz_P13				((vu32)0x00900000)
#define	GPIO_Mode_AF_PP_10MHz_P14				((vu32)0x09000000)
#define	GPIO_Mode_AF_PP_10MHz_P15				((vu32)0x90000000)

#define	GPIO_Mode_AF_PP_2MHz_P0					((vu32)0x0000000A)
#define	GPIO_Mode_AF_PP_2MHz_P1					((vu32)0x000000A0)
#define	GPIO_Mode_AF_PP_2MHz_P2					((vu32)0x00000A00)
#define	GPIO_Mode_AF_PP_2MHz_P3					((vu32)0x0000A000)
#define	GPIO_Mode_AF_PP_2MHz_P4					((vu32)0x000A0000)
#define	GPIO_Mode_AF_PP_2MHz_P5					((vu32)0x00A00000)
#define	GPIO_Mode_AF_PP_2MHz_P6					((vu32)0x0A000000)
#define	GPIO_Mode_AF_PP_2MHz_P7					((vu32)0xA0000000)
#define	GPIO_Mode_AF_PP_2MHz_P8					((vu32)0x0000000A)
#define	GPIO_Mode_AF_PP_2MHz_P9					((vu32)0x000000A0)
#define	GPIO_Mode_AF_PP_2MHz_P10				((vu32)0x00000A00)
#define	GPIO_Mode_AF_PP_2MHz_P11				((vu32)0x0000A000)
#define	GPIO_Mode_AF_PP_2MHz_P12				((vu32)0x000A0000)
#define	GPIO_Mode_AF_PP_2MHz_P13				((vu32)0x00A00000)
#define	GPIO_Mode_AF_PP_2MHz_P14				((vu32)0x0A000000)
#define	GPIO_Mode_AF_PP_2MHz_P15				((vu32)0xA0000000)

#define	GPIO_Mode_AF_PP_50MHz_P0				((vu32)0x0000000B)
#define	GPIO_Mode_AF_PP_50MHz_P1				((vu32)0x000000B0)
#define	GPIO_Mode_AF_PP_50MHz_P2				((vu32)0x00000B00)
#define	GPIO_Mode_AF_PP_50MHz_P3				((vu32)0x0000B000)
#define	GPIO_Mode_AF_PP_50MHz_P4				((vu32)0x000B0000)
#define	GPIO_Mode_AF_PP_50MHz_P5				((vu32)0x00B00000)
#define	GPIO_Mode_AF_PP_50MHz_P6				((vu32)0x0B000000)
#define	GPIO_Mode_AF_PP_50MHz_P7				((vu32)0xB0000000)
#define	GPIO_Mode_AF_PP_50MHz_P8				((vu32)0x0000000B)
#define	GPIO_Mode_AF_PP_50MHz_P9				((vu32)0x000000B0)
#define	GPIO_Mode_AF_PP_50MHz_P10				((vu32)0x00000B00)
#define	GPIO_Mode_AF_PP_50MHz_P11				((vu32)0x0000B000)
#define	GPIO_Mode_AF_PP_50MHz_P12				((vu32)0x000B0000)
#define	GPIO_Mode_AF_PP_50MHz_P13				((vu32)0x00B00000)
#define	GPIO_Mode_AF_PP_50MHz_P14				((vu32)0x0B000000)

//悫???a????3?????
#define	GPIO_Mode_Out_OD_10MHz_P0				((vu32)0x00000005)
#define	GPIO_Mode_Out_OD_10MHz_P1				((vu32)0x00000050)
#define	GPIO_Mode_Out_OD_10MHz_P2				((vu32)0x00000500)
#define	GPIO_Mode_Out_OD_10MHz_P3				((vu32)0x00005000)
#define	GPIO_Mode_Out_OD_10MHz_P4				((vu32)0x00050000)
#define	GPIO_Mode_Out_OD_10MHz_P5				((vu32)0x00500000)
#define	GPIO_Mode_Out_OD_10MHz_P6				((vu32)0x05000000)
#define	GPIO_Mode_Out_OD_10MHz_P7				((vu32)0x50000000)
#define	GPIO_Mode_Out_OD_10MHz_P8				((vu32)0x00000005)
#define	GPIO_Mode_Out_OD_10MHz_P9				((vu32)0x00000050)
#define	GPIO_Mode_Out_OD_10MHz_P10				((vu32)0x00000500)
#define	GPIO_Mode_Out_OD_10MHz_P11				((vu32)0x00005000)
#define	GPIO_Mode_Out_OD_10MHz_P12				((vu32)0x00050000)
#define	GPIO_Mode_Out_OD_10MHz_P13				((vu32)0x00500000)
#define	GPIO_Mode_Out_OD_10MHz_P14				((vu32)0x05000000)
#define	GPIO_Mode_Out_OD_10MHz_P15				((vu32)0x50000000)

#define	GPIO_Mode_Out_OD_2MHz_P0				((vu32)0x00000006)
#define	GPIO_Mode_Out_OD_2MHz_P1				((vu32)0x00000060)
#define	GPIO_Mode_Out_OD_2MHz_P2				((vu32)0x00000600)
#define	GPIO_Mode_Out_OD_2MHz_P3				((vu32)0x00006000)
#define	GPIO_Mode_Out_OD_2MHz_P4				((vu32)0x00060000)
#define	GPIO_Mode_Out_OD_2MHz_P5				((vu32)0x00600000)
#define	GPIO_Mode_Out_OD_2MHz_P6				((vu32)0x06000000)
#define	GPIO_Mode_Out_OD_2MHz_P7				((vu32)0x60000000)
#define	GPIO_Mode_Out_OD_2MHz_P8				((vu32)0x00000006)
#define	GPIO_Mode_Out_OD_2MHz_P9				((vu32)0x00000060)
#define	GPIO_Mode_Out_OD_2MHz_P10				((vu32)0x00000600)
#define	GPIO_Mode_Out_OD_2MHz_P11				((vu32)0x00006000)
#define	GPIO_Mode_Out_OD_2MHz_P12				((vu32)0x00060000)
#define	GPIO_Mode_Out_OD_2MHz_P13				((vu32)0x00600000)
#define	GPIO_Mode_Out_OD_2MHz_P14				((vu32)0x06000000)
#define	GPIO_Mode_Out_OD_2MHz_P15				((vu32)0x60000000)

#define	GPIO_Mode_Out_OD_50MHz_P0				((vu32)0x00000007)
#define	GPIO_Mode_Out_OD_50MHz_P1				((vu32)0x00000070)
#define	GPIO_Mode_Out_OD_50MHz_P2				((vu32)0x00000700)
#define	GPIO_Mode_Out_OD_50MHz_P3				((vu32)0x00007000)
#define	GPIO_Mode_Out_OD_50MHz_P4				((vu32)0x00070000)
#define	GPIO_Mode_Out_OD_50MHz_P5				((vu32)0x00700000)
#define	GPIO_Mode_Out_OD_50MHz_P6				((vu32)0x07000000)
#define	GPIO_Mode_Out_OD_50MHz_P7				((vu32)0x70000000)
#define	GPIO_Mode_Out_OD_50MHz_P8				((vu32)0x00000007)
#define	GPIO_Mode_Out_OD_50MHz_P9				((vu32)0x00000070)
#define	GPIO_Mode_Out_OD_50MHz_P10				((vu32)0x00000700)
#define	GPIO_Mode_Out_OD_50MHz_P11				((vu32)0x00007000)
#define	GPIO_Mode_Out_OD_50MHz_P12				((vu32)0x00070000)
#define	GPIO_Mode_Out_OD_50MHz_P13				((vu32)0x00700000)
#define	GPIO_Mode_Out_OD_50MHz_P14				((vu32)0x07000000)
#define	GPIO_Mode_Out_OD_50MHz_P15				((vu32)0x70000000)

//?'??1|???a????3?????
#define	GPIO_Mode_AF_OD_10MHz_P0				((vu32)0x0000000D)
#define	GPIO_Mode_AF_OD_10MHz_P1				((vu32)0x000000D0)
#define	GPIO_Mode_AF_OD_10MHz_P2				((vu32)0x00000D00)
#define	GPIO_Mode_AF_OD_10MHz_P3				((vu32)0x0000D000)
#define	GPIO_Mode_AF_OD_10MHz_P4				((vu32)0x000D0000)
#define	GPIO_Mode_AF_OD_10MHz_P5				((vu32)0x00D00000)
#define	GPIO_Mode_AF_OD_10MHz_P6				((vu32)0x0D000000)
#define	GPIO_Mode_AF_OD_10MHz_P7				((vu32)0xD0000000)
#define	GPIO_Mode_AF_OD_10MHz_P8				((vu32)0x0000000D)
#define	GPIO_Mode_AF_OD_10MHz_P9				((vu32)0x000000D0)
#define	GPIO_Mode_AF_OD_10MHz_P10				((vu32)0x00000D00)
#define	GPIO_Mode_AF_OD_10MHz_P11				((vu32)0x0000D000)
#define	GPIO_Mode_AF_OD_10MHz_P12				((vu32)0x000D0000)
#define	GPIO_Mode_AF_OD_10MHz_P13				((vu32)0x00D00000)
#define	GPIO_Mode_AF_OD_10MHz_P14				((vu32)0x0D000000)
#define	GPIO_Mode_AF_OD_10MHz_P15				((vu32)0xD0000000)

#define	GPIO_Mode_AF_OD_2MHz_P0					((vu32)0x0000000E)
#define	GPIO_Mode_AF_OD_2MHz_P1					((vu32)0x000000E0)
#define	GPIO_Mode_AF_OD_2MHz_P2					((vu32)0x00000E00)
#define	GPIO_Mode_AF_OD_2MHz_P3					((vu32)0x0000E000)
#define	GPIO_Mode_AF_OD_2MHz_P4					((vu32)0x000E0000)
#define	GPIO_Mode_AF_OD_2MHz_P5					((vu32)0x00E00000)
#define	GPIO_Mode_AF_OD_2MHz_P6					((vu32)0x0E000000)
#define	GPIO_Mode_AF_OD_2MHz_P7					((vu32)0xE0000000)
#define	GPIO_Mode_AF_OD_2MHz_P8					((vu32)0x0000000E)
#define	GPIO_Mode_AF_OD_2MHz_P9					((vu32)0x000000E0)
#define	GPIO_Mode_AF_OD_2MHz_P10				((vu32)0x00000E00)
#define	GPIO_Mode_AF_OD_2MHz_P11				((vu32)0x0000E000)
#define	GPIO_Mode_AF_OD_2MHz_P12				((vu32)0x000E0000)
#define	GPIO_Mode_AF_OD_2MHz_P13				((vu32)0x00E00000)
#define	GPIO_Mode_AF_OD_2MHz_P14				((vu32)0x0E000000)
#define	GPIO_Mode_AF_OD_2MHz_P15				((vu32)0xE0000000)

#define	GPIO_Mode_AF_OD_50MHz_P0				((vu32)0x0000000F)
#define	GPIO_Mode_AF_OD_50MHz_P1				((vu32)0x000000F0)
#define	GPIO_Mode_AF_OD_50MHz_P2				((vu32)0x00000F00)
#define	GPIO_Mode_AF_OD_50MHz_P3				((vu32)0x0000F000)
#define	GPIO_Mode_AF_OD_50MHz_P4				((vu32)0x000F0000)
#define	GPIO_Mode_AF_OD_50MHz_P5				((vu32)0x00F00000)
#define	GPIO_Mode_AF_OD_50MHz_P6				((vu32)0x0F000000)
#define	GPIO_Mode_AF_OD_50MHz_P7				((vu32)0xF0000000)
#define	GPIO_Mode_AF_OD_50MHz_P8				((vu32)0x0000000F)
#define	GPIO_Mode_AF_OD_50MHz_P9				((vu32)0x000000F0)
#define	GPIO_Mode_AF_OD_50MHz_P10				((vu32)0x00000F00)
#define	GPIO_Mode_AF_OD_50MHz_P11				((vu32)0x0000F000)
#define	GPIO_Mode_AF_OD_50MHz_P12				((vu32)0x000F0000)
#define	GPIO_Mode_AF_OD_50MHz_P13				((vu32)0x00F00000)
#define	GPIO_Mode_AF_OD_50MHz_P14				((vu32)0x0F000000)
#define	GPIO_Mode_AF_OD_50MHz_P15				((vu32)0xF0000000)

//---------------------------------------------------------------------------
#define TIM_ENABLE						        ((vu16)0x0001)		//使能计数器
#define TIM_DISABLE						        ((vu16)0xFFFE)		//失能计数器

#endif


