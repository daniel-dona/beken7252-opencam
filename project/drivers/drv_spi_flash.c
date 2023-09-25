#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>

#include "typedef.h"
#include "sys_config.h"
#include "bk_rtos_pub.h"

#ifdef BEKEN_USING_SPI_FLASH

#if !CFG_USE_SPI_MST_FLASH
#error "spi flash need 'CFG_USE_SPI_MST_FLASH ENABLE' "
#endif

#include "spi_flash.h"
#include "drv_spi_flash.h"

////////////////////////////////////////////////////////
static rt_err_t rt_spi_flash_init(rt_device_t dev)
{
    UINT32 uid;
    if(spi_flash_init())
    {
        rt_kprintf("spi_flash_init failed\r\n");
        return RT_ERROR;
    }
    
    uid = spi_flash_read_id();
    rt_kprintf("uid = 0x%06x\r\n", uid);

    return RT_EOK;
}

static rt_err_t rt_spi_flash_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_spi_flash_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t rt_spi_flash_read(rt_device_t dev, rt_off_t pos,
                                   void *buffer, rt_size_t size)
{
    int ret;
    ret = spi_flash_read(pos, size, buffer);

    return ret;
}
static rt_size_t rt_spi_flash_write(rt_device_t dev, rt_off_t pos,
                                   const void *buffer, rt_size_t size)
{
    int ret;
    ret = spi_flash_write(pos, size, (uint8_t*)buffer);

    return ret;
}

rt_err_t  rt_spi_flash_control(rt_device_t dev, int cmd, void *args)
{
    switch(cmd)
    {
        case BK_SPI_FLASH_ERASE_CMD:
        {
            BK_SPIFLASH_ERASE_PTR erase_ptr = (BK_SPIFLASH_ERASE_PTR)args;
            spi_flash_erase(erase_ptr->addr, erase_ptr->size);
            break;
        }

        case BK_SPI_FLASH_PROTECT_CMD:
        {
            spi_flash_protect();
            break;
        }
        
        case BK_SPI_FLASH_UNPROTECT_CMD:
        {
            spi_flash_unprotect();
            break;
        }
    }

    return RT_EOK;
}


#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops spi_flash_ops =
{
    rt_spi_flash_init,
    rt_spi_flash_open,
    rt_spi_flash_close,
    rt_spi_flash_read,
    rt_spi_flash_write,
    rt_spi_flash_control
};
#endif /* RT_USING_DEVICE_OPS */

static struct rt_device _g_spi_flash;

int rt_spi_flash_hw_init(void)
{
    struct rt_device *device = &_g_spi_flash;

    device->type = RT_Device_Class_Miscellaneous;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    device->user_data   = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops = &spi_flash_ops;
#else
    device->control = rt_spi_flash_control;
    device->init    = rt_spi_flash_init;
    device->open    = rt_spi_flash_open;
    device->close   = rt_spi_flash_read;
    device->read    = rt_spi_flash_read;
    device->write   = rt_spi_flash_write;
#endif /* RT_USING_DEVICE_OPS */

    /* register the device */
    rt_device_register(device, "spi_flash", 
        RT_DEVICE_FLAG_STANDALONE | RT_DEVICE_FLAG_RDWR);

    //rt_device_init(device);

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_spi_flash_hw_init);



#endif  // BEKEN_USING_SPI_FLASH
