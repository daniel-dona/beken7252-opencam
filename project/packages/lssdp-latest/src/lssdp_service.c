/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: MIT License
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-05-11     SummerGift   first version
 */

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
//#include <netdev.h>
#include "lssdp_service.h"

#define DBG_SECTION_NAME  "lssdp service"
#define DBG_LEVEL         DBG_INFO
#include <rtdbg.h>

static rt_slist_t _lssdp_list = RT_SLIST_OBJECT_INIT(_lssdp_list);

// register a service to lssdp daemon
static int lssdp_service_register(struct lssdp_service *h)
{
    struct rt_slist_node *head;

    if (h == RT_NULL)
        return -RT_ERROR;
    
    /* enter interrupt */
    rt_interrupt_enter();

    rt_slist_for_each(head, &_lssdp_list)
    {
        if (rt_strcmp(h->name, ((lssdp_service_t)head)->name) == 0)
        {
            /* leave interrupt */
            rt_interrupt_leave();

            LOG_E("service %s is already exists!", h->name );
            return -RT_ERROR;
        }
    }

    rt_slist_init(&h->list);
    rt_slist_append(&_lssdp_list, &h->list);
    
    /* leave interrupt */
    rt_interrupt_leave();

    return RT_EOK;
}

// unregister a service to lssdp daemon
static int lssdp_service_unregister(struct lssdp_service *h)
{
    struct rt_slist_node *head;

    if (h == RT_NULL)
        return -RT_ERROR;
    
    /* enter interrupt */
    rt_interrupt_enter();

    rt_slist_for_each(head, &_lssdp_list)
    {
        if (rt_strcmp(h->name, ((lssdp_service_t)head)->name) == 0)
        {
            rt_slist_remove(&_lssdp_list, head);
            
            /* leave interrupt */
            rt_interrupt_leave();
            
            rt_free(head);
            return RT_EOK;
        }
    }
    
    /* leave interrupt */
    rt_interrupt_leave();

    LOG_E("Can't find service name %s to unregister", h->name);

    return -RT_ERROR;
}

// add a service to lssdp
int lssdp_service_add(struct lssdp_service *h)
{
    struct lssdp_service *ser;

    if (h == RT_NULL)
        return -RT_ERROR;

    ser = (struct lssdp_service*) rt_malloc (sizeof(struct lssdp_service));
    if (ser == NULL)
    {
        LOG_E("lssdp_service_add malloc failed!");
    }
    else
    {
        memset(ser, 0, sizeof(sizeof(struct lssdp_service)));
    }

    ser->info.port = h->info.port;
    ser->info.neighbor_timeout = h->info.neighbor_timeout;

    memcpy(ser->name,                      h->name,                      LSSDP_FIELD_LEN);
    memcpy(ser->info.search_target,        h->info.search_target,        LSSDP_FIELD_LEN);
    memcpy(ser->info.unique_service_name,  h->info.unique_service_name,  LSSDP_FIELD_LEN);
    memcpy(ser->info.sm_id,                h->info.sm_id,                LSSDP_FIELD_LEN);
    memcpy(ser->info.device_type,          h->info.device_type,          LSSDP_FIELD_LEN);
    memcpy(ser->info.suffix,               h->info.suffix,               LSSDP_FIELD_LEN);

    // if register failed
    if(lssdp_service_register(ser) != 0)
    {
        rt_free(ser);
        return -RT_ERROR;
    }

    return RT_EOK;
}

// del a service from lssdp
int lssdp_service_del(struct lssdp_service *h)
{
    if (h == RT_NULL)
        return -RT_ERROR;

    if(lssdp_service_unregister(h) != 0)
    {
        LOG_E("lssdp_service_del service %s failed!", h->name);
        return -RT_ERROR;
    }

    return RT_EOK;
}

// get the number of lssdp services
int lssdp_service_count(void)
{
    return rt_slist_len((const rt_slist_t*)(&_lssdp_list));
}

// send notify messages to network
int lssdp_service_send_notify(lssdp_ctx * lssdp)
{
    struct rt_slist_node *head;

    if (!lssdp)
        return -RT_ERROR;

    rt_slist_for_each(head, &_lssdp_list)
    {
        LOG_D("name: %s ", ((struct lssdp_service *)head)->name);

        if((rt_strcmp(lssdp->header.unique_service_name, ((lssdp_service_t)head)->info.unique_service_name) != 0))
        {
            strcpy(lssdp->header.search_target,       ((lssdp_service_t)head)->info.search_target);
            strcpy(lssdp->header.unique_service_name, ((lssdp_service_t)head)->info.unique_service_name);
            strcpy(lssdp->header.sm_id,               ((lssdp_service_t)head)->info.sm_id);
            strcpy(lssdp->header.device_type,         ((lssdp_service_t)head)->info.device_type);
            strcpy(lssdp->header.location.suffix,     ((lssdp_service_t)head)->info.suffix);
        }

        lssdp_send_notify(lssdp);
    }

    return RT_EOK;
}

#define THREAD_PRIORITY         10
#define THREAD_STACK_SIZE       4096
#define THREAD_TIMESLICE        5

int lssdp_start(void)
{
    rt_thread_t tid1 = RT_NULL;
    extern int lssdp_daemon();
    tid1 = rt_thread_create("lssdp",
                            (void (*)(void *))lssdp_daemon, RT_NULL,
                            THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
    else
        return -1;

    return RT_EOK;
}

INIT_APP_EXPORT(lssdp_start);
//MSH_CMD_EXPORT(lssdp_start, LSSDP START);
