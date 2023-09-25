#include "typedef.h"
#include "pwm_pub.h"
#include "icu_pub.h"
#include "drv_model_pub.h"
#include "fake_clock_pub.h"
#include <rtthread.h>
#include "power_save_pub.h"

#if CFG_BK7221_MDM_WATCHDOG_PATCH
void rc_reset_patch(void);
#endif

static volatile UINT32 current_clock = 0;
static volatile UINT32 current_seconds = 0;
static UINT32 second_countdown = FCLK_SECOND;

static void fclk_hdl(UINT8 param)
{
	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();
#if CFG_USE_STA_PS
    if(power_save_use_timer0())
    {
        power_save_pwm0_isr(param);
        return;
    }
#endif
	current_clock ++;

    #if CFG_BK7221_MDM_WATCHDOG_PATCH
    rc_reset_patch();
    #endif

    rt_tick_increase();
	GLOBAL_INT_RESTORE();

	if (--second_countdown == 0) 
	{
		current_seconds ++;
		second_countdown = FCLK_SECOND;
	}
}


UINT32 fclk_update_tick(UINT32 tick)
{
    current_clock += tick;

    if(second_countdown <= tick)
        {
        current_seconds ++;
        second_countdown = FCLK_SECOND - (tick - second_countdown);
    }
    else
        {
        second_countdown -= tick;
    }

    return 0;
}

UINT32 fclk_get_tick(void)
{
	return current_clock;
}

UINT32 fclk_get_second(void)
{
	return current_seconds;
}

UINT32 fclk_from_sec_to_tick(UINT32 sec)
{
	return sec * FCLK_SECOND;
}
	
void fclk_reset_count(void)
{
	current_clock = 0;
	current_seconds = 0;
}

UINT32 fclk_cal_endvalue(UINT32 mode)
{
	UINT32 value = 1;
	
	if(PWM_CLK_32K == mode)
	{	/*32k clock*/
		value = FCLK_DURATION_MS * 32;
	}
	else if(PWM_CLK_26M == mode)
	{	/*26m clock*/
		value = FCLK_DURATION_MS * 26000;
	}

	return value;
}

void os_clk_init(void)
{
	UINT32 ret;
	pwm_param_t param;

	/*init pwm*/
	param.channel         = FCLK_PWM_ID;	
	param.cfg.bits.en     = PWM_ENABLE;
	param.cfg.bits.int_en = PWM_INT_EN;
	param.cfg.bits.mode   = PMODE_TIMER;
    
#if(CFG_RUNNING_PLATFORM == FPGA_PLATFORM)  // FPGA:PWM0-2-32kCLK, pwm3-5-24CLK
	param.cfg.bits.clk    = PWM_CLK_32K; 
#else
	param.cfg.bits.clk    = PWM_CLK_26M; 
#endif

	param.p_Int_Handler   = fclk_hdl;
	param.duty_cycle      = 0;
	param.end_value       = fclk_cal_endvalue((UINT32)param.cfg.bits.clk);
	
	ret = sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_PARAM, &param);
	ASSERT(PWM_SUCCESS == ret);
}

// eof

