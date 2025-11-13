#ifndef  __APP_H__
#define  __APP_H__

/* 系统寄存器地址 */
#define AGS_R_STATE             0x00    /* 读状态地址 */
#define AGS_R_CUR_PORT          0x01    /* 读通道地址 */
#define AGS_RW_ADDR             0x02    /* 读地址 */
#define AGS_R_VERSION           0x03    /* 读版本地址 */
#define AGS_R_VOLUME            0x03    /* 读体积地址? */
#define AGS_R_FIXORG            0x04    /* 读原点补偿地址 */
#define AGS_R_FIXDIR            0x05    /* 读方向补偿地址 */
#define AGS_R_BAUDRATE          0x07    /* 读波特率地址 */

#define AGS_RW_SN               0x08    /* 读写序列号地址 */
#define AGS_W_P_ABS_VOLUME      0x41    /* 'A' 写绝对体积位置地址 */
#define AGS_W_P_REL_DISPENSE    0x44    /* 'D' 写相对推出地址 */
#define AGS_W_JERK              0x45    /* 'E' 写急停地址 */
#define AGS_RW_P_FIXVOL         0x46    /* 'F' 读写泵体积点补偿地址 */
#define AGS_W_V_IN_PORT         0x49    /* 'I' 写通道地址 */
#define AGS_RW_P_FIXORG         0x4B    /* 'K' 读写泵原点补偿地址 */
#define AGS_W_V_OUT_PORT        0x4F    /* 'O' 写通道地址 */
#define AGS_W_P_REL_PICK        0x50    /* 'P' 写相对抽取地址 */
#define AGS_RW_P_SPEED          0x53    /* 'S' 读写抽推速度地址 */
#define AGS_W_RESET             0x59    /* 'Y' 写复位地址 */
#define AGS_RW_P_ABS_STEP       0x61    /* 'a' 读写泵绝对步数位置地址 */

/// 回复方式
enum AGS_REPLY_MODE
{
    REPLYMODE_AGS = 0ul,      /* AGS标准回复方式 */
    REPLYMODE_CUSTOM_1 = 1ul, /* 定制回复方式1 */
    REPLYMODE_CUSTOM_2 = 2ul, /* 保留 */
    REPLYMODE_CUSTOM_3 = 3ul, /* 保留 */
};

#endif
