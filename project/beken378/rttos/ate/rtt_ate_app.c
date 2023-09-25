#include "rtt_ate_app.h"
#if ATE_APP_FUN

#include "include.h"

#include "app.h"
#include "wlan_cli_pub.h"
#include "arm_arch.h"
#include "uart.h"
#include "rtthread.h"

// #define ATE_USE_DEGUB

#ifdef ATE_USE_DEGUB
#define ATE_DBG    rt_kprintf

static int ate_status = 0;
#else
#define ATE_DBG
#endif

char ate_mode_state = 0;
static int uart_index = 0;

void ate_gpio_init(void)
{
    uint32_t param;
    char *name = RT_NULL;
    rt_device_t dev = RT_NULL;
#if 1
    dev = rt_console_get_device();
    if (dev != RT_NULL)
    {
        name = dev->parent.name;
        ATE_DBG("L:%d name:%s\n", __LINE__, name);
    }
#else
    name = finsh_get_device();
#endif
    if (name == RT_NULL)
    {
        name = RT_CONSOLE_DEVICE_NAME;
    }
    if (rt_strcmp(name, "uart1") == 0)
    {
        uart_index = 1;
        param = GPIO_CFG_PARAM(ATE_GPIO_ID1, GMODE_INPUT_PULLUP);
        ATE_DBG("L:%d init use UART1\n", __LINE__);
        while(!UART1_TX_FIFO_EMPTY_GET());
    }
    else if (rt_strcmp(name, "uart2") == 0)
    {
        uart_index = 2;
        param = GPIO_CFG_PARAM(ATE_GPIO_ID2, GMODE_INPUT_PULLUP);
        ATE_DBG("L:%d init use UART2\n", __LINE__);
        while(!UART2_TX_FIFO_EMPTY_GET());
    }
    else
    {
        uart_index = 0;
        rt_kprintf("[ate] err!! not find uart\n");
        return;
    }

    rt_thread_delay(rt_tick_from_millisecond(2));
    gpio_ctrl( CMD_GPIO_CFG, &param);
}

void ate_gpio_recovery(void)
{
    uint32_t param;

    if (uart_index == 1)
    {
        param = GFUNC_MODE_UART1;
        ATE_DBG("L:%d resuse UART1\n", __LINE__);
    }
    else if (uart_index == 2)
    {
        param = GFUNC_MODE_UART2;
        ATE_DBG("L:%d resuse UART2\n", __LINE__);
    }
    else
    {
        return;
    }

    gpio_ctrl( CMD_GPIO_ENABLE_SECOND, &param);
}

uint32_t ate_mode_check(void)
{
    uint32_t ret;
    uint32_t param;

    if (uart_index == 1)
    {
        param = ATE_GPIO_ID1;
    }
    else if (uart_index == 2)
    {
        param = ATE_GPIO_ID2;
    }
    else
    {
        return 0;
    }
    ret = gpio_ctrl( CMD_GPIO_INPUT, &param);
    ATE_DBG("L:%d pin value:%d\n", __LINE__, ret);
    return (0 == ret);
}

void ate_app_init(void)
{
    uint32_t mode = 0;
    rt_base_t level;

    level = rt_hw_interrupt_disable();

    ate_gpio_init();

    mode = ate_mode_check();
    ATE_DBG("L:%d mode:%d\n", __LINE__, mode);
#ifdef ATE_USE_DEGUB
    ate_status = mode;
#endif
    if(mode)
    {
        ate_mode_state = (char)1;
    }
    else
    {
        ate_mode_state = (char)0;
    }

    ate_gpio_recovery();

    rt_hw_interrupt_enable(level);
}

uint32_t get_ate_mode_state(void)
{
    //ATE_PRT("ateflag:%d\r\n", ate_mode_state);
#if 0
    rt_kprintf("test... the state is 1\n");
    return 1;
#else
    if(ate_mode_state != (char)0)
    {
        return 1;
    }

    return 0;
#endif
}

void ate_start(void)
{
    app_pre_start();

    ATE_PRT("ate_start\r\n");
}
#ifdef ATE_USE_DEGUB
int ate_status_dump(int argc, char *argv[])
{
    rt_kprintf("status:%d\n", ate_status);
    return 0;
}
FINSH_FUNCTION_EXPORT_ALIAS(ate_status_dump, __cmd_ate_dump, ate status dump);
#endif
#endif /*ATE_APP_FUN */
// eof
