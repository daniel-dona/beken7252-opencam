#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>

#include "drv_flash.h"

#ifdef BEKEN_USING_FLASH

struct flash_disk_readonly
{
    struct rt_device parent;
    uint32_t base;
    uint32_t sector_size;
    uint32_t capacity;
};
static struct flash_disk_readonly flash_disk_device;

static rt_size_t flash_disk_read(rt_device_t dev,
                                 rt_off_t pos,
                                 void *buffer,
                                 rt_size_t size)
{
    struct flash_disk_readonly *disk = (struct flash_disk_readonly *)dev;

    uint32_t sector_count = disk->capacity / disk->sector_size;

    if( pos > (sector_count - 1))
    {
        rt_kprintf("%s:%d pos:%d out of range:%d!\n", __FUNCTION__, __LINE__, pos, sector_count);
        return 0;
    }

    if ( (pos + size) > sector_count )
    {
        size = sector_count - pos;
    }

    beken_flash_read(disk->base + disk->sector_size * pos, buffer, disk->sector_size * size);

    return size;
}

static rt_size_t flash_disk_write(rt_device_t dev,
                                  rt_off_t pos,
                                  const void *buffer,
                                  rt_size_t size)
{
    return 0;
}

static rt_err_t flash_disk_control(rt_device_t dev, int cmd, void *args)
{
    struct flash_disk_readonly *disk = (struct flash_disk_readonly *)dev;

    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry;

        geometry = (struct rt_device_blk_geometry *)args;
        if (geometry == RT_NULL) return -RT_ERROR;

        geometry->bytes_per_sector = disk->sector_size;
        geometry->sector_count = disk->capacity / disk->sector_size;
        geometry->block_size = disk->sector_size;
    }
    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops flash_disk_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    flash_disk_read,
    flash_disk_write,
    flash_disk_control
};
#endif /* RT_USING_DEVICE_OPS */

int rt_hw_flash_disk_readonly_init(const char *name, uint32_t base, uint32_t sector_size, uint32_t capacity)
{
    struct flash_disk_readonly *disk;
    struct rt_device *device;

    disk = &flash_disk_device;
    device = (struct rt_device *)&flash_disk_device.parent;

    disk->base = base;
    disk->sector_size = sector_size;
    disk->capacity = capacity;

    device->type    = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
    device->ops     = &flash_disk_ops;
#else
    device->init    = RT_NULL;
    device->open    = RT_NULL;
    device->close   = RT_NULL;
    device->read    = flash_disk_read;
    device->write   = flash_disk_write;
    device->control = flash_disk_control;
#endif /* RT_USING_DEVICE_OPS */

    /* register device */
    return rt_device_register(device, name, \
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}

#endif /* BEKEN_USING_FLASH */
