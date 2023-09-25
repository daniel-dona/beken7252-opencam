/*
 * File      : rt_ota_flash_beken7231_port.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 * 2018-02-03     Murphy       Adapted beken7231 board feature
 * 2018-02-06     Murphy       Fix addr problem, add flash control
 */
#include <stdint.h>

#include <rt_ota.h>
#include <rt_ota_flash_dev.h>

#ifdef RT_OTA_FLASH_PORT_DRIVER_BEKEN
#include "flash.h"
#include <flash_pub.h>
#include <string.h>

static int read(uint32_t offset, uint8_t *buf, size_t size)
{
    flash_read((unsigned char *)buf, (unsigned long)size, beken_onchip_flash.addr + offset);
    return size;
}

static int write(uint32_t offset, const uint8_t *buf, size_t size)
{

    int param = 1;
    flash_ctrl(CMD_FLASH_SET_WSR, &param);
    flash_ctrl(CMD_FLASH_WRITE_ENABLE, NULL);

    flash_write((unsigned char *)buf, (unsigned long)size, beken_onchip_flash.addr + offset);


    flash_ctrl(CMD_FLASH_WRITE_DISABLE, NULL);

    return size;
}

/* 
 * size param: The flash size of you want to erase in bytes.
 * return: Returns the size of the actual erase.
 */
static int erase(uint32_t offset, size_t size)
{
    unsigned int _size = size;
    uint32_t addr;
    
    /* start erase */


    int param = 1;
    flash_ctrl(CMD_FLASH_SET_WSR, &param);    
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

    } while (_size);

    return addr - ( beken_onchip_flash.addr + offset); // return true erase size
}

/* beken_onchip dev */
const struct rt_ota_flash_dev beken_onchip_flash = { RT_OTA_BL_FLASH_DEV_NAME, RT_OTA_FLASH_START_ADDR, RT_OTA_FLASH_SIZE, {read, write, erase} };
#endif /* RT_OTA_FLASH_PORT_DRIVER_BEKEN */
