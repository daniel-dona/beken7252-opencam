/*
 * File      : tls_app_test.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
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
 * Date          Author          Notes
 * 2018-01-22    chenyong     first version
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>

#include "tls_certificate.h"
#include "tls_client.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#define malloc  tls_malloc
#define free    tls_free
#define strdup  rt_strdup

#define MBEDTLS_WEB_SERVER  "www.howsmyssl.com"
#define MBEDTLS_WEB_PORT    "443"

#define MBEDTLS_READ_BUFFER 1024

static const char *REQUEST = "GET https://www.howsmyssl.com/a/check HTTP/1.0\r\n"
    "Host: www.howsmyssl.com\r\n"
    "User-Agent: rtthread/3.1 rtt\r\n"
    "\r\n";

static MbedTLSSession *tls_session = RT_NULL;


static void mbedlts_client_entry(void *parament)
{
    int ret = 0, len = 0;
    char *pers = "hello_world";
    MbedTLSSession *session = (MbedTLSSession *)parament;
    
    if((ret = mbedtls_client_init(session, (void *)pers, strlen(pers))) != 0)
    {
        rt_kprintf("MbedTLSClientInit err return : -0x%x\n", -ret);
        goto __exit;
    }

    if((ret = mbedtls_client_context(session)) < 0)
    {
        rt_kprintf("MbedTLSCLlientContext err return : -0x%x\n", -ret);
        goto __exit;
    }

    if((ret = mbedtls_client_connect(session)) != 0)
    {
        rt_kprintf("MbedTLSCLlientConnect err return : -0x%x\n", -ret);
        goto __exit;
    }

    while((ret = mbedtls_client_write(session, (const unsigned char *)REQUEST, strlen(REQUEST))) <= 0)
    {
        if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            rt_kprintf("mbedtls_ssl_write returned -0x%x\n", -ret);
            goto __exit;
        }
    }
    rt_kprintf("Writing HTTP request success...\n");
    
    len = ret;

    rt_kprintf("Getting HTTP response...\n");
    do
    {
        int i= 0;
        
        memset(session->buffer, 0x00, session->buffer_len);
        ret = mbedtls_client_read(session, (unsigned char *)session->buffer, len);
        if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)	
                continue;
        
        if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
            break;

        if(ret < 0)
        {
            rt_kprintf("mbedtls_ssl_read returned -0x%x\n", -ret);
            break;
        }

        if(ret == 0)
        {
            rt_kprintf("connection closed\n");
            break;
        }

        len = ret;
        for(i = 0; i<len; i++)
            rt_kprintf("%c", session->buffer[i]);
    }while(1);
    rt_kprintf("\n");
    
__exit:
    mbedtls_client_close(session);

    rt_kprintf("mbedlts_client_entry close!\n");

    return ;
}
int mbedlts_client_start(void)
{
    rt_thread_t tid;

    tls_session = (MbedTLSSession *)malloc(sizeof(MbedTLSSession));
    if(!tls_session)
    {   
        rt_kprintf("no memory for tls_session struct\n");
        return RT_ERROR;
    }
    
    tls_session->host = strdup(MBEDTLS_WEB_SERVER);
    tls_session->port = strdup(MBEDTLS_WEB_PORT);

    tls_session->buffer_len = MBEDTLS_READ_BUFFER;
    tls_session->buffer = malloc(tls_session->buffer_len);
    if(!tls_session->buffer)
    {
        rt_kprintf("no memory for tls_session->buffer malloc\n");
        return RT_ERROR;
    }
  
    tid = rt_thread_create("tls_c",
                    mbedlts_client_entry, (void *)tls_session,
                    6 * 1024, RT_THREAD_PRIORITY_MAX / 3 - 1, 5);
    if (tid != RT_NULL)
            rt_thread_startup(tid); 

    return RT_EOK;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
    FINSH_FUNCTION_EXPORT_ALIAS(mbedlts_client_start, tls_test, mbedtls client test start);
#ifdef FINSH_USING_MSH
    MSH_CMD_EXPORT_ALIAS(mbedlts_client_start, tls_test, mbedtls client test start);
#endif 
#endif

