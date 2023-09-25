#include "rtthread.h"
#include "rtdevice.h"
#include "drv_iic.h"

#include "finsh.h"

#ifdef RT_USING_I2C2
#define THREAD_PRIORITY 20
#define THREAD_STACK_SIZE 2048
#define THREAD_TIMESLICE 5
static rt_thread_t tid = RT_NULL;


static rt_device_t _i2c_find(const char *name)
{
    rt_device_t dev;

    dev = rt_device_find(name);
    if (!dev)
    {
        rt_kprintf("search device failed: %s\n", name);
        return RT_NULL;
    }

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("open device failed: %s\n", name);
        return RT_NULL;
    }

    rt_kprintf("open i2c bus: %s\n", name);

    return dev;
}

static void _search_i2c_device(rt_device_t dev, uint8_t cmd)
{
    int count = 0;
    struct rt_i2c_msg msgs[2];
    uint8_t buf= 0;

    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &cmd;
    msgs[0].len   = sizeof(cmd);

    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = &buf;
    msgs[1].len   = 1; 
    
    int result = 0;

    for (int i = 0; i <= 0x7f; i++)
    {
        int len;
        result = 0;

        msgs[0].addr  = i;
        msgs[1].addr  = i;
        result = rt_i2c_transfer((struct rt_i2c_bus_device *)dev, msgs, 2);
        if (result == 2)
        {
            count++;
            rt_kprintf("add:0x%02X transfer success, id: 0x%02X\n", i, buf);
        }
    }

    rt_kprintf("i2c device: %d\n", count);
}


static void threadtest_entry(void *parameter)
{
    char *name = "i2c2";
    uint8_t cmd = 0x75;

    rt_device_t dev = _i2c_find(name);

    if (dev == RT_NULL)
    {
        rt_kprintf("search i2c device faild\n");
    }

    _search_i2c_device(dev, cmd);
    rt_device_close(dev);
}

static int test_i2c2(void)
{
    tid = rt_thread_create("i2c2",
                           threadtest_entry, RT_NULL,
                           THREAD_STACK_SIZE,
                           THREAD_PRIORITY, THREAD_TIMESLICE);

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    return 0;
}
MSH_CMD_EXPORT(test_i2c2, test hw i2c2);

#endif

