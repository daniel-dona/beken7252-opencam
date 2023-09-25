#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>

#include "typedef.h"
#include "sys_config.h"
#include "bk_rtos_pub.h"

#ifdef BEKEN_USING_SPI_PSRAM

#if !CFG_USE_SPI_MST_PSRAM
#error "spi psram need 'CFG_USE_SPI_MST_PSRAM ENABLE' !"
#endif

#include "spi_psram.h"

static struct rt_device _g_spi_psram;

static rt_err_t rt_spi_psram_init(rt_device_t dev)
{
    if(spi_psram_init())
    {
        rt_kprintf("psram_init failed\r\n");
        return RT_ERROR;
    }
    spi_psram_read_id(NULL);

    return RT_EOK;
}

static rt_err_t rt_spi_psram_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_spi_psram_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t rt_spi_psram_read(rt_device_t dev, rt_off_t pos,
                                   void *buffer, rt_size_t size)
{
    int ret;
    ret = spi_psram_read(pos, buffer, size);

    return ret;
}
static rt_size_t rt_spi_psram_write(rt_device_t dev, rt_off_t pos,
                                   const void *buffer, rt_size_t size)
{
    int ret;
    ret = spi_psram_write(pos, (uint8_t*)buffer, size);

    return ret;
}

#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops spi_psram_ops =
{
    rt_spi_psram_init,
    rt_spi_psram_open,
    rt_spi_psram_close,
    rt_spi_psram_read,
    rt_spi_psram_write,
    RT_NULL
};
#endif /* RT_USING_DEVICE_OPS */

int rt_spi_psram_hw_init(void)
{
    struct rt_device *device = &_g_spi_psram;

    device->type = RT_Device_Class_Miscellaneous;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    device->user_data   = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops = &spi_psram_ops;
#else
    device->control = RT_NULL;
    device->init    = rt_spi_psram_init;
    device->open    = rt_spi_psram_open;
    device->close   = rt_spi_psram_read;
    device->read    = rt_spi_psram_read;
    device->write   = rt_spi_psram_write;
#endif /* RT_USING_DEVICE_OPS */

    /* register the device */
    rt_device_register(device, "spi_psram", 
        RT_DEVICE_FLAG_STANDALONE | RT_DEVICE_FLAG_RDWR);

    //rt_device_init(device);

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_spi_psram_hw_init);

#endif //BEKEN_USING_SPI_PSRAM