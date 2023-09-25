/*
 * File      : rt_ota_cfg.h
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-23     armink       the first version
 */

#ifndef _RT_OTA_CFG_H_
#define _RT_OTA_CFG_H_

#include <board.h>

/* ========================== Debug Configuration =========================== */
#define RT_OTA_DEBUG                   1

/* ===================== Flash device Configuration ========================= */
/* enable stm32f2 onchip flash driver sample */
#define RT_OTA_FLASH_PORT_DRIVER_STM32F2
/* enable SFUD flash driver sample */
#define RT_OTA_FLASH_PORT_DRIVER_SFUD

extern const struct rt_ota_flash_dev stm32f2_onchip_flash;
extern const struct rt_ota_flash_dev nor_flash0;

/* flash device table */
#define RT_OTA_FLASH_DEV_TABLE                                       \
{                                                                    \
    &stm32f2_onchip_flash,                                           \
    &nor_flash0,                                                     \
}

/* ====================== Partition Configuration ========================== */
/* has partition table configuration will defined 1 when running on bootloader */
#define RT_OTA_PART_HAS_TABLE_CFG      1

/* the flash device name which saved bootloader */
#define RT_OTA_BL_FLASH_DEV_NAME       "stm32_onchip"
/* bootloader partition name */
#define RT_OTA_BL_PART_NAME            "bl"
/* bootloader partition offset address */
#define RT_OTA_BL_PART_OFFSET          0
/* bootloader partition length */
#define RT_OTA_BL_PART_LEN             (USER_APP_ENTRY - FLASH_BASE)

#if RT_OTA_PART_HAS_TABLE_CFG
/* partition table */
#define RT_OTA_PART_TABLE                                            \
{                                                                    \
    {RT_OTA_PART_MAGIC_WROD, RT_OTA_BL_PART_NAME, RT_OTA_BL_FLASH_DEV_NAME, RT_OTA_BL_PART_OFFSET, RT_OTA_BL_PART_LEN, 0}, \
    {RT_OTA_PART_MAGIC_WROD, "app", RT_OTA_BL_FLASH_DEV_NAME, 64*1024, 704*1024, 0},  \
    {RT_OTA_PART_MAGIC_WROD, RT_OTA_DL_PART_NAME, "norflash0", 1024*1024, 1024*1024, 0},
}
#endif /* RT_OTA_PART_HAS_TABLE_CFG */

#endif /* _RT_OTA_CFG_H_ */
