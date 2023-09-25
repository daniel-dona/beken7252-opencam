/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: MIT License
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-05-11     SummerGift   first version
 */
 
#ifndef __LSSDP_SERVICE_H__
#define __LSSDP_SERVICE_H__

#include <rtthread.h>
#include "lssdp.h"

#ifdef __cplusplus
extern "C" {
#endif

struct lssdp_device
{
    unsigned short  port;
    long            neighbor_timeout;

    char        search_target          [LSSDP_FIELD_LEN];
    char        unique_service_name    [LSSDP_FIELD_LEN];
    char        sm_id                  [LSSDP_FIELD_LEN];
    char        device_type            [LSSDP_FIELD_LEN];
    char        suffix                 [LSSDP_FIELD_LEN];
};

struct lssdp_service
{
    rt_slist_t list;

    char name[LSSDP_FIELD_LEN];

    uint8_t is_search;
    uint8_t is_notify;

    struct lssdp_device info;
};
typedef struct lssdp_service * lssdp_service_t;

int lssdp_service_add(struct lssdp_service *h);
int lssdp_service_del(struct lssdp_service *h);
int lssdp_service_len(void);
int lssdp_service_send_notify(lssdp_ctx * lssdp);

#ifdef __cplusplus
}
#endif

#endif /* __LSSDP_SERVICE_H__ */
