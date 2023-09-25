/*
 * File      : rt_ota_flash_temp.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include <rt_ota_flash_dev.h>

static int read(uint32_t offset, uint8_t *buf, size_t size)
{
    /**
     * add your code under here
     * ======================
     * flash lock
     * ----------------------
     * flash operate
     * ----------------------
     * flash unlock
     * ----------------------
     */

    return size;
}

static int write(uint32_t offset, const uint8_t *buf, size_t size)
{
    /**
     * add your code under here
     * ======================
     * flash lock
     * ----------------------
     * flash operate
     * ----------------------
     * flash unlock
     * ----------------------
     */

    return size;
}

static int erase(uint32_t offset, size_t size)
{
    /**
     * add your code under here
     * ======================
     * flash lock
     * ----------------------
     * flash operate
     * ----------------------
     * flash unlock
     * ----------------------
     */

    return size;
}

const struct rt_ota_flash_dev temp_flash = { "temp", 0, 16*1024, {read, write, erase} };
