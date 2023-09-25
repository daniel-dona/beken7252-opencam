/*
 * File      : http_client_ota.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-22     Murphy       the first version
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <rtthread.h>
#include <finsh.h>

#include "webclient.h"
#include <fal.h>
#include <rt_ota.h>

#include "BkDriverFlash.h"
#include "drv_flash.h"

#define DBG_ENABLE
#define DBG_SECTION_NAME "httpota"
#define DBG_LEVEL        DBG_LOG
// #define DBG_LEVEL        DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#define HTTP_OTA_BUFF_LEN 4096
#define HTTP_OTA_DL_DELAY (10 * RT_TICK_PER_SECOND)

#define HTTP_OTA_URL "http://192.168.10.135:80/rtthread.rbl"

static void print_progress(size_t cur_size, size_t total_size)
{
    static unsigned char progress_sign[100 + 1];
    uint8_t i, per = cur_size * 100 / total_size;

    if (per > 100)
    {
        per = 100;
    }

    for (i = 0; i < 100; i++)
    {
        if (i < per)
        {
            progress_sign[i] = '=';
        }
        else if (per == i)
        {
            progress_sign[i] = '>';
        }
        else
        {
            progress_sign[i] = ' ';
        }
    }

    progress_sign[sizeof(progress_sign) - 1] = '\0';

    rt_kprintf("\r" "\033[36;22m" "Download: [%s] %d%% " "\033[0m", progress_sign, per);
}

static int http_ota_fw_download(const char *uri)
{
    int res, length, total_length = 0;
    rt_uint8_t need_reboot = 0;
    int resp_status = (-1);

    rt_uint8_t *buffer_read = RT_NULL;
    rt_uint8_t *buffer_swap = RT_NULL;

    struct webclient_session *session = RT_NULL;
    const struct fal_partition *dl_part = RT_NULL;
    const struct fal_partition *app_part = RT_NULL;
    const struct fal_partition *desc_part = RT_NULL;

    if ((app_part = fal_partition_find(RT_BK_APP_NAME)) != RT_NULL)
    {
        /* dump current firmware version. */
        rt_kprintf("current firmware name: %s, version: %s, timestamp: %d \n", rt_ota_get_fw_dest_part_name(app_part), rt_ota_get_fw_version(app_part), rt_ota_get_fw_timestamp(app_part));
    }
    else
    {
        rt_kprintf("not found %s partition \n", RT_BK_APP_NAME);
    }

    session = webclient_session_create(1024); 
    if (!session)
    {
        LOG_E("open uri failed.");
        goto __exit;
    }

    resp_status = webclient_get(session, uri);
    if (resp_status == 304)
    {
        LOG_E("Firmware download failed! Server http response : 304 not modified!");
        goto __exit;
    }

    if (resp_status != 200)
    {
        LOG_E("Firmware download failed! Server http response : %d !", resp_status);
        goto __exit;
    }

    if (session->content_length == 0)
    {
        LOG_I("Request file size is 0!");
        goto __exit;
    }

    /* Get download partition information and erase download partition data */
    if ((dl_part = fal_partition_find(RT_BK_DL_PART_NAME)) == RT_NULL)
    {
        LOG_E("Firmware download failed! Partition (%s) find error!", RT_BK_DL_PART_NAME);
        goto __exit;
    }

    // Check if the OTA file is too large
    if (session->content_length > dl_part->len)
    {
        LOG_E("Firmware download failed! OTA file is too big!");
        goto __exit;
    }

    //dump
    rt_kprintf("dl_part->name  : %s\n", dl_part->name);
    rt_kprintf("dl_part->flash : %s\n", dl_part->flash_name);
    rt_kprintf("dl_part->offset: 0x%08X\n", dl_part->offset);
    rt_kprintf("dl_part->len   : %d\n", dl_part->len);

    LOG_I("OTA file size is (%d)", session->content_length);

    buffer_read = web_malloc(HTTP_OTA_BUFF_LEN);
    buffer_swap = web_malloc(HTTP_OTA_BUFF_LEN);
    if ((buffer_read == RT_NULL) || (buffer_swap == RT_NULL))
    {
        LOG_E("No memory for http ota!");
        goto __exit;
    }

    do
    {
        uint32_t nw;
        uint32_t page_pos;

        nw = HTTP_OTA_BUFF_LEN;
        if ((nw + total_length) > session->content_length)
        {
            nw = session->content_length - total_length;
            //log_i("last page, content_length:%d, total_length:%d, nw:%d\n", session->content_length, total_length, nw);
        }

        /* read */
        page_pos = 0;
        do
        {
            length = webclient_read(session, buffer_read + page_pos, nw - page_pos);
            if (length <= 0)
            {
                log_e("read error: %d, content_pos:%d, page_pos:%d\n", length, total_length, page_pos);
                break;
            }
            //LOG_I("webclient_read %d\n", length);
            page_pos += length;
        }
        while (page_pos < nw);   /* read */

        if (page_pos != nw)
        {
            log_e("page_pos error, %d:%d\n", page_pos, nw);
            goto __exit;
        }

        if (total_length == 0)
        {
            if ((page_pos >= 96) && (rt_memcmp(buffer_read, "RBL", 4) == 0))
            {
                // Determine if this file is an RBL file. if not, do not process it.
                char desc_part_name[16] = {0}; 

                // Gets the original size of the OTA file
                rt_uint32_t raw_size = (buffer_read[84] + (buffer_read[85] << 8) + (buffer_read[86] << 16) + (buffer_read[87] << 24));
                log_i("OTA file raw size %d bytes.", raw_size); 

                // Gets the describe partition name of the OTA file
                rt_strncpy(desc_part_name, &buffer_read[12], 16);

                /* dump new firmware info. */
                {
                    const char *version;
                    uint32_t *timestamp;

                    version = (const char *)&buffer_read[28];
                    timestamp = (uint32_t *)&buffer_read[8];

                    LOG_I("OTA file describe partition name %s, version: %s, timestamp: %d.\n", desc_part_name, version, *timestamp);
                }

                // If a partition exists in our partition table, check described partition size
                desc_part = fal_partition_find((const char *)desc_part_name);
                if (desc_part != RT_NULL)
                {
                    // Check the OTA raw size is larger than the size of the described partition
                    if (raw_size > (desc_part->len - 96))
                    {
                        LOG_E("Firmware download failed! The firmware raw size is too big!");
                        goto __exit;
                    }
                }
            }

            log_i("FLASH_PROTECT_HALF.\n");
            bk_flash_enable_security(FLASH_PROTECT_HALF); // half or custom
        }

        res = fal_partition_read(dl_part, total_length, buffer_swap, nw);
        if (res < 0)
        {
            LOG_E("rt_ota_partition_read failed! res:%d, postion:%d, len:%d", res, total_length, nw);
            goto __exit;
        }

        if (memcmp(buffer_read, buffer_swap, nw) != 0)
        {
            res = fal_partition_erase(dl_part, total_length, nw);
            if (res < 0)
            {
                LOG_E("rt_ota_partition_erase failed! res:%d, postion:%d, len:%d", res, total_length, nw);
                goto __exit;
            }

            res = fal_partition_write(dl_part, total_length, buffer_read, nw);
            if (res < 0)
            {
                LOG_E("Firmware download failed! Partition (%s) write data error!", dl_part->name);
                goto __exit;
            }

#if 1
            /* read back for verify. */
            res = fal_partition_read(dl_part, total_length, buffer_swap, nw);
            if (res < 0)
            {
                LOG_E("rt_ota_partition_read failed! res:%d, postion:%d, len:%d", res, total_length, nw);
                goto __exit;
            }

            /* verify */
            if (memcmp(buffer_read, buffer_swap, nw) != 0)
            {
                LOG_E("verify failed! postion:%d, len:%d", total_length, nw);
                goto __exit;
            }
#endif
        }
        total_length += nw;
        print_progress(total_length, session->content_length);
    }
    while (total_length != session->content_length);

    if (total_length == session->content_length)
    {
        /* Check the download partition to verify the partition integrity */
        if (rt_ota_part_fw_verify(dl_part) < 0)
        {
            LOG_E("Firmware download failed! Partition (%s) header and body verify failed!", RT_BK_DL_PART_NAME);
            goto __exit;
        }
        // rt_thread_delay(rt_tick_from_millisecond(5));
        need_reboot = 1;

    }

__exit:
    if (session != RT_NULL)
        webclient_close(session);
    if (buffer_read != RT_NULL)
        web_free(buffer_read);
    if (buffer_swap != RT_NULL)
        web_free(buffer_swap);

    log_i("FLASH_UNPROTECT_LAST_BLOCK.\n");
    bk_flash_enable_security(FLASH_UNPROTECT_LAST_BLOCK); // last or custom

    /* Reset the device, Start new firmware */
    if(need_reboot)
    {
        extern void rt_hw_cpu_reset(void);
        rt_hw_cpu_reset();
    }

    return 0;
}

void http_ota(uint8_t argc, char **argv)
{
    int parts_num;
    parts_num = fal_init();

    if (parts_num <= 0)
    {
        LOG_E("Initialize failed! Don't found the partition table.");
        return;
    }
    if (argc < 2)
    {
        rt_kprintf("using uri: " HTTP_OTA_URL "\n");
        http_ota_fw_download(HTTP_OTA_URL);
    }
    else
    {
        http_ota_fw_download(argv[1]);
    }
}
/**
 * msh />http_ota [url]
*/
MSH_CMD_EXPORT(http_ota, OTA by http client: http_ota [url]);