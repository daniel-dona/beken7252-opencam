/*
 * File      : module_ssl.c
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

#if defined(WEBNET_USING_SSL) && defined(RT_USING_SSL)
int webnet_module_ssl(struct webnet_session* sesion, int event)
{
    if (event == WEBNET_EVENT_INIT)
    {
        /* start ssl thread */
    }

    return WEBNET_MODULE_CONTINUE;
}
#endif

