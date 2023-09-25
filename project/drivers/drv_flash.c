#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

// #define DBG_ENABLE
#define DBG_SECTION_NAME  "[FLASH]"
// #define DBG_LEVEL         DBG_LOG
// #define DBG_LEVEL         DBG_INFO
#define DBG_LEVEL         DBG_WARNING
// #define DBG_LEVEL         DBG_ERROR
#define DBG_COLOR
#include <rtdbg.h>

#ifdef BEKEN_USING_FLASH

#include "typedef.h"
#include "drv_flash.h"
#include "flash.h"

static struct rt_mutex flash_mutex;

void beken_flash_read(rt_uint32_t address, void *data, rt_uint32_t size)
{
    if (size == 0)
    {
        dbg_log(DBG_INFO, "flash read len is NULL\n");
        return;
    }
    rt_mutex_take(&flash_mutex, RT_WAITING_FOREVER);
    flash_read(data, size, address);
    rt_mutex_release(&flash_mutex);
}

void beken_flash_write(rt_uint32_t address, const void *data, rt_uint32_t size)
{
    if (size == 0)
    {
        dbg_log(DBG_INFO, "flash write len is NULL\n");
        return;
    }

    rt_mutex_take(&flash_mutex, RT_WAITING_FOREVER);
    flash_write((char *)data, size, address);
    rt_mutex_release(&flash_mutex);
}

void beken_flash_erase(rt_uint32_t address)
{
    rt_mutex_take(&flash_mutex, RT_WAITING_FOREVER);
    address &= (0xFFF000);
    flash_ctrl(CMD_FLASH_ERASE_SECTOR, &address);
    rt_mutex_release(&flash_mutex);
}

int beken_flash_init(void)
{
    return rt_mutex_init(&flash_mutex, "flash", RT_IPC_FLAG_PRIO);
}
INIT_DEVICE_EXPORT(beken_flash_init);
#endif


