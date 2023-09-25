/*
 * File      : board.c
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

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"
#include "interrupt.h"
#include "driver_pub.h"
#include "drv_uart.h"
#include "include.h"
#include "func_pub.h"
#include <string.h>

extern void os_clk_init(void);

#if (RT_TICK_PER_SECOND != 1000)
#error "RT_TICK_PER_SECOND of bk7221u can only be configured as 1000!!!"
#endif 

static struct rt_memheap tcm_heap; 

void *tcm_malloc(unsigned long size)
{
    return rt_memheap_alloc(&tcm_heap, size);
}

void tcm_free(void *ptr)
{
    rt_memheap_free(ptr); 
}

void *tcm_calloc(unsigned int n, unsigned int size)
{
    void* ptr = NULL;
    
    ptr = tcm_malloc(n * size);
    if (ptr)
    {
        memset(ptr, 0, n * size);
    }
    
    return ptr;
}

void *tcm_realloc(void *ptr, unsigned long size)
{
    return rt_memheap_realloc(&tcm_heap, ptr, size);
}

void rt_hw_board_init(void)
{
#ifdef RT_USING_HEAP
    /* init memory system */
    rt_system_heap_init(RT_HW_SDRAM_BEGIN, RT_HW_SDRAM_END);
    rt_memheap_init(&tcm_heap, "TCM", RT_HW_TCM_BEGIN, RT_HW_TCM_END-RT_HW_TCM_BEGIN); 

#if(CFG_SOC_NAME == SOC_BK7221U)
    rt_sdram_heap_init();
#endif // #if(CFG_SOC_NAME == SOC_BK7221U)
#endif

    /* init hardware */
    driver_init();
    /* interrupt init */
    rt_hw_interrupt_init();
    /* init hardware interrupt */
    rt_hw_uart_init();
    /* init system tick */
    os_clk_init();

#ifdef RT_USING_CONSOLE
    /* set console device */
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif /* RT_USING_CONSOLE */

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

}

#define WDT_DEV_NAME "wdt"
/**
 * reset cpu by dog's time-out
 */
void rt_hw_cpu_reset(void)
{
    struct rt_device *device;
    int time = 10;

    device = rt_device_find(WDT_DEV_NAME);

    if (!device)
    {
        rt_kprintf("Device %s not found \n");
        return ;
    }

    rt_kprintf("reboot system \n");
    rt_device_control(device, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &time);
    rt_device_control(device, RT_DEVICE_CTRL_WDT_START, RT_NULL);

    while (1);
}

FINSH_FUNCTION_EXPORT_ALIAS(rt_hw_cpu_reset, __cmd_reboot, reboot system);

#ifdef BEKEN_USING_WLAN
static int auto_func_init(void)
{
    func_init();
    return 0;
}
INIT_PREV_EXPORT(auto_func_init);
#endif

extern void cp15_enable_alignfault(void);
static int auto_enable_alignfault(void)
{
    cp15_enable_alignfault();
    return 0;
}
INIT_BOARD_EXPORT(auto_enable_alignfault);
