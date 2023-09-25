/*
 * File      : rt_ota_flash_sfud_port.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include <rt_ota_flash_dev.h>

#ifdef RT_OTA_FLASH_PORT_DRIVER_SFUD
#include <sfud.h>

extern sfud_flash sfud_norflash0;

static int read(uint32_t offset, uint8_t *buf, size_t size)
{
    sfud_read(&sfud_norflash0, nor_flash0.addr + offset, size, buf);

    return size;
}

static int write(uint32_t offset, const uint8_t *buf, size_t size)
{
    if (sfud_write(&sfud_norflash0, nor_flash0.addr + offset, size, buf) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}

static int erase(uint32_t offset, size_t size)
{
    if (sfud_erase(&sfud_norflash0, nor_flash0.addr + offset, size) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}
const struct rt_ota_flash_dev nor_flash0 = { "norflash0", 0, 8*1024*1024, {read, write, erase} };
#endif /* RT_OTA_FLASH_PORT_DRIVER_SFUD */
