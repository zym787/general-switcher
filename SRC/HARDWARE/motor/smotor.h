#ifndef _SMOTOR_H_
#define _SMOTOR_H_

#ifdef _SMOTOR_H_GLOBALS_
#define PEXT
#else
#define PEXT extern
#endif
//--------------------------------------------------------------------------------------------------
#define AXXN                0
#define AXYN                1
#define AXZN                2
#define AXPN                3
#define AXMN                4

//--------------------------------------------------------------------------------------------------
#define POSITIVE            1
#define NEGATIVE            -1

//从电机屁股的方向看，顺时针与逆时针
#define CW                  0   //clock wise顺时针
#define CCW                 1   //counter clock wise逆时针

#define FALSE               0
#define TRUE                1

#define T1_FREQ             1000000
#define SPR                 12800

// Maths constants. To simplify maths when calculating in AxisMoveRel().
#define ALPHA               (2*3.14159/SPR)                         // 2*pi/spr
#define A_T_x100            ((int)(ALPHA*T1_FREQ*100))              // (ALPHA / T1_FREQ)*100
#define T1_FREQ_148         ((int)((T1_FREQ*0.676)/100))            // divided by 100 and scaled by 0.676
#define A_SQ                (int)(ALPHA*2*100000*100000)
#define A_x20000            (int)(ALPHA*20000)                      // ALPHA*20000

// Speed ramp states
#define STOP                0
#define ACCEL               1
#define DECEL               2
#define RUN                 3

#define AXIS_N              5
PEXT int steps[AXIS_N];              //运动距离
PEXT unsigned int accel[AXIS_N];     //加速度
PEXT unsigned int decel[AXIS_N];     //减速度
PEXT unsigned int speed[AXIS_N];     //速度
PEXT int stpdecel[AXIS_N];           //急停减速度
PEXT int position[AXIS_N];           //当前位置
PEXT int HomePos[AXIS_N];

PEXT unsigned char bEmgStopping[AXIS_N]; //是否碰限位急停
PEXT unsigned char bLmtPos[AXIS_N];
PEXT unsigned char bLmtNeg[AXIS_N];
PEXT unsigned char bStopCmd[AXIS_N];
PEXT unsigned char MotionStatus[AXIS_N]; //是否在运动？0：停止，1：运动

typedef struct 
{
    unsigned char   run_state;              //What part of the speed ramp we are in.
    unsigned char   dir;                    //Direction stepper motor should move.
    int             step_delay;             //Peroid of next timer delay. At start this value set the acceleration rate.
    unsigned int    decel_start;            //What step_pos to start decelaration
    int             decel_val;              //Sets deceleration rate.
    int             min_delay;              //Minimum time delay (max speed)
    int             accel_count;            //Counter used when accelerateing/decelerateing to calculate step_delay.
    void            (*SearchOrg)(void);     //
    unsigned char*  bEmgStop;               //
    volatile unsigned long*     signalCLK;      //时钟信号引脚
    volatile unsigned long*     signalDIR;      //方向信号引脚
    volatile unsigned short*    signalCCR1;     //时钟信号
    volatile unsigned short*    signalARR;      //时钟信号
    volatile unsigned short*    signalCR1;      //时钟信号
}AXIS_T;
PEXT AXIS_T srd[AXIS_N];


PEXT float CarmackSqrt (float x);
PEXT void AxisMid(unsigned char axis, int step, unsigned int accel, unsigned int decel, unsigned int speed);
PEXT void AxisMoveAbs(unsigned char axis, int step, unsigned int accel, unsigned int decel, unsigned int speed);
PEXT void AxisMoveRel(unsigned char axis, int step, unsigned int accel, unsigned int decel, unsigned int speed);
PEXT void AxisXTimer(void);
PEXT void AxisYTimer(void);
PEXT void AxisZTimer(void);
PEXT void AxisPTimer(void);
PEXT void AxisMTimer(void);

#undef PEXT
#endif

