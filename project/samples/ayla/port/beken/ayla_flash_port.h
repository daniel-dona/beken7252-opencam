/*
 * File      : ayla_flash_port.h
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-22     Murphy       the first version
 */
#ifndef __AYLA_FLASH_PORT_H__
#define __AYLA_FLASH_PORT_H__

#include <rtthread.h>

#define AYLA_FLASH_DEBUG 0

/* error level log */
#ifdef  log_e
#undef  log_e
#endif
#define log_e(...)                     rt_kprintf("\033[31;22m[E/ayla] (%s:%d) ", __FUNCTION__, __LINE__);rt_kprintf(__VA_ARGS__);rt_kprintf("\033[0m\n")

/* info level log */
#ifdef  log_i
#undef  log_i
#endif
#define log_i(...)                     rt_kprintf("\033[36;22m[I/ayla] ");                                rt_kprintf(__VA_ARGS__);rt_kprintf("\033[0m\n")

#if AYLA_FLASH_DEBUG

/* debug level log */
#ifdef  log_d
#undef  log_d
#endif
#define log_d(...)                     rt_kprintf("[D/ayla] (%s:%d) ", __FUNCTION__, __LINE__);           rt_kprintf(__VA_ARGS__);rt_kprintf("\n")

#else

/* debug level log */
#ifdef  log_d
#undef  log_d
#endif
#define log_d(...)

#endif /* AYLA_FLASH_DEBUG */

#ifdef AYLA_USING_RT_OTA
#include "fal_cfg.h"
#define AYLA_START_CONF_SIZE        RT_BK_PARAM1_PART_LEN
#define AYLA_FACTORY_CONF_SIZE      RT_BK_PARAM2_PART_LEN

#define AYLA_CONF_START_PART_NAME         RT_BK_PARAM1_PART_NAME
#define AYLA_CONF_FACTORY_PART_NAME       RT_BK_PARAM2_PART_NAME
#else
#define AYLA_START_CONF_SIZE        (16 * 1024)
#define AYLA_FACTORY_CONF_SIZE      (16 * 1024)

#define AYLA_CONF_START_PART_NAME         ""
#define AYLA_CONF_FACTORY_PART_NAME       ""

#define BEKEN_TOTAL_FLASH_SIZE       (2 * 1024 * 1024)
#define AYLA_CONF_FLASH_START_ADDR (BEKEN_TOTAL_FLASH_SIZE - 0x2000 - AYLA_FACTORY_CONF_SIZE - AYLA_START_CONF_SIZE) // 0x2000: 8KBytes reserved for beken 
#endif

void ayla_conf_flash_init(void);
int ayla_flash_read(uint32_t dset_handle, uint8_t *buffer, uint32_t len, uint32_t offset, void *loc_name);
int ayla_flash_write(uint32_t dset_handle, uint8_t *buffer, uint32_t len, uint32_t offset, void *loc_name);
int ayla_flash_erase(uint32_t dset_handle, uint32_t len, uint32_t offset, void *loc_name);

#endif /* __AYLA_FLASH_PORT_H__ */