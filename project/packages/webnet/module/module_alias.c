/*
 * File      : module_alias.c
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
#include "util.h"

#ifdef WEBNET_USING_ALIAS
struct webnet_alias_item
{
    char* old_path;
    char* new_path;
};
static struct webnet_alias_item *_alias_items = RT_NULL;
static rt_uint32_t _alias_item_count = 0;

void webnet_alias_set(char* old_path, char* new_path)
{
    if (_alias_items == RT_NULL)
    {
        _alias_item_count = 1;
        _alias_items = (struct webnet_alias_item*)wn_malloc (sizeof(struct webnet_alias_item) *
                       _alias_item_count);
    }
    else
    {
        _alias_item_count += 1;
        _alias_items = (struct webnet_alias_item*) wn_realloc (_alias_items, sizeof(struct webnet_alias_item) *
                       _alias_item_count);
    }

    RT_ASSERT(_alias_items != RT_NULL);

    _alias_items[_alias_item_count - 1].old_path = wn_strdup(old_path);
    RT_ASSERT(_alias_items[_alias_item_count - 1].old_path != RT_NULL);
    _alias_items[_alias_item_count - 1].new_path = wn_strdup(new_path);
    RT_ASSERT(_alias_items[_alias_item_count - 1].new_path != RT_NULL);
}
RTM_EXPORT(webnet_alias_set);

int webnet_module_alias(struct webnet_session* session, int event)
{
    if (event == WEBNET_EVENT_URI_PHYSICAL)
    {
        int index;
        struct webnet_request* request;

        RT_ASSERT(session != RT_NULL);
        request = session->request;
        RT_ASSERT(request != RT_NULL);

        /* check whether the uri is a alias */
        for (index = 0; index < _alias_item_count; index ++)
        {
            if (str_begin_with(request->path, _alias_items[index].old_path))
            {
                char* map_path;
                map_path = (char*) wn_malloc (WEBNET_PATH_MAX);
                RT_ASSERT(map_path != RT_NULL);

                rt_snprintf(map_path, WEBNET_PATH_MAX, "%s/%s",
                            _alias_items[index].new_path,
                            request->path + strlen(_alias_items[index].old_path));

                /* set new path */
                wn_free(request->path);
                request->path = map_path;

                return WEBNET_MODULE_CONTINUE;
            }
        }
    }

    return WEBNET_MODULE_CONTINUE;
}
#endif

