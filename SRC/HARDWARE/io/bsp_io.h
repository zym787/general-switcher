/**
 * @file      : bsp_io.h
 * @brief     : IO
 *
 * @version   : 1.0
 * @author    : Drinkto
 * @date      : Apr 24, 2026
 *
 * @changelog:
 * | Date | version | Author | Description |
 * | --- | --- | --- | --- |
 * | Apr 24, 2026 | 1.0 | Drinkto | 初始化IO |
 */
#ifndef __BSP_IO_H
#define __BSP_IO_H

#ifdef A12_901
    #define IO_OUT          PAout(8)
    #ifndef C_901
        #define IO_IN           PBin(3)
    #else
        #define IO_IN           PBout(3)
    #endif
#endif
#ifdef A12_909
    #define IO_OUT          PBout(13)
    #define IO_IN           PBin(5)
#endif
#ifdef A12_906
    #define IO_OUT          PBout(13)
    #define IO_IN           PBin(14)
#endif

extern void bsp_IOInit(void);
extern void bsp_IODetect(void);
#ifdef C_901
void bsp_IOLedOut(void);
#endif

#endif // !__BSP_IO_H
