#ifndef _DELAY_H_
#define _DELAY_H_

#ifdef _DELAY_H_GLOBALS_
#define PEXT
#else
#define PEXT extern
#endif


PEXT void delay_init(u8 SYSCLK);
PEXT void delay_ms(u16 nms);
PEXT void delay_us(u32 nus);








#undef PEXT
#endif




























