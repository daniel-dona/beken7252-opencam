#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "typedef.h"
#include "drv_sys_ctrl.h"
#include "sys_ctrl.h"

struct _sys_ctrl_dev
{
    struct rt_device parent;
};

static struct _sys_ctrl_dev sys_ctrl_dev;

static rt_err_t rt_sys_ctrl_control(rt_device_t dev, int cmd, void *args)
{
    return sctrl_ctrl(cmd, args);
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops sys_ctrl_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    rt_sys_ctrl_control
};
#endif /* RT_USING_DEVICE_OPS */

int rt_hw_sys_ctrl_init(void)
{
    struct rt_device *device;

    rt_memset(&sys_ctrl_dev, 0, sizeof(sys_ctrl_dev));
    device = &(sys_ctrl_dev.parent);
    /* Setting the drive type */
    device->type        = RT_Device_Class_Miscellaneous;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    /* Setting the driver interface */
#ifdef RT_USING_DEVICE_OPS
    device->ops         = &sys_ctrl_ops;
#else
    device->init        = RT_NULL;
    device->open        = RT_NULL;
    device->close       = RT_NULL;
    device->read        = RT_NULL;
    device->write       = RT_NULL;
    device->control     = rt_sys_ctrl_control;
#endif /* RT_USING_DEVICE_OPS */

    /* Setting the user data */
    device->user_data   = RT_NULL;

    /* register device */
    return rt_device_register(device, SCTRL_DEV_NAME, RT_DEVICE_FLAG_DEACTIVATE);
}
INIT_DEVICE_EXPORT(rt_hw_sys_ctrl_init);

int rt_hw_sys_ctrl_exit(void)
{
    /* unregister device */
    rt_device_unregister(&(sys_ctrl_dev.parent));
}