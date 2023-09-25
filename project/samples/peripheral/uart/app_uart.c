/*
 * File      : app_uart.c
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
 *2017-12-15      DQL          the first version
 */

#include <rthw.h>
#include <rtthread.h>

#include "app_uart.h"

#define UART_RX_EVENT (1 << 0)

static struct rt_event event;

static rt_device_t uart_device = RT_NULL;

static rt_err_t uart_intput(rt_device_t dev, rt_size_t size)
{
    rt_event_send(&event, UART_RX_EVENT);

    return RT_EOK;
}

rt_uint8_t uart_getchar(void)
{
    rt_uint32_t e;
    rt_uint8_t ch;

    while (rt_device_read(uart_device, 0, &ch, 1) != 1)
    {
        rt_event_recv(&event, UART_RX_EVENT, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);
    }

    return ch;
}
void uart_putchar(const rt_uint8_t c)
{
    rt_size_t len = 0;
    rt_uint32_t timeout = 0;
    do
    {
        len = rt_device_write(uart_device, 0, &c, 1);
        timeout++;
    }
    while (len != 1 && timeout < 500);
}

void uart_putstring(const rt_uint8_t *s)
{
    while (*s)
    {
        uart_putchar(*s++);
    }
}

rt_err_t uart_open(const char *name)
{
    rt_err_t res;

    uart_device = rt_device_find(name);
    if (uart_device != RT_NULL)
    {
        res = rt_device_set_rx_indicate(uart_device, uart_intput);
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n", name, res);
            return -RT_ERROR;
        }
        res = rt_device_open(uart_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
        if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n", name, res);
            return -RT_ERROR;
        }

    }
    else
    {
        rt_kprintf("can't find %s device.\n", name);
        return -RT_ERROR;
    }

    rt_event_init(&event, "event", RT_IPC_FLAG_FIFO);

    return RT_EOK;
}


