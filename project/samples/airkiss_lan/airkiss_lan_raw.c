/*
 * File      : airkiss_lan_raw.c
 *             A simple DHCP server implementation
 *
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2011-2018, Shanghai Real-Thread Technology Co., Ltd
 * http://www.rt-thread.com
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
 * 2018-07-07     aozima       first commit.
 */

#include <stdio.h>
#include <stdint.h>

#include <rtthread.h>

#include <lwip/opt.h>
#include <lwip/inet_chksum.h>
#include <lwip/init.h>
#include <lwip/ip.h>
#include <lwip/udp.h>
#include <lwip/inet.h>

#include <netif/etharp.h>
#include <netif/ethernetif.h>

#if (LWIP_VERSION) < 0x02000000U
    #error "not support old LWIP"
#endif

#if !LWIP_IPV4
    #error "must enable IPV4"
#endif

#include "airkiss.h"
#include "weixin_config_custom.h"

#if 0
extern const airkiss_config_t ak_conf;
#else
static const airkiss_config_t ak_conf =
{
    (airkiss_memset_fn) &rt_memset,
    (airkiss_memcpy_fn) &rt_memcpy,
    (airkiss_memcmp_fn) &rt_memcmp,
    (airkiss_printf_fn) &rt_kprintf,
};
#endif

/**************************************************/
#define DEBUG_PRINTF     rt_kprintf("[airkiss_lan] ");rt_kprintf

#define malloc  rt_malloc
#define realloc rt_realloc
#define free    rt_free
/**************************************************/

#define DEFAULT_LAN_PORT            (12476)
#define AIRKISS_LAN_BUF_LEN         (1024)

static char DEVICE_ID[33]; /* 32+1 */

static struct udp_pcb *airkiss_pcb;

static void airkiss_lan_handler(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *recv_addr, u16_t port)
{
    airkiss_lan_ret_t ret = AIRKISS_LAN_CONTINUE;
    airkiss_lan_ret_t packret;
    char *lan_buf = RT_NULL;
    uint16_t lan_buf_len;
    struct pbuf *q = RT_NULL;

    DEBUG_PRINTF("recv %d byte from(%s , %d)\n", p->tot_len, ipaddr_ntoa(recv_addr), port);

    lan_buf_len = (p->tot_len > AIRKISS_LAN_BUF_LEN)?(p->tot_len):AIRKISS_LAN_BUF_LEN;
    lan_buf_len += 4;

    if(p->tot_len == p->len)
    {
        DEBUG_PRINTF("same tot_len&len, use payload for airkiss_lan_recv!\n");
        ret = airkiss_lan_recv(p->payload, p->tot_len, &ak_conf);
    }
    else
    {
        DEBUG_PRINTF("diff tot_len&len, use malloc for airkiss_lan_recv!\n");
        lan_buf = rt_malloc(lan_buf_len);
        if(lan_buf)
        {
            pbuf_copy_partial(p, lan_buf, p->tot_len, 0);
            ret = airkiss_lan_recv(lan_buf, p->tot_len, &ak_conf);
        }
    }

    switch (ret)
    {
        case AIRKISS_LAN_SSDP_REQ:
        {
            DEBUG_PRINTF("AIRKISS_LAN_SSDP_REQ!\n");
        
            if(!lan_buf)
            {
                lan_buf = (char *)malloc(lan_buf_len + 4);
            }

            if(!lan_buf)
            {
                DEBUG_PRINTF("no memory for lan_buf!\n");
                goto _exit;
            }

            packret = airkiss_lan_pack(AIRKISS_LAN_SSDP_RESP_CMD, (void *)WEIXIN_DEVICE_TYPE, (void *)DEVICE_ID, 
            0, 0, lan_buf, &lan_buf_len, &ak_conf);
            if (packret != AIRKISS_LAN_PAKE_READY)
            {
                DEBUG_PRINTF("Pack lan packet error!\n");
                goto _exit;
            }

            DEBUG_PRINTF("alloc pbuf for send! %d\n", lan_buf_len);
            q = pbuf_alloc(PBUF_TRANSPORT, lan_buf_len, PBUF_RAM);
            if (q)
            {
                pbuf_take(q, lan_buf, lan_buf_len);
                packret = udp_sendto(pcb, q, recv_addr, port);
                DEBUG_PRINTF("pack %d byte to(%s , %d)\n", lan_buf_len, ipaddr_ntoa(recv_addr), port);
            }
            else
            {
                DEBUG_PRINTF("no memory for send alloc!\n");
                packret = ERR_MEM;
            }

            if (packret != ERR_OK)
            {
                DEBUG_PRINTF("LAN UDP Send err! %d:%d\n", packret, lan_buf_len);
            }
        }
        break;

        default:
        if (ret != AIRKISS_LAN_CONTINUE)
        {
            DEBUG_PRINTF("Pack %d is not ssdq req!\n", ret);
        }
        break;
    }


_exit:
    if(p)
        pbuf_free(p);

    if(q)
        pbuf_free(q);

    if(lan_buf)
        rt_free(lan_buf);
}

int airkiss_lan_init(void)
{
    if(airkiss_pcb)
    {
        DEBUG_PRINTF("already init.\n");
        return -1;
    }

    airkiss_pcb = udp_new();
    if(!airkiss_pcb)
    {
        DEBUG_PRINTF("create pcb faild.\n");
        return -1;
    }

    ip_set_option(airkiss_pcb, SOF_BROADCAST);
    udp_bind(airkiss_pcb, IP_ADDR_ANY, DEFAULT_LAN_PORT);
    udp_recv(airkiss_pcb, airkiss_lan_handler, RT_NULL);

    /* custom */
    sprintf(DEVICE_ID, "EF4017D1632048DF1F1B31");

    return 0;
}
INIT_APP_EXPORT(airkiss_lan_init);

