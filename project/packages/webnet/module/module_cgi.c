/*
 * File      : module_cgi.c
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
#include <string.h>

#ifdef WEBNET_USING_CGI
#include <dfs_posix.h>

#include "module.h"
#include "util.h"

#define CGI_ROOT_PATH_MAX	64
static char _cgi_root[CGI_ROOT_PATH_MAX] = {0};
struct webnet_cgi_item
{
    const char* name;
    void (*handler)(struct webnet_session* session);
};
static struct webnet_cgi_item* _cgi_items = RT_NULL;
static rt_uint32_t _cgi_count = 0;

void webnet_cgi_set_root(const char* root)
{
    if (strlen(root) > CGI_ROOT_PATH_MAX)
    {
        RT_ASSERT(0);
        return;
    }

    strncpy(_cgi_root, root, strlen(root));
    if (_cgi_root[strlen(_cgi_root)] != '/')
    {
        _cgi_root[strlen(_cgi_root) + 1] = '/';
        _cgi_root[strlen(_cgi_root) + 1] = '\0';
    }
}
RTM_EXPORT(webnet_cgi_set_root);

void webnet_cgi_register(const char* name, void (*handler)(struct webnet_session* session))
{
    if (_cgi_items == RT_NULL)
    {
        _cgi_count = 1;
        _cgi_items = (struct webnet_cgi_item*) wn_malloc (sizeof(struct webnet_cgi_item) * _cgi_count);
    }
    else
    {
        _cgi_count += 1;
        _cgi_items = (struct webnet_cgi_item*) wn_realloc (_cgi_items, sizeof(struct webnet_cgi_item) * _cgi_count);
    }

    RT_ASSERT(_cgi_items != RT_NULL);
    _cgi_items[_cgi_count - 1].name = name;
    _cgi_items[_cgi_count - 1].handler = handler;
}
RTM_EXPORT(webnet_cgi_register);

int webnet_module_cgi(struct webnet_session* session, int event)
{
    if (event == WEBNET_EVENT_INIT)
    {
        /* set default cgi path */
        if (_cgi_root[0] == '\0')
        {
            strcpy(_cgi_root, "/cgi-bin/");
        }
    }
    else if (event == WEBNET_EVENT_URI_PHYSICAL)
    {
        struct webnet_request* request;

        RT_ASSERT(session != RT_NULL);
        request = session->request;
        RT_ASSERT(request != RT_NULL);
        /* check whether a cgi request */
        if (str_begin_with(request->path, _cgi_root))
        {
            char* cgi_name;
            rt_uint32_t index;

            cgi_name = request->path + strlen(_cgi_root);
            for (index = 0; index < _cgi_count; index ++)
            {
                if (strncasecmp(cgi_name, _cgi_items[index].name, strlen(_cgi_items[index].name)) == 0)
                {
                    /* found it */
                    _cgi_items[index].handler(session);
                    return WEBNET_MODULE_FINISHED;
                }
            }

            /* set 404 not found error */
            request->result_code = 404;
        }
    }

    return WEBNET_MODULE_CONTINUE;
}
#endif
