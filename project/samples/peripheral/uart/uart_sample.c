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
 *2017-12-15      DQL          the first version
 */

#include "app_uart.h"

#include "board.h"


void test_thread_entry(void *parameter)
{
    rt_uint8_t uart_rx_data;

    if (uart_open("uart2") != RT_EOK)
    {
        rt_kprintf("uart open error.\n");
        while (1)
        {
            rt_thread_delay(10);
        }
    }

    uart_putchar('2');
    uart_putchar('0');
    uart_putchar('1');
    uart_putchar('8');
    uart_putchar('\n');

    uart_putstring("Hello RT-Thread!\r\n");

    while (1)
    {
        uart_rx_data = uart_getchar();
        uart_rx_data = uart_rx_data + 1;
        uart_putchar(uart_rx_data);
    }
}

int uart_samples(int argc, char *argv)
{

    rt_thread_t tid;

    tid = rt_thread_create("test",
                           test_thread_entry,
                           RT_NULL,
                           1024,
                           2,
                           10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

FINSH_FUNCTION_EXPORT_ALIAS(uart_samples, __cmd_uart_sample, uart sample);


