#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>

#define BEKEN_USING_FLASH_DISK

#ifdef BEKEN_USING_FLASH_DISK

#include "drv_flash.h"

#define FLASH_BASE      (0x100000)
#define SECTOR_SIZE     (4096)
#define DISK_SIZE       (1024*1024)

static struct rt_device flash_disk_device;

static rt_size_t flash_disk_read(rt_device_t dev,
                                 rt_off_t pos,
                                 void *buffer,
                                 rt_size_t size)
{
    beken_flash_read(FLASH_BASE + SECTOR_SIZE * pos, buffer, SECTOR_SIZE * size);
    return size;
}

static rt_size_t flash_disk_write(rt_device_t dev,
                                  rt_off_t pos,
                                  const void *buffer,
                                  rt_size_t size)
{
    int i = 0;

    for (i = 0; i < size; i++)
    {
        beken_flash_erase(FLASH_BASE + SECTOR_SIZE * (pos + i));
        beken_flash_write(FLASH_BASE + SECTOR_SIZE * (pos + i), buffer, SECTOR_SIZE);
    }

    return size;
}

static rt_err_t flash_disk_control(rt_device_t dev, int cmd, void *args)
{
    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry;

        geometry = (struct rt_device_blk_geometry *)args;
        if (geometry == RT_NULL) return -RT_ERROR;

        geometry->bytes_per_sector = SECTOR_SIZE;
        geometry->sector_count = DISK_SIZE / SECTOR_SIZE;
        geometry->block_size = SECTOR_SIZE;
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

static int rt_hw_flash_disk_init(void)
{
    memset(&flash_disk_device, 0, sizeof(flash_disk_device));

    flash_disk_device.type    = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
    flash_disk_device.ops     = &flash_disk_ops;
#else
    flash_disk_device.init    = RT_NULL;
    flash_disk_device.open    = RT_NULL;
    flash_disk_device.close   = RT_NULL;
    flash_disk_device.read    = flash_disk_read;
    flash_disk_device.write   = flash_disk_write;
    flash_disk_device.control = flash_disk_control;
#endif /* RT_USING_DEVICE_OPS */

    /* register device */
    return rt_device_register(&flash_disk_device, "disk0", \
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}
INIT_DEVICE_EXPORT(rt_hw_flash_disk_init);

#endif