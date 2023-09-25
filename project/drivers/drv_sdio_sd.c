#include <rtthread.h>
#include <rthw.h>

#include "drv_sdio_sd.h"
#include "include.h"
#include "sys_rtos.h"
#include "bk_rtos_pub.h"
#include "sdio_driver.h"

#include "sdcard.h"
#include "sdcard_pub.h"

#include "drv_model_pub.h"
#include "sys_ctrl_pub.h"
#include "mem_pub.h"
#include "arch.h"
#include <string.h>

extern SDIO_Error sdcard_initialize(void);
extern SDIO_Error sdcard_read_single_block(UINT8 *readbuff, UINT32 readaddr, UINT32 blocksize);
extern SDIO_Error sdcard_write_multi_block(UINT8 *writebuff, UINT32 writeaddr, UINT32 blocksize);

/* RT-Thread Device Driver Interface */
static struct rt_device sdcard_device;
static struct rt_mutex sdcard_mutex;
static rt_err_t rt_sdcard_init(rt_device_t dev)
{
    rt_mutex_init(&sdcard_mutex, "sdcard", RT_IPC_FLAG_PRIO);
    return RT_EOK;
}

static rt_err_t rt_sdcard_open(rt_device_t dev, rt_uint16_t oflag)
{
    rt_err_t ret;
    rt_mutex_take(&sdcard_mutex, RT_WAITING_FOREVER);
    ret = sdcard_open(0);
    rt_mutex_release(&sdcard_mutex);
    return ret;
}

static rt_err_t rt_sdcard_close(rt_device_t dev)
{
    rt_err_t ret;
    rt_mutex_take(&sdcard_mutex, RT_WAITING_FOREVER);
    ret = sdcard_close();
    rt_mutex_release(&sdcard_mutex);
    return ret;
}

//static uint32_t sdio_buffer[SD_DEFAULT_BLOCK_SIZE/sizeof(uint32_t)];
static rt_size_t rt_sdcard_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    rt_uint32_t result = RT_EOK;

    UINT32 start_blk_addr;
    UINT8  read_blk_numb, numb;
    UINT8* read_data_buf;

    rt_mutex_take(&sdcard_mutex, RT_WAITING_FOREVER);
    // check operate parameter
    start_blk_addr = pos;
    read_blk_numb = size;
    read_data_buf = (UINT8*)buffer;
    
    for(numb=0; numb<read_blk_numb; numb++)
    {
        result = sdcard_read_single_block(read_data_buf, start_blk_addr, 
                    SD_DEFAULT_BLOCK_SIZE);
        if(result!=SD_OK)
        {
            rt_kprintf("sdcard_read err:%d, curblk:0x%x\r\n",result, start_blk_addr);
            size = 0;
            goto exit;
        }

        start_blk_addr++;
        read_data_buf += SD_DEFAULT_BLOCK_SIZE;
    }
    
exit:
    rt_mutex_release(&sdcard_mutex);

    return size;
}

static rt_size_t rt_sdcard_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    UINT32 start_blk_addr;
    UINT32 writesize;
    UINT8* write_data_buf;	

    //	rt_kprintf("===rt write start addr=%x,size=%x====\r\n",pos,size);
    rt_mutex_take(&sdcard_mutex, RT_WAITING_FOREVER);
    start_blk_addr = pos;
    write_data_buf = (rt_uint8_t *)buffer;
    writesize = size * SD_DEFAULT_BLOCK_SIZE;
    if(SD_OK != sdcard_write_multi_block(write_data_buf,start_blk_addr,writesize))
        size = 0;
    rt_mutex_release(&sdcard_mutex);

    return size;
}

static rt_err_t rt_sdcard_control(rt_device_t dev, int cmd, void *args)
{
    RT_ASSERT(dev != RT_NULL);

    rt_kprintf("[SDIO]:rt_sdcard_control, CMD=%d\n", cmd);
    
    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry;
        SDCARD_S card_info;

        memset(&card_info, 0, sizeof(SDCARD_S));
        sdcard_get_card_info(&card_info);
        
        if ((card_info.total_block == 0) || (card_info.block_size == 0)) {
            return -RT_ERROR;
        }

        geometry = (struct rt_device_blk_geometry *)args;
        
        if (geometry == RT_NULL) 
            return -RT_ERROR;

        geometry->bytes_per_sector = SD_DEFAULT_BLOCK_SIZE;
        geometry->block_size = card_info.block_size;
        geometry->sector_count = card_info.total_block;
    }

    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops sdcard_ops =
{
    rt_sdcard_init,
    rt_sdcard_open,
    rt_sdcard_close,
    rt_sdcard_read,
    rt_sdcard_write,
    rt_sdcard_control
};
#endif /* RT_USING_DEVICE_OPS */

int rt_hw_sdcard_init(void)
{
    /*this 'sdcard_init' has called in dd.c */
    //sdcard_init();
    
    //sdcard_initialize();

	/* register sdcard device */
	sdcard_device.type  = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
    sdcard_device.ops   = &sdcard_ops;
#else
	sdcard_device.init 	= rt_sdcard_init;
	sdcard_device.open 	= rt_sdcard_open;
	sdcard_device.close = rt_sdcard_close;
	sdcard_device.read 	= rt_sdcard_read;
	sdcard_device.write = rt_sdcard_write;
	sdcard_device.control = rt_sdcard_control;
#endif /* RT_USING_DEVICE_OPS */

	/* no private */
	sdcard_device.user_data = RT_NULL;
    
	rt_device_register(&sdcard_device, "sd0",
			RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE);

    return RT_EOK;
}

INIT_DEVICE_EXPORT(rt_hw_sdcard_init);
