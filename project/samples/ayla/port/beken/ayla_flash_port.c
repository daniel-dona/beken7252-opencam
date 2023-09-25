/*
 * File      : ayla_flash_port.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-22     Murphy       the first version
 */

#include "ayla_flash_port.h"

#define AYLA_FLASH_PORT_DEBUG (0)

#ifdef AYLA_USING_RT_OTA
#include <fal.h>
static const struct fal_partition * start_part = RT_NULL;
static const struct fal_partition * factory_part = RT_NULL;
#else
#include "flash.h"
#include <flash_pub.h>
#endif
#include <string.h>
#include <stdio.h>

static struct rt_mutex locker;

/*
 * Initialize flash handle
 */
void ayla_conf_flash_init(void)
{
    int parts_num;
    rt_mutex_init(&locker, "ayla_flash", RT_IPC_FLAG_FIFO);

#ifdef AYLA_USING_RT_OTA
    parts_num = fal_init();

    if (parts_num <= 0)
    {
        log_e("Initialize failed! Don't found the partition table.");
        // result = -1;
        // goto __exit;
        return;
    }    

    /* Get user param partition information */
    if ((start_part = fal_partition_find(AYLA_CONF_START_PART_NAME)) == RT_NULL)
    {
        log_e("Partition (%s) find error!", AYLA_CONF_START_PART_NAME);
        return;
    }

    /* Get user param partition information */
    if ((factory_part = fal_partition_find(AYLA_CONF_FACTORY_PART_NAME)) == RT_NULL)
    {
        log_e("Partition (%s) find error!", AYLA_CONF_FACTORY_PART_NAME);
        return;
    }    
#endif    

}

int ayla_flash_read(uint32_t dset_handle, uint8_t *buffer, uint32_t len, uint32_t offset, void *loc_name)
{
    uint32_t true_offset = offset;
    if (loc_name == NULL || start_part == NULL || factory_part == NULL )
    {
        return -1;
    }
    rt_mutex_take(&locker, RT_WAITING_FOREVER);

#ifdef AYLA_USING_RT_OTA

    if (!strncmp(AYLA_CONF_START_PART_NAME, (const char*)loc_name, strlen(loc_name)))
    {
        if ((factory_part->offset < start_part->offset) && (offset >= start_part->len))
        {
            true_offset -= start_part->len;
        }        
        /* write data of application to DL partition  */
        if (fal_partition_read(start_part, true_offset, buffer, len) < 0)
        {
            log_e("Partition (%s) read data error! Partition start addr (0x%08x); Partition len (%d); \
                    Read offset addr (0x%08x); Read len (%d); conf offset (0x%08x); handle (0x%08x)", 
                    start_part->name, start_part->offset, start_part->len,
                    dset_handle + true_offset, len, offset, dset_handle);        
        }
    }
    else if (!strncmp(AYLA_CONF_FACTORY_PART_NAME, (const char*)loc_name, strlen(loc_name)))
    {
        if ((factory_part->offset > start_part->offset) && (offset >= factory_part->len))
        {
            true_offset -= factory_part->len;
        }
        /* write data of application to DL partition  */
        if (fal_partition_read(factory_part, true_offset, buffer, len) < 0)
        {
            log_e("Partition (%s) read data error! Partition start addr (0x%08x); Partition len (%d); \
                    Read offset addr (0x%08x); Read len (%d); conf offset (0x%08x); handle (0x%08x)", 
                    factory_part->name, factory_part->offset, factory_part->len,
                    dset_handle + true_offset, len, offset, dset_handle);        
        }
    }

#else
    flash_read((unsigned char *)buffer, (unsigned long)len, AYLA_CONF_FLASH_START_ADDR + dset_handle + offset);
#endif

    rt_mutex_release(&locker);

    return 0;
}

int ayla_flash_write(uint32_t dset_handle, uint8_t *buffer, uint32_t len, uint32_t offset, void *loc_name)
{
    int param = 1;
    uint32_t true_offset = offset;

    if (loc_name == NULL || start_part == NULL || factory_part == NULL )
    {
        return -1;
    }

    rt_mutex_take(&locker, RT_WAITING_FOREVER);

#ifdef AYLA_USING_RT_OTA

    if (!strncmp(
            AYLA_CONF_START_PART_NAME, 
            (const char*)loc_name, 
            strlen(loc_name)
            )
        )
    {
        log_d("AYLA_CONF_START_PART_NAME:%s", AYLA_CONF_START_PART_NAME);
        if ((factory_part->offset < start_part->offset) && (offset >= start_part->len))
        {
            true_offset -= start_part->len;
        }
        log_d("ayla flash write, partition name:%s; write addr:0x%08x; start_part->offset:0x%08x; offset:0x%08x; true_offset:0x%08x", 
                loc_name, start_part->offset + true_offset, start_part->offset, offset, true_offset);
                
        /* write data of application to DL partition  */
        if (fal_partition_write(start_part, true_offset, buffer, len) < 0)
        {
            log_e("Partition (%s) write data error! Partition start addr (0x%08x); Partition len (%d); \
                    Write offset addr (0x%08x); Write len (%d); conf offset (0x%08x); handle (0x%08x)", 
                    start_part->name, start_part->offset, start_part->len,
                    dset_handle + true_offset, len, offset, dset_handle);        
        }
    }
    else if (!strncmp(
                AYLA_CONF_FACTORY_PART_NAME, 
                (const char*)loc_name, 
                strlen(loc_name)
                )
            )
    {
        log_d("AYLA_CONF_FACTORY_PART_NAME:%s", AYLA_CONF_FACTORY_PART_NAME);
        if ((factory_part->offset > start_part->offset) && (offset >= factory_part->len))
        {
            true_offset -= factory_part->len;
        }
        log_d("ayla flash write, partition name:%s; write addr:0x%08x; factory_part->offset:0x%08x; offset:0x%08x; true_offset:0x%08x", 
                loc_name, factory_part->offset + true_offset, factory_part->offset, offset, true_offset);
        
        /* write data of application to DL partition  */
        if (fal_partition_write(factory_part, true_offset, buffer, len) < 0)
        {
            log_e("Partition (%s) write data error! Partition start addr (0x%08x); Partition len (%d); \
                    Write offset addr (0x%08x); Write len (%d); conf offset (0x%08x); handle (0x%08x)", 
                    factory_part->name, factory_part->offset, factory_part->len,
                    dset_handle + true_offset, len, offset, dset_handle);        
        }
    }
#else
    flash_ctrl(CMD_FLASH_SET_WSR, &param);
    flash_ctrl(CMD_FLASH_WRITE_ENABLE, NULL);
    flash_write((unsigned char *)buffer, (unsigned long)len, AYLA_CONF_FLASH_START_ADDR + dset_handle + offset);
    flash_ctrl(CMD_FLASH_WRITE_DISABLE, NULL);
#endif

    rt_mutex_release(&locker);
    
    return 0;
}

int ayla_flash_erase(uint32_t dset_handle, uint32_t len, uint32_t offset, void *loc_name)
{
    int param = 1;
    unsigned int _size = len;
    uint32_t addr;    

    if (loc_name == NULL || start_part == NULL || factory_part == NULL )
    {
        return -1;
    }    

    rt_mutex_take(&locker, RT_WAITING_FOREVER);

#ifdef AYLA_USING_RT_OTA
    if (!strcmp(AYLA_CONF_START_PART_NAME, (const char*)loc_name))
    {
        fal_partition_erase(start_part, offset, len);
        log_d("[START] flash conf erase addr= [0x%08x]; conf offset (0x%08x); handle (0x%08x)", 
                    dset_handle + offset + start_part->offset, 
                    offset, 
                    dset_handle);
    }
    else if (!strcmp(AYLA_CONF_FACTORY_PART_NAME, (const char*)loc_name))
    {
        fal_partition_erase(factory_part, offset, len);
        log_d("[FACTORY] flash conf erase addr= [0x%08x]; conf offset (0x%08x); handle (0x%08x)", dset_handle + offset + factory_part->offset, offset, dset_handle);
    }
#else
    flash_ctrl(CMD_FLASH_SET_WSR, &param);    
    /* Calculate the start address of the flash sector(4kbytes) */
    addr = (AYLA_CONF_FLASH_START_ADDR + dset_handle + offset) & 0x00FFF000;

    do{
        flash_ctrl(CMD_FLASH_ERASE_SECTOR, &addr);
        addr += 4096;
        if (_size < 4096)
            _size = 0;
        else
            _size -= 4096;

    } while (_size);

#endif
    rt_mutex_release(&locker);

    return 0;
}

#if AYLA_FLASH_PORT_DEBUG
#ifdef RT_USING_FINSH
#include <finsh.h>
int ayla_erase_start(void)
{
    if (start_part == NULL)
    {
        return -1;
    }

    #ifdef AYLA_USING_RT_OTA
    fal_partition_erase(start_part, 0, AYLA_START_CONF_SIZE);
    log_i("Erase ayla start conf finish!");
    #endif
}
int ayla_erase_factory(void)
{
    if (factory_part == NULL )
    {
        return -1;
    }    
    #ifdef AYLA_USING_RT_OTA
    fal_partition_erase(factory_part, 0, AYLA_FACTORY_CONF_SIZE);
    log_i("Erase ayla factory conf finish!");
    #endif
}
MSH_CMD_EXPORT(ayla_erase_start, ayla erase start conf);
MSH_CMD_EXPORT(ayla_erase_factory, ayla erase factory conf);
#endif /* RT_USING_FINSH */
#endif /* AYLA_FLASH_PORT_DEBUG */
