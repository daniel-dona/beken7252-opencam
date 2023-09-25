/*
 * File      : main.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
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
 * 2018-1-20       DQL         the first version
 */

#include <rthw.h>
#include <rtthread.h>

#include <drivers/pin.h>

#define LED0    30
#define KEY0    4
#define KEY1    7

void hdr_callback(void *args)
{
    char *a = args;
    rt_kprintf("key0 down! %s\n", a);
}

void irq_thread_entry(void *parameter)
{
    rt_pin_mode(KEY0, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(KEY0, PIN_IRQ_MODE_FALLING, hdr_callback, (void *)"callback args");
    rt_pin_irq_enable(KEY0, PIN_IRQ_ENABLE);

}

void key_thread_entry(void *parameter)
{
    rt_pin_mode(KEY1, PIN_MODE_INPUT_PULLUP);

    while (1)
    {
        if (rt_pin_read(KEY1) == PIN_LOW)
        {
            rt_kprintf("key1 pressed!\n");
        }

        rt_thread_delay(rt_tick_from_millisecond(10));
    }
}

void led_thread_entry(void *parameter)
{
    rt_pin_mode(LED0, PIN_MODE_OUTPUT);

    while (1)
    {
        rt_pin_write(LED0, PIN_LOW);

        rt_thread_delay(rt_tick_from_millisecond(500));

        rt_pin_write(LED0, PIN_HIGH);

        rt_thread_delay(rt_tick_from_millisecond(500));
    }
}

int gpio_sample(void)
{

    rt_thread_t tid = RT_NULL;

    tid = rt_thread_create("key",
                           key_thread_entry,
                           RT_NULL,
                           1024,
                           2,
                           10);

    if (tid != RT_NULL)
        rt_thread_startup(tid);


    tid = rt_thread_create("led",
                           led_thread_entry,
                           RT_NULL,
                           1024,
                           3,
                           10);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    tid = rt_thread_create("exirq",
                           irq_thread_entry,
                           RT_NULL,
                           1024,
                           25,
                           10);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

FINSH_FUNCTION_EXPORT_ALIAS(gpio_sample, __cmd_gpio, gpio sample);