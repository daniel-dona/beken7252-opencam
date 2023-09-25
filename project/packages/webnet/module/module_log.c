/*
 * File      : module_log.c
 * This file is part of RT-Thread RTOS/WebNet Server
 * COPYRIGHT (C) 2011, Shanghai Real-Thread Technology Co., Ltd
 *
 * All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-08-02     Bernard      the first version
 */
#include <webnet.h>
#include <module.h>
#include <util.h>

#ifdef WEBNET_USING_LOG
#include <lwip/inet.h>

int webnet_module_log(struct webnet_session* session, int event)
{
    struct webnet_request* request;

    if (session != RT_NULL)
    {
        request = session->request;
    }
    else
    {
        request = RT_NULL;
    }

    if (event == WEBNET_EVENT_INIT)
    {
        rt_kprintf("webnet server initialized\n");
    }
    else if (event == WEBNET_EVENT_URI_PHYSICAL)
    {
        rt_uint32_t index;
        rt_kprintf("\n  new client: %s:%u\n",
                   inet_ntoa(session->cliaddr.sin_addr),
                   ntohs(session->cliaddr.sin_port));

        switch (request->method)
        {
        case WEBNET_GET:
            rt_kprintf("      method: GET\n");
            break;
        case WEBNET_PUT:
            rt_kprintf("      method: PUT\n");
            break;
        case WEBNET_POST:
            rt_kprintf("      method: POST\n");
            break;
        case WEBNET_HEADER:
            rt_kprintf("      method: HEADER\n");
            break;
        case WEBNET_SUBSCRIBE:
            rt_kprintf("      method: SUBSCRIBE\n");
            break;
        case WEBNET_UNSUBSCRIBE:
            rt_kprintf("      method: UNSUBSCRIBE\n");
            break;
        default:
            break;
        }
        rt_kprintf("     request: %s\n", request->path);
        for (index = 0; index < request->query_counter; index ++)
        {
            rt_kprintf("query[%d]: %s => %s\n", index,
                       request->query_items[index].name,
                       request->query_items[index].value);
        }
    }
    else if (event == WEBNET_EVENT_URI_POST)
    {
        rt_kprintf("physical url: %s\n", request->path);
        rt_kprintf("   mime type: %s\n", mime_get_type(request->path));
    }
    else if (event == WEBNET_EVENT_RSP_HEADER)
    {
    }
    else if (event == WEBNET_EVENT_RSP_FILE)
    {
    }

    return WEBNET_MODULE_CONTINUE;
}
#endif
