#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

#include "typedef.h"
#include "flash_pub.h"

int beken_flash_init(void);
void beken_flash_read(rt_uint32_t address, void *data, rt_uint32_t size);
void beken_flash_write(rt_uint32_t address, const void *data, rt_uint32_t size);
void beken_flash_erase(rt_uint32_t address);

#endif