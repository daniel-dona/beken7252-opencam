#include "ate_app.h"
#if ATE_APP_FUN

#include "include.h"

#if (CFG_OS_FREERTOS)
#include "app.h"
#include "wlan_cli_pub.h"
#include "sys_ctrl_pub.h"
#endif

char ate_mode_state = 0;
int ate_gpio_port = GPIO0;

#define CMD_SINGLE_WAVE  "txevm -b 0 -r 24 -c 1 -w 1"

void ate_gpio_init(void)
{
    uint32_t param;

    param = GPIO_CFG_PARAM(ate_gpio_port, GMODE_INPUT_PULLUP);
    gpio_ctrl( CMD_GPIO_CFG, &param);
}

uint32_t ate_mode_check(void)
{
    uint32_t ret;
    uint32_t param;

    param = ate_gpio_port;
    ret = gpio_ctrl( CMD_GPIO_INPUT, &param);

    return (ATE_ENABLE_GIPO_LEVEL == ret);
}

void ate_app_init(void)
{
    uint32_t mode = 0;
    ate_gpio_init();

    mode = ate_mode_check();
    if(mode)
    {
        ate_mode_state = (char)1;
    }
    else
    {
        ate_mode_state = (char)0;
    }
}

uint32_t get_ate_mode_state(void)
{
    //ATE_PRT("ateflag:%d\r\n", ate_mode_state);
    if(ate_mode_state != (char)0)
        return 1;
    return 0;
}

#include "mem_pub.h"
#include "str_pub.h"

#ifdef SINGLE_WAVE_TEST
static void do_single_wave_test(void)
{
    uint32_t cmd_len = os_strlen(CMD_SINGLE_WAVE) + 1;
    uint8 *cmd_buf = os_malloc(cmd_len);
    if (cmd_buf) {
        extern void bk_test_cmd_handle_input(char *inbuf, int len);

        os_memcpy(cmd_buf, CMD_SINGLE_WAVE, cmd_len);
        bk_test_cmd_handle_input((char *)cmd_buf, cmd_len);
        os_free(cmd_buf);
    }
}
#endif

#if (CFG_OS_FREERTOS)
void ate_start(void)
{
    app_pre_start();

    cli_init();

    sctrl_rf_ps_enable_clear();

    ATE_PRT("ate_start\r\n");

    //do_single_wave_test();
}
#endif

#endif /*ATE_APP_FUN */
// eof
