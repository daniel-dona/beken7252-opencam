/*
 * File      : rt_ota_cfg.h
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-23     armink       the first version
 * 2018-02-03     Myrphy       Adapted beken7231 board feature
 */
/*
 * Note:
 * Beken7231 program code is stored in ROM, the address range is 0x0-0x200000(total size 2Mbytes).
 * Code storage features on flash: Add 2 bytes of checksum every 32 bytes using crc16.
 * Address translation: flash storage addr is physical address; CPU execution address is logical address.
 *                      [physical_addr = logical_addr/32*34]
 *
 * Flash Table: <Logical address>   <Physical address>
*/

#ifndef _RT_OTA_CFG_H_
#define _RT_OTA_CFG_H_

#include <board.h>

#define RT_OTA_MALLOC                  rt_malloc
#define RT_OTA_REALLOC                 rt_realloc
#define RT_OTA_CALLOC                  rt_calloc
#define RT_OTA_FREE                    rt_free

#define RT_OTA_FLASH_PORT_DRIVER_BEKEN
#define RT_OTA_FLASH_PORT_DRIVER_BEKEN_CRC

/* ========================== Debug Configuration =========================== */
#define RT_OTA_DEBUG                   1

/* ===================== Flash device Configuration ========================= */
extern const struct rt_ota_flash_dev beken_onchip_flash;
extern const struct rt_ota_flash_dev beken_onchip_flash_crc;

/* flash device table */
#define RT_OTA_FLASH_DEV_TABLE                                       \
{                                                                    \
    &beken_onchip_flash,                                             \
    &beken_onchip_flash_crc,                                         \
}

/* ====================== Partition Configuration ========================== */
/* has partition table configuration will defined 1 when running on bootloader */
#define RT_OTA_PART_HAS_TABLE_CFG      1

/* flash start addr */
#define RT_OTA_FLASH_START_ADDR        0x00000000
#define RT_OTA_FLASH_SIZE              (4 * 1024 * 1024)               // unit: bytes, total 2Mbytes

/* the flash device name which saved bootloader */
#define RT_OTA_BL_FLASH_DEV_NAME       "beken_onchip"
#define RT_OTA_BL_FLASH_DEV_CRC_NAME   "beken_onchip_crc"

/* bootloader partition name */
#define RT_OTA_BL_PART_NAME            "bootloader"

#define RT_PARAM1_PART_NAME            "param1"
#define RT_PARAM1_PART_ADDR            0x3EB000
#define RT_PARAM1_PART_LEN             (16 * 1024)

#define RT_PARAM2_PART_NAME            "param2"
#define RT_PARAM2_PART_ADDR            0x3EF000
#define RT_PARAM2_PART_LEN             (16 * 1024)

#define RT_PARAM3_PART_NAME            "param3"
#define RT_PARAM3_PART_ADDR            0x3F3000
#define RT_PARAM3_PART_LEN             (16 * 1024)

#define RT_PARAM4_PART_NAME            "param4"
#define RT_PARAM4_PART_ADDR            0x3F7000
#define RT_PARAM4_PART_LEN             (16 * 1024)

/* bootloader partition offset address */
#define RT_OTA_BL_PART_ADDR          0x00
#define RT_OTA_BL_PART_OFFSET        RT_OTA_BL_PART_ADDR
/* bootloader partition length */
#define RT_OTA_BL_PART_LEN             (64 * 1024)

#define RT_OTA_APP_NAME                "app"
#define RT_OTA_APP_PART_ADDR           (RT_OTA_BL_PART_ADDR + RT_OTA_BL_PART_LEN)
#define RT_OTA_APP_PART_LEN            (1904/68*64*1024 - 96)
#define RT_OTA_APP_PART_END_ADDR       (RT_OTA_APP_PART_ADDR + RT_OTA_APP_PART_LEN)

#define RT_OTA_DL_PART_ADDR            (0x1ED000)
#define RT_OTA_DL_PART_LEN             (1496*1024 - 96)
#define RT_OTA_DL_PART_END_ADDR        (RT_OTA_DL_PART_ADDR + RT_OTA_DL_PART_LEN)

#define RT_OTA_ROMFS_PART_NAME         "romfs"
#define RT_OTA_ROMFS_PART_ADDR         (0x363000/68*64)
#define RT_OTA_ROMFS_PART_LEN          (544/68*64*1024 - 96)
#define RT_OTA_ROMFS_PART_END_ADDR     (RT_OTA_ROMFS_PART_ADDR + RT_OTA_ROMFS_PART_LEN)

/* Preprocess */
/* bootloader_end_addr < app_start_addr; bootloader_end_addr 4K aligned*/
#if (RT_OTA_BL_PART_LEN + RT_OTA_BL_PART_ADDR) > RT_OTA_APP_PART_ADDR
#error "boot end addr > app start addr!"
#endif

#if (RT_OTA_BL_PART_LEN + RT_OTA_BL_PART_ADDR)%(4 * 1024) != 0
#error "boot end addr is not 4K aligned!"
#endif

#if RT_OTA_PART_HAS_TABLE_CFG
/* partition table of rt_ota_partition struct, app table need crc_write mode */
#define RT_OTA_PART_TABLE                                            \
{                                                                    \
    {RT_OTA_PART_MAGIC_WROD, RT_OTA_BL_PART_NAME, RT_OTA_BL_FLASH_DEV_NAME, RT_OTA_BL_PART_ADDR, RT_OTA_BL_PART_LEN, 0}, \
    {RT_OTA_PART_MAGIC_WROD, RT_OTA_APP_NAME, RT_OTA_BL_FLASH_DEV_CRC_NAME, RT_OTA_APP_PART_ADDR, RT_OTA_APP_PART_LEN, 0}, \
    {RT_OTA_PART_MAGIC_WROD, RT_OTA_DL_PART_NAME, RT_OTA_BL_FLASH_DEV_NAME, RT_OTA_DL_PART_ADDR, RT_OTA_DL_PART_LEN, 0},   \
    {RT_OTA_PART_MAGIC_WROD, RT_OTA_ROMFS_PART_NAME, RT_OTA_BL_FLASH_DEV_CRC_NAME, RT_OTA_ROMFS_PART_ADDR, RT_OTA_ROMFS_PART_LEN, 0},   \
    {RT_OTA_PART_MAGIC_WROD, RT_PARAM1_PART_NAME, RT_OTA_BL_FLASH_DEV_NAME, RT_PARAM1_PART_ADDR, RT_PARAM1_PART_LEN, 0},   \
    {RT_OTA_PART_MAGIC_WROD, RT_PARAM2_PART_NAME, RT_OTA_BL_FLASH_DEV_NAME, RT_PARAM2_PART_ADDR, RT_PARAM2_PART_LEN, 0},   \
    {RT_OTA_PART_MAGIC_WROD, RT_PARAM3_PART_NAME, RT_OTA_BL_FLASH_DEV_NAME, RT_PARAM3_PART_ADDR, RT_PARAM3_PART_LEN, 0},   \
    {RT_OTA_PART_MAGIC_WROD, RT_PARAM4_PART_NAME, RT_OTA_BL_FLASH_DEV_NAME, RT_PARAM4_PART_ADDR, RT_PARAM4_PART_LEN, 0},   \
}
#endif /* RT_OTA_PART_HAS_TABLE_CFG */

#endif /* _RT_OTA_CFG_H_ */
