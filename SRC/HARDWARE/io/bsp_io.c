/**
 * @file      : bsp_io.c
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
#include "common.h"

void bsp_IOInit(void)
{
        RCC->APB2ENR |= RCC_APB2Periph_AFIO;
        RCC->APB2ENR |= (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB);

        /// C版本
#ifdef C_901
        // FB OUT  1
        GPIOA->CRH &= (GPIO_Crh_P8);
        GPIOA->CRH |= (GPIO_Mode_Out_PP_50MHz_P8);
        GPIOA->ODR |= (GPIO_Pin_8);
        /// KEY OUT 1
        GPIOB->CRL &= (GPIO_Crl_P3);
        GPIOB->CRL |= (GPIO_Mode_Out_PP_50MHz_P3);
        GPIOB->ODR |= (GPIO_Pin_3);
#endif

#ifdef IOCTRL

#ifdef A12_901
        // FB OUT   PA8
        GPIOA->CRH &= (GPIO_Crh_P8);
        GPIOA->CRH |= (GPIO_Mode_Out_PP_50MHz_P8);
        GPIOA->ODR |= (GPIO_Pin_8);
        // KEY IN   PB3
        GPIOB->CRL &= (GPIO_Crl_P3);
        GPIOB->CRL |= (GPIO_Mode_IN_PU_PD_P3);
#endif

#ifdef A12_909
        // FB OUT   PB13
        GPIOB->CRH &= (GPIO_Crh_P13);
        GPIOB->CRH |= (GPIO_Mode_Out_PP_50MHz_P13);
        GPIOB->ODR |= (GPIO_Pin_13);
        // KEY IN   PB5
        GPIOB->CRL &= (GPIO_Crl_P5);
        GPIOB->CRL |= (GPIO_Mode_IN_PU_PD_P5);
#endif

#ifdef A12_906
        // FB OUT   PB13
        GPIOB->CRH &= (GPIO_Crh_P13);
        GPIOB->CRH |= (GPIO_Mode_Out_PP_50MHz_P13);
        GPIOB->ODR |= (GPIO_Pin_13);
        // KEY IN   PB14
        GPIOB->CRH &= (GPIO_Crh_P14);
        GPIOB->CRH |= (GPIO_Mode_IN_PU_PD_P14);
#endif
#endif
}
// clang-format off
///                A                  B            状态
/// A12-901  IN悬空/1  输出1    IN悬空/1  输出0    状态A
///          IN0       输出0     IN0     输出1    状态B
/// A12-906  BI 悬空/5.0V  输出0
/// A12-909  IN1    IN2    OUT1    OUT2    FBOUT    ERROUT
///          0       0       0       0       0       0
///          1       0       1       0       0       0
// clang-format on
void bsp_IODetect(void)
{
#ifdef IOCTRL
        /* IO (100ms) */
        if (false == syspara.ioCtrl) {
                return; /* IO未开 */
        }

        if (valve.status != VALVE_RUN_END) {
                return; /* 阀门未运行结束 */
        }

        if (timerPara.timeMilli < DCSEC) {
                return; /* 时间未到 */
        }
        timerPara.timeMilli = 0;

        /* AI/IN  -- 1 */
        if (IO_IN == ON) {
                // 未接通
                if (valve.portCur == POS_B) {
                        valve.portDes = POS_A;
                }
  #if IO_RS  // IO_RS 1 A 232/485/IO
    #ifdef A12_901
                IO_OUT = ON;  // AI -- 1  BO -- 1
    #endif
    #ifdef A12_906
                IO_OUT = OFF;  // AI -- 1  BO -- 0
    #endif
    #ifdef A12_909
                IO_OUT = ON;  // AI -- 1  BO -- 0(无AB区别)
    #endif
  #else  // IO_RS 0 B IO
    #ifdef A12_901
                IO_OUT = OFF;  // AI -- 1  BO -- 0
    #endif
    #ifdef A12_906
                IO_OUT = ON;  // AI -- 1  BO -- 1
    #endif
    #ifdef A12_909
                IO_OUT = ON;  // AI -- 1  BO -- 0(无AB区别)
    #endif
  #endif
        }
        /* AI/IN  -- 0 */
        else {
                if (valve.portCur == POS_A) {
                        valve.portDes = POS_B;
                }
  #if IO_RS  // IO_RS 1 A 232/485/IO
    #ifdef A12_901
                IO_OUT = OFF;  // AI -- 0  BO -- 0
    #endif
    #ifdef A12_906
                IO_OUT = ON;  // AI -- 0  BO -- 1
    #endif
    #ifdef A12_909
                IO_OUT = OFF;  // AI -- 0  BO -- 1(无AB区别)
    #endif
  #else  // IO_RS 0 B IO
    #ifdef A12_901
                IO_OUT = ON;  // AI -- 0  BO -- 1
    #endif
    #ifdef A12_906
                IO_OUT = OFF;  // AI -- 0  BO -- 0
    #endif
    #ifdef A12_909
                IO_OUT = OFF;  // AI -- 0  BO -- 0(无AB区别)
    #endif
  #endif /* IO_RS */
        }
#endif /* IOCTRL */

        /// C901 IO输出灯状态
#ifdef C_901
        bsp_IOLedOut();
#endif
}

/**
 * @brief    C901 IO输出灯状态
 */
#ifdef C_901
void bsp_IOLedOut(void)
{
        /// IO输出灯状态
        if (valve.status != VALVE_RUN_END) {
                return; /* 阀门未运行结束 */
        }

        if (timerPara.timeMilli < 210) {
                return; /* 时间未到 */
        }
        timerPara.timeMilli = 0;

        switch (valve.portCur) {
                case POS_A:
                        IO_IN = ON;
                        IO_OUT = OFF;
                        break;
                case POS_B:
                        IO_IN = OFF;
                        IO_OUT = ON;
                        break;
                case POS_M:
                        IO_IN = OFF;
                        IO_OUT = OFF;
                        break;
                default:
                        IO_IN = ON;
                        IO_OUT = ON;
                        break;
        }
}
#endif
