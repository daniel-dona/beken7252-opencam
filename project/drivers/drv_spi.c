#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>

#include "typedef.h"
#include "spi_pub.h"
#include "drv_spi.h"
#include "sys_config.h"

#if ((CFG_USE_SPI_MASTER) &&(CFG_USE_SPI_SLAVE))

#define DBG_ENABLE
#define DBG_SECTION_NAME  "[SPI]:"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

struct bk_spi_dev
{
    struct rt_spi_bus *spi_bus;
};

static struct bk_spi_dev *spi_dev;

rt_err_t _spi_configure(struct rt_spi_device *dev, struct rt_spi_configuration *cfg)
{
    int result = RT_EOK;
    uint32_t mode;

    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    dbg_log(DBG_LOG, "data_width = %d \n", cfg->data_width);
    
    /* baudrate */
    dbg_log(DBG_LOG, "max_hz = %d,  mode:0x%02x\n", cfg->max_hz, cfg->mode);

    mode = 0;
    /* CPOL */
    if (cfg->mode & RT_SPI_CPOL)
    {
        mode |= BK_SPI_CPOL;
    }

    /* CPHA */
    if (cfg->mode & RT_SPI_CPHA)
    {
        mode |= BK_SPI_CPHA;
    }

    if (cfg->mode & RT_SPI_SLAVE)
    {
        bk_spi_slave_init(cfg->max_hz, mode);
    }
    else
    {
        bk_spi_master_init(cfg->max_hz, mode);
    }

    return RT_EOK;
}

rt_uint32_t _spi_xfer(struct rt_spi_device *dev, struct rt_spi_message *msg)
{
    rt_uint32_t master;
    struct rt_spi_bus *spi_bus = RT_NULL;
    struct rt_spi_configuration *cfg = RT_NULL;
    struct spi_message spi_msg;

    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(msg != RT_NULL);

    if(msg->length == 0)
        return 0;

    spi_bus = dev->bus;
    cfg = &dev->config;
    master = (cfg->mode & RT_SPI_SLAVE) ? 0 : 1;

    int length = 0;
    if(master)
    {
        if((msg->recv_buf) && msg->length)
        {
            spi_msg.send_buf = NULL;
            spi_msg.send_len = 0;
            
            spi_msg.recv_buf = msg->recv_buf;
            spi_msg.recv_len = msg->length;
        }
        else if((msg->send_buf) && msg->length)
        {
            spi_msg.send_buf = (UINT8*)msg->send_buf;
            spi_msg.send_len = msg->length;
            
            spi_msg.recv_buf = NULL;
            spi_msg.recv_len = 0;
        }
        else
        {
            return 0;
        }
        
        length = bk_spi_master_xfer(&spi_msg);
    }
    else
    {
        if((msg->recv_buf) && msg->length)
        {
            spi_msg.send_buf = NULL;
            spi_msg.send_len = 0;
            
            spi_msg.recv_buf = msg->recv_buf;
            spi_msg.recv_len = msg->length;
        }
        else if((msg->send_buf) && msg->length)
        {
            spi_msg.send_buf = (UINT8*)msg->send_buf;
            spi_msg.send_len = msg->length;
            
            spi_msg.recv_buf = NULL;
            spi_msg.recv_len = 0;
        }
        else
        {
            return 0;
        }
        
        length = bk_spi_slave_xfer(&spi_msg);
    }
  
    return length;
}

static struct rt_spi_ops spi_ops =
{
    .configure = _spi_configure,
    .xfer = _spi_xfer
};

int rt_hw_spi_bus_register(char *name)
{
    int result = RT_EOK;
    struct rt_spi_bus *spi_bus = RT_NULL;

    if (spi_dev)
    {
        return RT_EOK;
    }

    spi_dev = rt_malloc(sizeof(struct bk_spi_dev));
    if (!spi_dev)
    {
        rt_kprintf("[spi]:malloc memory for spi_dev failed\n");
        result = -RT_ENOMEM;
        goto _exit;
    }
    memset(spi_dev, 0, sizeof(struct bk_spi_dev));

    spi_bus = rt_malloc(sizeof(struct rt_spi_bus));
    if (!spi_bus)
    {
        rt_kprintf("[spi]:malloc memory for spi_bus failed\n");
        result = -RT_ENOMEM;
        goto _exit;
    }
    memset(spi_bus, 0, sizeof(struct rt_spi_bus));

    spi_bus->parent.user_data = spi_dev;
    rt_spi_bus_register(spi_bus, name, &spi_ops);
    
    return result;

_exit:
    if (spi_dev)
    {
        rt_free(spi_dev);
        spi_dev = RT_NULL;
    }

    if (spi_bus)
    {
        rt_free(spi_bus);
        spi_bus = RT_NULL;
    }

    return result;
}

static struct rt_spi_device *spi_device = RT_NULL;
int rt_hw_spi_device_init(void)
{
    int result = RT_EOK;

    rt_kprintf("[spi]:rt_hw_spi_device_init\n");
    
    if (spi_device)
    {
        return RT_EOK;
    }
    spi_device = rt_malloc(sizeof(struct rt_spi_device));
    if (!spi_device)
    {
        rt_kprintf("[spi]:malloc memory for spi_device failed\n");
        result = -RT_ENOMEM;
    }
    memset(spi_device, 0, sizeof(struct rt_spi_device));

    /* register spi bus */
    result = rt_hw_spi_bus_register("spi0");
    if (result != RT_EOK)
    {
        rt_kprintf("[spi]:register spi bus error : %d !!!\n", result);
        goto _exit;
    }

    /* attach cs */
    result = rt_spi_bus_attach_device(spi_device, "gspi", "spi0", NULL);
    if (result != RT_EOK)
    {
        rt_kprintf("[spi]:attach spi bus error : %d !!!\n", result);
        goto _exit;
    }

    rt_kprintf("[spi]:rt_hw_spi_device_init ok\n");
    return RT_EOK;

_exit:
    if (spi_device)
    {
        rt_free(spi_device);
        spi_device = RT_NULL;
    }

    return result;
}
INIT_PREV_EXPORT(rt_hw_spi_device_init);

#endif
