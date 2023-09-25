#ifndef _TIMER_PUB_H_
#define _TIMER_PUB_H_


#define TIMER_DEV_NAME                "timer"


#define TIMER_CMD_MAGIC              (0xe340000)
enum
{
    CMD_TIMER_UNIT_ENABLE = TIMER_CMD_MAGIC + 1,
    CMD_TIMER_UNIT_DISABLE,
    CMD_TIMER_INIT_PARAM
};



typedef void (*TFUNC)(UINT8);

typedef struct
{
    UINT8 channel;
    UINT8 div;
    UINT32 period;
    TFUNC t_Int_Handler;
} timer_param_t;

void timer_init(void);
void timer_exit(void);
void timer_isr(void);


#endif //_TIMER_PUB_H_

