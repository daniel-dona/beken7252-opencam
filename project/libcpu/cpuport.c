/*
 * File      : cpuport.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013, RT-Thread Develop Team
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rthw.h>
#include <rtthread.h>

extern void machine_reset(void);
extern void machine_shutdown(void);

/**
 *  shutdown CPU
 *
 */
void rt_hw_cpu_shutdown()
{
    rt_uint32_t level;
    rt_kprintf("shutdown...\n");

    level = rt_hw_interrupt_disable();
    machine_shutdown();
    while (level)
    {
        RT_ASSERT(0);
    }
}

#ifdef RT_USING_CPU_FFS
/**
 * This function finds the first bit set (beginning with the least significant bit)
 * in value and return the index of that bit.
 *
 * Bits are numbered starting at 1 (the least significant bit).  A return value of
 * zero from any of these functions means that the argument was zero.
 *
 * @return return the index of the first bit set. If value is 0, then this function
 * shall return 0.
 */
#if defined(__CC_ARM)
int __rt_ffs(int value)
{
    register rt_uint32_t x;

    if (value == 0)
        return value;

    __asm
    {
        rsb x, value, #0
        and x, x, value
        clz x, x
        rsb x, x, #32
    }

    return x;
}
#elif defined(__GNUC__) || defined(__ICCARM__)
int __rt_ffs(int value)
{
    return __builtin_ffs(value);
}
#endif

#endif

