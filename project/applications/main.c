/*
 * File      : main.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2017, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-5-30      Bernard      the first version
 */

#include "rtthread.h"
#include <dfs.h>
#include <dfs_fs.h>
#include "player.h"

#include "include.h"
#include "driver_pub.h"
#include "func_pub.h"
#include "app.h"
#include "ate_app.h"
#include "shell.h"
#include <fal.h>
#include "rt_ota.h"

#include "format_m4a.h"
#include "codec_helixmp3.h"
#include "codec_opencore_amr.h"
#include "player_system.h"

#ifdef PKG_USING_LITTLEFS
#include "BkDriverFlash.h"
#include "drv_flash.h"
#endif /* PKG_USING_LITTLEFS */

static int wlan_app_init(void);

extern const struct romfs_dirent romfs_root;
#define DFS_ROMFS_ROOT          (&romfs_root) 
extern rt_err_t rt_audio_codec_hw_init(void);
extern int player_system_init(void);
extern void user_main( beken_thread_arg_t args );

extern int rt_hw_flash_disk_readonly_init(const char *name, uint32_t base, uint32_t sector_size, uint32_t capacity);

int main(int argc, char **argv)
{
    /* mount ROMFS as root directory */
    if (dfs_mount(RT_NULL, "/", "rom", 0, (const void *)DFS_ROMFS_ROOT) == 0)
    {
        rt_kprintf("ROMFS File System initialized!\n");
    }
    else
    {
        rt_kprintf("ROMFS File System initialized Failed!\n");
    }

#if 1
    /* mount sd card fat partition 1 as root directory */
    if(dfs_mount("sd0", "/sd", "elm", 0, 0) == 0)
        rt_kprintf("SD File System initialized!\n");
    else
        rt_kprintf("SD File System initialzation failed!\n");
#endif

#if 0
    const struct fal_partition *dl_part = RT_NULL;

    if ((dl_part = fal_partition_find(RT_BK_DL_PART_NAME)) != RT_NULL)
    {
        /* dump current firmware version. */
        rt_kprintf("current image name: %s, version: %s, timestamp: %d \n", rt_ota_get_fw_dest_part_name(dl_part), rt_ota_get_fw_version(dl_part), rt_ota_get_fw_timestamp(dl_part));

        rt_hw_flash_disk_readonly_init("flash0", dl_part->offset + 96, 512, dl_part->len - 1024);
        /* mount sd card fat partition 1 as root directory */
        if(dfs_mount("flash0", "/flash0", "elm", 0, 0) == 0)
            rt_kprintf("FLASH File System initialized!\n");
        else
            rt_kprintf("FLASH File System initialzation failed!\n");
    }
    else
    {
        rt_kprintf("not found %s partition \n", RT_BK_DL_PART_NAME);
    }

#endif

#ifdef PKG_USING_LITTLEFS
    {
        const char *lfs_part_name = "filesystem";
        const char *lfs_mount_path = "/flash0";
        struct rt_device *mtd_dev = RT_NULL;

        mtd_dev = fal_mtd_nor_device_create(lfs_part_name);
        if (!mtd_dev)
        {
            rt_kprintf("Can't create a mtd device on [%s] partition.\n", lfs_part_name);
        }
        else
        {
            bk_flash_enable_security(FLASH_PROTECT_NONE);
            /* 挂载 littlefs */
            if (dfs_mount(lfs_part_name, lfs_mount_path, "lfs", 0, 0) == 0)
            {
                rt_kprintf("LFS Filesystem initialized! %s ==> %s\n", lfs_part_name, lfs_mount_path);
            }
            else
            {
                /* 格式化文件系统 */
                dfs_mkfs("lfs", lfs_part_name);
                rt_kprintf("mkfs [%s] LFS success\n", lfs_part_name);
                /* 挂载 littlefs */
                if (dfs_mount(lfs_part_name, lfs_mount_path, "lfs", 0, 0) == 0)
                {
                    rt_kprintf("LFS Filesystem initialized! %s ==> %s\n", lfs_part_name, lfs_mount_path);
                }
                else
                {
                    rt_kprintf("Failed to initialize LFS filesystem! %s ==> %s\n", lfs_part_name, lfs_mount_path);
                }
            }
        }
    }
#endif /* PKG_USING_LITTLEFS */

#ifdef PKG_USING_EASYFLASH
    extern int easyflash_init(void);
    easyflash_init();
#endif /* PKG_USING_EASYFLASH */

    wlan_app_init();
	rt_audio_codec_hw_init();

    /* 注册Codec */ 
    player_codec_helixmp3_register(); 
    player_codec_beken_aac_register(); 
    player_codec_beken_m4a_register(); 
	player_codec_opencore_amr_register(); 

    
	
	/* 初始化 */ 
	player_system_init();
    audio_device_mic_open(); /*open mic device */

    #if ((CFG_USE_APP_DEMO_VIDEO_TRANSFER) || (CFG_SUPPORT_TIANZHIHENG_DRONE))
    // user_main(NULL);
    #endif

    return 0;
}

#ifdef BEKEN_USING_WLAN

extern void ate_app_init(void);
extern void ate_start(void);

static int wlan_app_init(void)
{
	/* init ate mode check. */
	ate_app_init();

	if (get_ate_mode_state())
	{
		rt_kprintf("\r\n\r\nEnter automatic test mode...\r\n\r\n");

		finsh_set_echo(0);
		finsh_set_prompt("#");

		ate_start();
	}
	else
	{
		rt_kprintf("Enter normal mode...\r\n\r\n");
		app_start();

		//user_app_start();
	}

	return 0;
}

#endif
