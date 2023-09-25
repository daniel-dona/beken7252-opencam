#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "typedef.h"
#include "drv_wdt.h"
#include "wdt.h"

#ifdef BEKEN_USING_WDT

struct _wdt_dev
{
    struct rt_device parent;
};

static struct _wdt_dev wdt_dev;

static rt_err_t rt_wdt_control(rt_device_t dev, int cmd, void *args)
{
    switch (cmd)
    {
        case WCMD_POWER_UP:
        case WCMD_SET_PERIOD:
        case WCMD_RELOAD_PERIOD:
        case WCMD_POWER_DOWN:
            wdt_ctrl(cmd, args);
            break;
#ifdef RT_USING_WDT
        case RT_DEVICE_CTRL_WDT_START:
            wdt_ctrl(WCMD_POWER_UP, RT_NULL);
            break;
        case RT_DEVICE_CTRL_WDT_SET_TIMEOUT:
            wdt_ctrl(WCMD_SET_PERIOD, args);
            break;
        case RT_DEVICE_CTRL_WDT_KEEPALIVE:
            wdt_ctrl(WCMD_RELOAD_PERIOD, RT_NULL);
            break;
        case RT_DEVICE_CTRL_WDT_STOP:
            wdt_ctrl(WCMD_POWER_DOWN, RT_NULL);
            break;
        case RT_DEVICE_CTRL_WDT_GET_TIMEOUT:
        case RT_DEVICE_CTRL_WDT_GET_TIMELEFT:
#endif
        default:
            return (RT_ERROR);
    }

    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops wdt_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    rt_wdt_control
};
#endif /* RT_USING_DEVICE_OPS */

int rt_hw_wdt_init(void)
{
    struct rt_device *device;

    rt_memset(&wdt_dev, 0, sizeof(wdt_dev));
    device = &(wdt_dev.parent);
    /* Setting the drive type */
    device->type        = RT_Device_Class_Miscellaneous;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    /* Setting the driver interface */
#ifdef RT_USING_DEVICE_OPS
    device->ops         = &wdt_ops;
#else
    device->init        = RT_NULL;
    device->open        = RT_NULL;
    device->close       = RT_NULL;
    device->read        = RT_NULL;
    device->write       = RT_NULL;
    device->control     = rt_wdt_control;
#endif /* RT_USING_DEVICE_OPS */

    /* Setting the user data */
    device->user_data   = RT_NULL;

    /* register device */
    return rt_device_register(device, WDT_DEV_NAME, RT_DEVICE_FLAG_DEACTIVATE);
}
INIT_DEVICE_EXPORT(rt_hw_wdt_init);

int rt_hw_wdt_exit(void)
{
    /* unregister device */
    rt_device_unregister(&(wdt_dev.parent));
}

#endif

