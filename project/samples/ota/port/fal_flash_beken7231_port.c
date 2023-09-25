/*
 * File      : fal_flash_stm32f2_port.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include <fal.h>

#ifdef RT_FLASH_PORT_DRIVER_BEKEN
#include "flash.h"
#include <flash_pub.h>
#include <string.h>

static int init(void)
{
    /* do nothing now */
}

static int read(long offset, uint8_t *buf, size_t size)
{
    flash_read((unsigned char *)buf, (unsigned long)size, beken_onchip_flash.addr + offset);
    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    flash_write((unsigned char *)buf, (unsigned long)size, beken_onchip_flash.addr + offset);

    return size;
}

static int erase(long offset, size_t size)
{
    unsigned int _size = size;
    uint32_t addr;

    /* Calculate the start address of the flash sector(4kbytes) */
    offset = offset & 0x00FFF000;

    addr = beken_onchip_flash.addr + offset;

    do{
        flash_ctrl(CMD_FLASH_ERASE_SECTOR, &addr);
        addr += 4096;
        if (_size < 4096)
            _size = 0;
        else
            _size -= 4096;

        rt_kprintf(".");
        rt_thread_delay(rt_tick_from_millisecond(5)); // delay 5 tick every 4K

    } while (_size);
    //rt_kprintf("\n");

    return addr - ( beken_onchip_flash.addr + offset); // return true erase size
}

/* beken_onchip dev */
const struct fal_flash_dev beken_onchip_flash = 
{
    RT_BK_FLASH_DEV_NAME, 
    RT_FLASH_BEKEN_START_ADDR, 
    RT_FLASH_BEKEN_SIZE, 
    RT_FLASH_BEKEN_SECTOR_SIZE,
    {init, read, write, erase} 
};
#endif /* RT_FLASH_PORT_DRIVER_BEKEN */