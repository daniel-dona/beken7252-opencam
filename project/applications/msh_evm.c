/*
 * File      : msh_evm.c
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
 * 2018-06-06     RT-Thread    the first version
 */

#include <rtthread.h>
#include "cmd_evm.h"
#include "cmd_rx_sensitivity.h"
int txevm(int argc, char **argv)
{
    int ret = do_evm(NULL, 0, argc, argv);
    if(ret)
    {
        rt_kprintf("tx_evm bad parameters\r\n");
    }
}
MSH_CMD_EXPORT(txevm, do_tx_evm);

int rxsens(int argc, char **argv)
{
    int ret = do_rx_sensitivity(NULL, 0, argc, argv);
    if(ret)
    {
        rt_kprintf("rx sensitivity bad parameters\r\n");
    }
}
MSH_CMD_EXPORT(rxsens, do_rx_sens);

#include "param_config.h"
#include "common.h"
int mac(int argc, char **argv)
{
    uint8_t mac[6];

    if (argc == 1)
    {
        wifi_get_mac_address((char *)mac, CONFIG_ROLE_STA);
        rt_kprintf("MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else if(argc == 2)
    {
        hexstr2bin(argv[1], mac, 6);
        wifi_set_mac_address((char *)mac);
        rt_kprintf("Set MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else
    {
        rt_kprintf("invalid cmd\r\n");
    }

}
MSH_CMD_EXPORT(mac, set_or_read_mac);
