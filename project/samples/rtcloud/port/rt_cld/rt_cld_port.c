/*
 * File      : rt_cld_port.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-06     chenyong     the first version
 * 2018-03-20     Murphy       adapt to beken7231
 */
#include <stdio.h>
#include <string.h>

#include <board.h>
#include "rt_cld.h"

/* Replace CLD_SN with your own serial number in rtcloud */
#define CLD_SN     "11223344"

void cld_port_get_device_sn(char *sn)
{
    strncpy(sn, CLD_SN, strlen(CLD_SN) + 1);
}

void cld_port_get_mac_addr(char *mac)
{
    //Get the MAC
}

void cld_port_jump_to_bl(void)
{
    //Reset the device
    extern void rt_hw_cpu_reset(void);
    rt_hw_cpu_reset();
}
