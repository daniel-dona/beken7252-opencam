#include "include.h"
#include "arm_arch.h"

#include "pwm.h"
#include "pwm_pub.h"
#include "bk_timer_pub.h"
#include "bk_timer.h"
#include "icu_pub.h"
#include "fake_clock_pub.h"
#include "target_util_pub.h"


void ps_pwm0_enable(void)
{
    UINT32 reg = 0;
    reg = REG_READ(PWM_CTL);
    reg &= (~0xf);
    reg |= 0x7;
    REG_WRITE(PWM_CTL, reg);
}

void ps_pwm0_disable(void )
{
    UINT32 reg;
    reg = REG_READ(PWM_CTL);
    REG_WRITE(PWM_CTL, reg & (~0xf));
}

void ps_pwm0_set_period(UINT32 period, UINT8 clk_mux)
{
    UINT32 reg = 0,ch;

    ch = PWM0;
    if(PWM_MUX_LPO == clk_mux)
    {
        icu_ctrl(CMD_CONF_PWM_LPOCLK, (void *)&ch);
    }
    else
    {
        icu_ctrl(CMD_CONF_PWM_PCLK, (void *)&ch);
    }
    
#if (CFG_SOC_NAME == SOC_BK7231)
    reg = 0;
    reg |= period << PWM0_END_POSI | 0x0 << PWM0_DC_POSI;
    REG_WRITE(PWM0_COUNTER , reg);
#else
	reg = period;
    REG_WRITE(PWM0_COUNTER , reg);
    reg = 0;
    REG_WRITE(PWM0_DUTY_CYCLE , reg);   
#endif
}


void ps_pwm0_reconfig(UINT32 period, UINT8 clk_mux)
{
    UINT32 param;
    //disable
#if (CFG_SOC_NAME == SOC_BK7231)	
    ps_pwm0_disable();
    delay(5);
    //new
    ps_pwm0_set_period(period, clk_mux);
    delay(1);
    //reenable
    ps_pwm0_enable();
#else
    ps_pwm0_disable();
    //new
    ps_pwm0_set_period(period, clk_mux);
    //reenable
    ps_pwm0_enable();
    delay(5);

    REG_WRITE(PWM_INTERRUPT_STATUS,0x3f);
#endif
	
}

void ps_pwm0_suspend_tick(UINT32 period)
{
    ps_pwm0_reconfig(period, PWM_MUX_LPO);
}

void ps_pwm0_resume_tick(void)
{
    ps_pwm0_reconfig(fclk_cal_endvalue(PWM_CLK_26M), PWM_MUX_PCLK);
}

#if (CFG_SOC_NAME != SOC_BK7231)
void ps_timer3_enable(UINT32 period)
{
    UINT32 reg;
    REG_WRITE(TIMER3_CNT,period);
    delay(5);
    reg = REG_READ(TIMER3_5_CTL);
    reg |= (TIMERCTL3_EN_BIT);
    reg &= ~(0x7 << TIMERCTLB_INT_POSI);
    REG_WRITE(TIMER3_5_CTL,reg);
}

void ps_timer3_disable(void)
{
    UINT32 reg;
    reg = REG_READ(TIMER3_5_CTL);
    reg &= (~TIMERCTL3_EN_BIT);
    reg &= ~(0x7 << TIMERCTLB_INT_POSI);
    REG_WRITE(TIMER3_5_CTL,reg);
}
#endif

