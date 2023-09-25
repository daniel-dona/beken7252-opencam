/*
 * File      : ymodem_beken.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-30     armink       the first version
 * 2018-02-27     Murphy       adapt to beken
 */

#include <rtthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <finsh.h>

#include <ymodem.h>
#include "drv_flash.h"
#include <fal.h>
#include "ymodem_beken.h"

static size_t update_file_total_size, update_file_cur_size;
static uint32_t crc32_checksum = 0;

static const struct fal_partition * dl_part = RT_NULL;

static enum rym_code ymodem_on_begin(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    char *file_name, *file_size;    

    /* calculate and store file size */
    file_name = (char *)&buf[0];
    file_size = (char *)&buf[rt_strlen(file_name) + 1];
    update_file_total_size = atol(file_size);

    update_file_cur_size = 0;

    /* Get download partition information and erase download partition data */
    if ((dl_part = fal_partition_find(RT_BK_DL_PART_NAME)) == RT_NULL)
    {
        log_e("Firmware download failed! Partition (%s) find error!", RT_BK_DL_PART_NAME);
        return RYM_CODE_CAN;
    }

    if (update_file_total_size > dl_part->len)
    {
        log_e("Firmware is too large! File size (%d), '%s' partition size (%d)", update_file_total_size, RT_BK_DL_PART_NAME, dl_part->len);
        return RYM_CODE_CAN;
    }

    /* erase DL section */
    if (fal_partition_erase_all(dl_part) < 0)
    {
        log_e("Firmware download failed! Partition (%s) erase error!", dl_part->name);
        return RYM_CODE_CAN;
    }

    return RYM_CODE_ACK;
}

static enum rym_code ymodem_on_data(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    /* write data of application to DL partition  */
    if (fal_partition_write(dl_part, update_file_cur_size, buf, len) < 0)
    {
        log_e("Firmware download failed! Partition (%s) write data error!", dl_part->name);
        return RYM_CODE_CAN;
    }

    update_file_cur_size += len;

    return RYM_CODE_ACK;
}

void update(uint8_t argc, char **argv)
{
    char new_char[2], c_file_size[11] = {0}, c_crc32_checksum[11] = {0};
    struct rym_ctx rctx;

    rt_kprintf("Waring: This operator will not recovery. If you want, press 'Y'.\n");
    new_char[0] = getchar();
    rt_kprintf("%c", new_char[0]);
    new_char[1] = getchar();
    rt_kprintf("%c", new_char[1]);
    if ((new_char[0] != 'y') && (new_char[0] != 'Y'))
    {
        goto __exit;
    }

    int parts_num;
    parts_num = fal_init();

    if (parts_num <= 0)
    {
        log_e("Initialize failed! Don't found the partition table.");
        return;
    }

    rt_kprintf("Please select the application firmware file and use Ymodem to send.\n");

    if (!rym_recv_on_device(&rctx, rt_console_get_device(), RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                            ymodem_on_begin, ymodem_on_data, NULL, RT_TICK_PER_SECOND))
    {
        rt_kprintf("Download firmware to flash OK.\r\n");
        rt_kprintf("System now will restart...\r\n");

        /* wait some time for terminal response finish */
        rt_thread_delay(rt_tick_from_millisecond(200));

        /* Reset the device, Start new firmware */
        extern void rt_hw_cpu_reset(void);
        rt_hw_cpu_reset();
        /* wait some time for terminal response finish */
        rt_thread_delay(rt_tick_from_millisecond(200));        
    }
    else
    {
        /* wait some time for terminal response finish */
        rt_thread_delay(RT_TICK_PER_SECOND);
        rt_kprintf("Update firmware fail.\n");
    }

__exit:
    return;
}
MSH_CMD_EXPORT(update, Update user application firmware);