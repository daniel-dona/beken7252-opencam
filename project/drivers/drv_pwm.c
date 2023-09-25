#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "drivers/rt_drv_pwm.h"

#include "typedef.h"
#include "drv_pwm.h"
#include "pwm.h"
#include "generic.h"

#include "drv_model_pub.h"

#define MAX_PERIOD              (65535 / 26)
#define MIN_PERIOD              (2)

#define PWM_MIN_CHANNEL         (0)
#define PWM_MAX_CHANNEL         (5)

static rt_err_t drv_pwm_control(struct rt_device_pwm *device, int cmd, void *arg);
static rt_err_t rt_pwm_set_channel(pwm_param_t *param, struct rt_pwm_configuration *configuration);

static struct rt_pwm_ops drv_ops =
{
    drv_pwm_control
};

static rt_err_t drv_pwm_enable(pwm_param_t *param, struct rt_pwm_configuration *configuration, rt_bool_t enable)
{
    rt_err_t result = RT_EOK;
    UINT32 ret = DRV_SUCCESS;

    result = rt_pwm_set_channel(param, configuration);
    if (result != RT_EOK)
    {
        return result;
    }

    if (!enable)
    {
        param->cfg.bits.en     = PWM_DISABLE;
        ret = sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_PARAM, param);
    }
    else
    {
        param->cfg.bits.en     = PWM_ENABLE;
        ret = sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_PARAM, param);
    }

    if (ret != DRV_SUCCESS)
    {
        result = -RT_EIO;
    }

    return result;
}

static rt_err_t rt_pwm_set_channel(pwm_param_t *param, struct rt_pwm_configuration *configuration)
{
    if (configuration->channel < PWM_MIN_CHANNEL || configuration->channel > PWM_MAX_CHANNEL)
    {
        rt_kprintf("pwm channel invalid\n");
        return -RT_EINVAL;
    }
    param->channel = configuration->channel;

    return RT_EOK;
}

static rt_err_t drv_pwm_set(pwm_param_t *param, struct rt_pwm_configuration *configuration)
{
    UINT32 ret = RT_EOK;
    if (configuration->period < 0 || configuration->pulse < 0 || (configuration->period <= configuration->pulse))
    {
        rt_kprintf("invalid param\n");
        return -RT_ERROR;
    }

    configuration->period = configuration->period / 1000;
    configuration->pulse = configuration->pulse / 1000;
    if(configuration->period <= MIN_PERIOD || configuration->period > MAX_PERIOD)
    {
        rt_kprintf("invalid param, period should be 2000 ~ 2520000\n");
        return -RT_ERROR;
    }
    ret = rt_pwm_set_channel(param, configuration);
    if (ret == -RT_EINVAL)
    {
        return ret;
    }

    param->duty_cycle = configuration->pulse * 26;
    param->end_value = configuration->period * 26;
    ret = sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_PARAM, param);

    return ret;
}

static rt_err_t drv_pwm_control(struct rt_device_pwm *device, int cmd, void *arg)
{
    struct rt_pwm_configuration *configuration = (struct rt_pwm_configuration *)arg;
    pwm_param_t *param = (pwm_param_t *)device->parent.user_data;

    switch (cmd)
    {
    case PWM_CMD_ENABLE:
        return drv_pwm_enable(param, configuration, RT_TRUE);
    case PWM_CMD_DISABLE:
        return drv_pwm_enable(param, configuration, RT_FALSE);
    case PWM_CMD_SET:
        return drv_pwm_set(param, configuration);
    // case PWM_CMD_GET:
    //     return drv_pwm_get(param, configuration);
    default:
        return -RT_EINVAL;
    }
}

#ifdef RT_USING_PWM
    pwm_param_t pwm_param;
#endif

#ifdef RT_USING_PWM
static void rt_pwm_init(void)
{
    pwm_param.channel         = PWM0;
    pwm_param.cfg.bits.en     = PWM_DISABLE;
    pwm_param.cfg.bits.int_en = PWM_INT_DIS;
    pwm_param.cfg.bits.mode   = PMODE_PWM;
    pwm_param.cfg.bits.clk    = PWM_CLK_26M;
    pwm_param.p_Int_Handler   = 0;
    pwm_param.duty_cycle      = 0;
    pwm_param.end_value       = 0x00;
}
#endif /* RT_USING_PWM */

int drv_pwm_init(void)
{
#ifdef RT_USING_PWM
    rt_pwm_init();
    rt_device_pwm_register(rt_calloc(1, sizeof(struct rt_device_pwm)), "pwm", &drv_ops, &pwm_param);
#endif /* RT_USING_PWM */
}
INIT_DEVICE_EXPORT(drv_pwm_init);