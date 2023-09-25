#include "include.h"
#include "arm_arch.h"

#include "timer_pub.h"
#include "timer.h"

#include "drv_model_pub.h"
#include "intc_pub.h"
#include "icu_pub.h"
#include "uart_pub.h"

#if (CFG_SOC_NAME == SOC_BK7231)
static void init_timer_param(timer_param_t *timer_param)
{}

UINT32 timer_ctrl(UINT32 cmd, void *param)
{}

void timer_init(void)
{}

void timer_exit(void)
{}

void timer_isr(void)
{
}
#else
static SDD_OPERATIONS timer_op =
{
    timer_ctrl
};

void (*p_TIMER_Int_Handler[TIMER_CHANNEL_NO])(UINT8) = {NULL,};

static void init_timer_param(timer_param_t *timer_param)
{
    UINT32 value;
    UINT32 ucChannel = timer_param->channel;

    if((timer_param == NULL)
            && (timer_param->channel > 5))
    {
        return;
    }


    p_TIMER_Int_Handler[timer_param->channel] = timer_param->t_Int_Handler;

    if(ucChannel < 3)
    {
        value = (PWD_TIMER_26M_CLK);
        sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, (void *)&value);

        value = ((UINT32)timer_param->period);
        REG_WRITE(REG_TIMERCTLA_PERIOD_ADDR(ucChannel), value);

        value = REG_READ(TIMER0_2_CTL);
        value &= ~(TIMERCTLA_CLKDIV_MASK << TIMERCTLA_CLKDIV_POSI);
        value |= ((timer_param->div - 1) << TIMERCTLA_CLKDIV_POSI);
        value |= (1 << ucChannel );
        REG_WRITE(TIMER0_2_CTL, value);
    }
    else
    {
        value = (PWD_TIMER_32K_CLK);
        sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, (void *)&value);

        value = ((UINT32)timer_param->period);
        REG_WRITE(REG_TIMERCTLB_PERIOD_ADDR(ucChannel), value);

        value = REG_READ(TIMER3_5_CTL);
        value &= ~(TIMERCTLB_CLKDIV_MASK << TIMERCTLB_CLKDIV_POSI);
        value |= ((timer_param->div - 1) << TIMERCTLB_CLKDIV_POSI);
        value |= (1 << (ucChannel - 3));
        REG_WRITE(TIMER3_5_CTL, value);
    }

    intc_enable(IRQ_TIMER);
}


UINT32 timer_ctrl(UINT32 cmd, void *param)
{
    UINT32 ret = TIMER_SUCCESS;
    UINT32 ucChannel;
    UINT32 value;
    timer_param_t *p_param;

    switch(cmd)
    {
    case CMD_TIMER_UNIT_ENABLE:
        ucChannel = (*(UINT32 *)param);
        if(ucChannel > 5)
        {
            ret = TIMER_FAILURE;
            break;
        }

        if(ucChannel < 3)
        {
            value = REG_READ(TIMER0_2_CTL);
            value |= (1 << ucChannel );
            REG_WRITE(TIMER0_2_CTL, value);
        }
        else
        {
            value = REG_READ(TIMER3_5_CTL);
            value |= (1 << (ucChannel - 3));
            REG_WRITE(TIMER3_5_CTL, value);
        }

        break;
    case CMD_TIMER_UNIT_DISABLE:
        ucChannel = (*(UINT32 *)param);
        if(ucChannel > 5)
        {
            ret = TIMER_FAILURE;
            break;
        }

        if(ucChannel < 3)
        {
            value = REG_READ(TIMER0_2_CTL);
            value &= ~(1 << ucChannel );
            REG_WRITE(TIMER0_2_CTL, value);
        }
        else
        {
            value = REG_READ(TIMER3_5_CTL);
            value &= ~(1 << (ucChannel - 3));
            REG_WRITE(TIMER3_5_CTL, value);
        }

        break;
    case CMD_TIMER_INIT_PARAM:
        p_param = (timer_param_t *)param;
        init_timer_param(p_param);
        break;
    default:
        ret = TIMER_FAILURE;
        break;
    }

    return ret;
}

void timer_init(void)
{
    intc_service_register(IRQ_TIMER, PRI_IRQ_TIMER, timer_isr);
    sddev_register_dev(TIMER_DEV_NAME, &timer_op);
}

void timer_exit(void)
{
    sddev_unregister_dev(TIMER_DEV_NAME);
}

void timer_isr(void)
{
    int i;
    UINT32 status;

    status = REG_READ(TIMER0_2_CTL);
    for(i = 0; i < 3; i++)
    {
        if(status & (1 << (i + TIMERCTLA_INT_POSI)))
        {
            if(p_TIMER_Int_Handler[i])
            {
                p_TIMER_Int_Handler[i]((UINT8)i);
            }
        }
    }

    do
    {
        REG_WRITE(TIMER0_2_CTL, status);
    }
    while(REG_READ(TIMER0_2_CTL) & status & (0x7 << TIMERCTLA_INT_POSI));

    status = REG_READ(TIMER3_5_CTL);
    for(i = 0; i < 3; i++)
    {
        if(status & (1 << (i + TIMERCTLB_INT_POSI)))
        {
            if(p_TIMER_Int_Handler[i + 3])
            {
                p_TIMER_Int_Handler[i + 3]((UINT8)(i + 3));
            }
        }
    }

    do
    {
        REG_WRITE(TIMER3_5_CTL, status);
    }
    while(REG_READ(TIMER3_5_CTL) & status & (0x7 << TIMERCTLB_INT_POSI));

}
#endif

