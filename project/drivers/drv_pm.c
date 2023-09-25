#include <rthw.h>
#include <rtthread.h>

#include <stdint.h>

#include "sys_rtos.h"
#include "bk_rtos_pub.h"
#include "power_save_pub.h"

extern void WFI(void);
extern UINT32 mcu_power_save(UINT32 sleep_tick);
extern void rt_user_idle_hook(void);


/*
0: normal
1: cpu sleep. -4mA
2: RF sleep. 27mA
3: RF+CPU sleep. 7mA
4: standby to reset. 7uA
*/
static char log_print = 0;

static void idle_hook(void)
{
    rt_tick_t timeout_tick, delta_tick=0;

    rt_enter_critical();
    /* get next os tick */
    timeout_tick = rt_timer_next_timeout_tick();
    if (timeout_tick != RT_TICK_MAX)
    {
        timeout_tick -= rt_tick_get();
    }

    #if CFG_USE_MCU_PS
    /* sleep cpu */
    delta_tick = mcu_power_save(timeout_tick);
    if(log_print)
        rt_kprintf("s:%d, d:%d\n", timeout_tick, delta_tick);
    #endif

    if (delta_tick)
    {
        /* adjust OS tick */
        rt_tick_set(rt_tick_get() + delta_tick);
        /* check system timer */
        rt_timer_check();
    }
    rt_exit_critical();

    rt_user_idle_hook();
}

static int drv_pm_init(void)
{
    rt_kprintf("%s\n", __FUNCTION__);
    rt_thread_idle_sethook(idle_hook);

    return 0;
}

int set_log(int argc, char *argv[])
{
    char val;

    val = atoi(argv[1]);

    if(val == 1)
    {
        log_print = 1;
    }
    else
    {
        log_print = 0;
    }
}

MSH_CMD_EXPORT(set_log, set_log on or off);
INIT_DEVICE_EXPORT(drv_pm_init);
