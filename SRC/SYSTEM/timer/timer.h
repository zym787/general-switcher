#ifndef _TIMER_H_
#define _TIMER_H_

#ifdef _TIMER_H_GLOBALS_
#define PEXT
#else
#define PEXT extern
#endif



typedef struct
{
    unsigned short timeDbg;
    unsigned short timeCmd;
    unsigned short timeMilli;
    unsigned short timeWaitMill;
    unsigned short timeOut;
    unsigned short intDelay;
    unsigned short sec;
    uint8_t timeLedDetect;
}_TIMER_T;

PEXT _TIMER_T timerPara;

#define SEC     1000
#define HSEC    500
#define DCSEC    100


PEXT void TIM2_Init(u16 arr,u16 psc);
PEXT void TIM3_Init(u16 arr,u16 psc);
PEXT void TIM4_Init(u16 arr,u16 psc);
#if STM32RC_C8
PEXT void TIM5_Init(u16 arr,u16 psc);
PEXT void TIM6_Init(u16 arr,u16 psc);
PEXT void TIM7_Init(u16 arr,u16 psc);
PEXT void EXTI_Init(void);
#endif






#undef PEXT
#endif






















