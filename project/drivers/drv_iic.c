#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "rtconfig.h"

// #define DBG_ENABLE
#define DBG_SECTION_NAME  "[IIC]"
#define DBG_LEVEL         DBG_LOG
// #define DBG_LEVEL         DBG_INFO
// #define DBG_LEVEL         DBG_WARNING
// #define DBG_LEVEL         DBG_ERROR
#define DBG_COLOR
#include <rtdbg.h>

#define I2C_CLK (2)
#define I2C_DAT (3)

#ifdef BEKEN_USING_IIC
#include "typedef.h"
#include "drv_iic.h"
#include "gpio.h"
#include "gpio_pub.h"
#if defined(RT_USING_I2C) && defined(RT_USING_I2C_BITOPS)

static void iic_udelay(rt_uint32_t us)
{
	volatile rt_int32_t i = us;
    while (i-- > 0) i = i;
}

static void iic_gpio_init(void)
{
    bk_gpio_config_input_pup(I2C_DAT);
    bk_gpio_config_input_pup(I2C_CLK);
}

static void iic_set_sda(void *data, rt_int32_t state)
{
    dbg_log(DBG_LOG, "set sda v:%d\n", state);

    if (state)
    {
        bk_gpio_config_input_pup(I2C_DAT);
    }
    else
    {
        bk_gpio_config_output(I2C_DAT);
        bk_gpio_output(I2C_DAT, state);
    }
}

static void iic_set_scl(void *data, rt_int32_t state)
{
    dbg_log(DBG_LOG, "set scl v:%d\n", state);

    if (state)
    {
        bk_gpio_config_input_pup(I2C_CLK);
    }
    else
    {
        bk_gpio_config_output(I2C_CLK);
        bk_gpio_output(I2C_CLK, state);
    }
}

static rt_int32_t  iic_get_sda(void *data)
{
    rt_int32_t state;

    state = bk_gpio_input(I2C_DAT);
    dbg_log(DBG_LOG, "get sda v:%d\n", state);

    return state;
}

static rt_int32_t  iic_get_scl(void *data)
{
    rt_int32_t state;

    state = bk_gpio_input(I2C_CLK);
    dbg_log(DBG_LOG, "get scl v:%d\n", state);

    return state;
}

static const struct rt_i2c_bit_ops bit_ops = {
	RT_NULL,
	iic_set_sda,
	iic_set_scl,
	iic_get_sda,
	iic_get_scl,
	iic_udelay,
	20,
	50
};

static struct rt_i2c_bus_device i2c_bus;

int iic_bus_attach(void)
{
	iic_gpio_init();
	rt_kprintf("---[wf]iic_bus_attach\r\n---");

	i2c_bus.priv = (void *)&bit_ops;
	rt_i2c_bit_add_bus(&i2c_bus, "i2c");
	rt_kprintf(" iic_bus_attach\r\n");

    return RT_EOK;
}
INIT_DEVICE_EXPORT(iic_bus_attach);

#endif

#endif
