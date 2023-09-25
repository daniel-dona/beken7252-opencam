#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "interrupt.h"
#include "typedef.h"
#include "drv_gpio.h"
#include "gpio.h"
#include "gpio_pub.h"

#define DBG_ENABLE
#define DBG_SECTION_NAME  "[GPIO]"
// #define DBG_LEVEL         DBG_LOG
// #define DBG_LEVEL         DBG_INFO
#define DBG_LEVEL         DBG_WARNING
// #define DBG_LEVEL         DBG_ERROR
#define DBG_COLOR
#include <rtdbg.h>

#ifdef BEKEN_USING_GPIO

typedef void (*gpio_isr_handler_t)(void *param);

struct gpio_irq_desc
{
    gpio_isr_handler_t handler;
    void              *param;
    rt_base_t          mode;
};

struct _gpio_dev
{
    struct rt_device *parent;
    struct gpio_irq_desc irq_desc[GPIONUM];
};

static struct _gpio_dev gpio_dev;

static void gpio_irq_dispatch(unsigned char index)
{
    struct _gpio_dev *_dev = &gpio_dev;

    dbg_log(DBG_LOG, "%s run...\n", __FUNCTION__);
    if (index >= GPIONUM)
    {
        dbg_log(DBG_ERROR, "%s index[%d] Wrongful\n", __FUNCTION__, index);
        return;
    }
    if (_dev->irq_desc[index].handler != RT_NULL)
    {
        dbg_log(DBG_LOG, "gpio irq pin:%d\n", index);
        _dev->irq_desc[index].handler(_dev->irq_desc[index].param);
    }
}

static void _gpio_mode(struct rt_device *device, rt_base_t pin, rt_base_t mode)
{
    RT_ASSERT(device != RT_NULL);

    switch (mode)
    {
    case PIN_MODE_INPUT:
        bk_gpio_config_input(pin);
        break;

    case PIN_MODE_INPUT_PULLUP:
        bk_gpio_config_input_pup(pin);
        break;

    case PIN_MODE_INPUT_PULLDOWN:
        bk_gpio_config_input_pdwn(pin);
        break;

    case PIN_MODE_OUTPUT:
        bk_gpio_config_output(pin);
        break;
    }
}

static void _gpio_write(struct rt_device *device, rt_base_t pin, rt_base_t value)
{
    RT_ASSERT(device != RT_NULL);

    bk_gpio_output(pin, value);
}

static int _gpio_read(struct rt_device *device, rt_base_t pin)
{
    RT_ASSERT(device != RT_NULL);

    return bk_gpio_input(pin);
}

static rt_err_t _gpio_attach_irq(struct rt_device *device, rt_int32_t pin, 
                                 rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    dbg_log(DBG_LOG, "attach irq pin:%d mode:%d\n", pin, mode);

    if (pin >= GPIONUM)
        return RT_ERROR;

    gpio_dev.irq_desc[pin].handler = hdr;
    gpio_dev.irq_desc[pin].param   = args;
    if (mode == PIN_IRQ_MODE_RISING)
    {
        gpio_dev.irq_desc[pin].mode = GPIO_INT_LEVEL_RISING;
    }
    else if (mode == PIN_IRQ_MODE_FALLING)
    {
        gpio_dev.irq_desc[pin].mode = GPIO_INT_LEVEL_FALLING;
    }
	else if (mode == PIN_IRQ_MODE_HIGH_LEVEL)
    {
        gpio_dev.irq_desc[pin].mode = GPIO_INT_LEVEL_HIGH;
    }
	else if (mode == PIN_IRQ_MODE_LOW_LEVEL)
    {
        gpio_dev.irq_desc[pin].mode = GPIO_INT_LEVEL_LOW;
    }
	
    return RT_EOK;
}

static rt_err_t _gpio_dettach_irq(struct rt_device *device, rt_int32_t pin)
{
    dbg_log(DBG_LOG, "dettach irq pin:%d\n", pin);

    if (pin >= GPIONUM)
        return RT_ERROR;

    gpio_int_disable(pin);
    rt_memset(&gpio_dev.irq_desc[pin], 0, sizeof(struct gpio_irq_desc));

    return RT_EOK;
}

static rt_err_t _gpio_irq_enable(struct rt_device *device, rt_base_t pin, rt_uint32_t enabled)
{
    dbg_log(DBG_LOG, "enable irq pin:%d enabled:%d\n", pin, enabled);

    if (pin >= GPIONUM)
        return RT_ERROR;

    if (enabled)
        gpio_int_enable(pin, gpio_dev.irq_desc[pin].mode, gpio_irq_dispatch);
    else
        gpio_int_disable(pin);

    return RT_EOK;
}

const static struct rt_pin_ops _pin_ops =
{
    _gpio_mode,
    _gpio_write,
    _gpio_read,
    _gpio_attach_irq,
    _gpio_dettach_irq,
    _gpio_irq_enable,
};

int rt_hw_gpio_init(void)
{
    rt_memset(&gpio_dev, 0, sizeof(gpio_dev));
    rt_device_pin_register(GPIO_DEV_NAME, &_pin_ops, RT_NULL);
    gpio_dev.parent = rt_device_find(GPIO_DEV_NAME);

    return RT_EOK;
}

int rt_hw_gpio_exit(void)
{
    /* unregister device */
    rt_device_unregister(gpio_dev.parent);
}

INIT_DEVICE_EXPORT(rt_hw_gpio_init);

#endif
