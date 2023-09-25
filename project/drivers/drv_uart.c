/*
 * File      : drv_uart.c
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

#include "interrupt.h"

#include "typedef.h"
#include "drv_uart.h"
#include "uart.h"
#include "icu_pub.h"
#include "board.h"
#include "drv_model_pub.h"

#ifdef RT_USING_SERIAL

struct device_uart
{
    struct rt_serial_device serial;
    rt_uint32_t port;
    rt_uint32_t irqno;
    char name[RT_NAME_MAX];
};

static int uart_putc(struct rt_serial_device *serial, char c)
{
    struct device_uart* uart = serial->parent.user_data;

    RT_ASSERT(serial != RT_NULL);
    return uart_write_byte(uart->port, c);
}

static int uart_getc(struct rt_serial_device *serial)
{
    struct device_uart* uart = serial->parent.user_data;

    RT_ASSERT(serial != RT_NULL);
    return uart_read_byte(uart->port);
}

static rt_size_t uart_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
    return (0);
}

/* UART ISR */
static void uart_rx_callback(int uport, void *param)
{
    // struct rt_serial_device *serial = (struct rt_serial_device*)param;
    // struct device_uart* uart = serial->parent.user_data;

    rt_hw_serial_isr(param, RT_SERIAL_EVENT_RX_IND);
}

static void uart_tx_callback(int uport, void *param)
{
    // struct rt_serial_device *serial = (struct rt_serial_device*)param;
    // struct device_uart* uart = serial->parent.user_data;

    ///rt_hw_serial_isr(param, RT_SERIAL_EVENT_TX_DONE);
}

static rt_err_t uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    rt_uint32_t addr, val;
    struct device_uart * uart;
    uart_config_t config;

    RT_ASSERT(serial != RT_NULL);
    serial->config = *cfg;

    uart = serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    RT_ASSERT((serial->config.data_bits >= DATA_BITS_5) && \
              (serial->config.data_bits <= DATA_BITS_8));

    config.baud_rate = serial->config.baud_rate;
    config.data_width = serial->config.data_bits - DATA_BITS_5;
    config.parity = serial->config.parity;
    config.stop_bits = serial->config.stop_bits;
    config.flow_control = FLOW_CTRL_DISABLED;
    config.flags = 0;
    
    uart_hw_set_change(uart->port, &config);

    return RT_EOK;
}

static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct device_uart * uart;
	unsigned int param;
    uart = serial->parent.user_data;

    RT_ASSERT(uart != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* Disable the UART Interrupt */
        //rt_hw_interrupt_mask(uart->irqno);
        uart_rx_callback_set(uart->port, RT_NULL, RT_NULL);
		if(uart->port == UART1_PORT)
			param = IRQ_UART1_BIT;
		else 
			param = IRQ_UART2_BIT;
    	sddev_control(ICU_DEV_NAME, CMD_ICU_INT_DISABLE, &param);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        /* install interrupt */
        // rt_hw_interrupt_install(uart->irqno, uart_irq_handler,
                                 // serial, uart->name);
        // rt_hw_interrupt_umask(uart->irqno);
        if(uart->port == UART1_PORT)
        	param = IRQ_UART1_BIT;
		else 
			param = IRQ_UART2_BIT;
    	sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);
        uart_rx_callback_set(uart->port, uart_rx_callback, serial);
		uart_tx_fifo_needwr_callback_set(uart->port, uart_tx_callback, serial);
        /* Enable the UART Interrupt */
        break;
    }

    return (RT_EOK);
}

static const struct rt_uart_ops _uart_ops =
{
    uart_configure,
    uart_control,
    uart_putc,
    uart_getc,
    uart_dma_transmit
};

int rt_hw_uart_init(void)
{
    struct rt_serial_device *serial;
    struct device_uart      *uart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

#ifdef BEKEN_USING_UART1
    {
        // static struct rt_serial_device  serial 1;
        static struct device_uart uart1;

        serial  = &uart1.serial;
        uart    = &uart1;

        serial->ops              = &_uart_ops;
        serial->config           = config;
        serial->config.baud_rate = 1*1000*1000;
        serial->config.bufsz     = 2048;

        uart->port  = UART1_PORT; // UART1_PORT;
        uart->irqno = IRQ_UART1;  // IRQ_UART1;

        rt_hw_serial_register(serial,
                              UART1_DEV_NAME,
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              uart);
    }
#endif

#ifdef BEKEN_USING_UART2
    {
        // static struct rt_serial_device  serial 2;
        static struct device_uart uart2;

        serial  = &uart2.serial;
        uart    = &uart2;

        serial->ops              = &_uart_ops;
        serial->config           = config;
        serial->config.baud_rate = 115200;
        serial->config.bufsz     = 2048;

        uart->port  = UART2_PORT; // UART2_PORT;
        uart->irqno = IRQ_UART2;  // IRQ_UART2;

        rt_hw_serial_register(serial,
                              UART2_DEV_NAME,
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              uart);
    }
#endif

    return 0;
}

int rt_hw_uart_exit(void)
{
    rt_device_t dev;
#ifdef BEKEN_USING_UART1
    dev = rt_device_find(UART1_DEV_NAME);
    if (dev)
        rt_device_unregister(dev);
#endif

#ifdef BEKEN_USING_UART2
    dev = rt_device_find(UART2_DEV_NAME);
    if (dev)
        rt_device_unregister(dev);
#endif
}

#endif