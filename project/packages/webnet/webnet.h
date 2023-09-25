/*
 * File      : webnet.h
 * This file is part of RT-Thread RTOS/WebNet Server
 * COPYRIGHT (C) 2011, Shanghai Real-Thread Technology Co., Ltd
 *
 * All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-08-02     Bernard      the first version
 *
 * Release 1.0.3
 * 2012-09-15     Bernard      fixed basic authentication issue in Safari.
 * Release 1.0.4
 * 2012-11-08     Bernard      fixed the buffer issue in FireFox upload.
 * Release 1.0.5
 * 2012-12-10     aozima       fixed small file upload issue.
 * Release 1.0.6
 * 2012-12-17     Bernard      fixed the content multi-transmission issue in POST.
 * Release 1.0.7
 * 2013-03-01     aozima       add feature: add Last-Modified and Cache-Control.
 * Release 1.0.8
 * 2015-04-17     Bernard      Use select for write socket
 * Release 1.1.0
 * 2015-05-01     aozima       support deflate gzip.
 * Release 2.0.0
 * 2016-07-31     Bernard      Rewrite http read handling and change version to 2.0.0
 */

#ifndef __WEBNET_H__
#define __WEBNET_H__

#include <rtthread.h>

/* webnet configuration */
#ifdef PKG_WEBNET_USING_KEEPALIVE
#define WEBNET_USING_KEEPALIVE
#endif
#ifdef PKG_WEBNET_USING_COOKIE
#define WEBNET_USING_COOKIE
#endif
#ifdef PKG_WEBNET_USING_AUTH
#define WEBNET_USING_AUTH
#endif
#ifdef PKG_WEBNET_USING_CGI
#define WEBNET_USING_CGI
#endif
#ifdef PKG_WEBNET_USING_ASP
#define WEBNET_USING_ASP
#endif
#ifdef PKG_WEBNET_USING_INDEX
#define WEBNET_USING_INDEX
#endif
#ifdef PKG_WEBNET_USING_LOG
#define WEBNET_USING_LOG
#endif
#ifdef PKG_WEBNET_USING_ALIAS
#define WEBNET_USING_ALIAS
#endif
#ifdef PKG_WEBNET_USING_UPLOAD
#define WEBNET_USING_UPLOAD
#endif
#ifdef PKG_WEBNET_USING_SSI
#define WEBNET_USING_SSI
#endif
#ifdef PKG_WEBNET_USING_DAV
#define WEBNET_USING_DAV
#endif
#ifdef PKG_WEBNET_USING_DMR
#define WEBNET_USING_DMR
#endif
#ifdef PKG_WEBNET_USING_LUA
#define WEBNET_USING_LUA
#endif
#ifdef PKG_WEBNET_USING_SSL
#define WEBNET_USING_SSL
#endif

#define MAX_SERV                 16			/* Maximum number of webnet services. Don't need too many */

#define WEBNET_VERSION			"2.0.0"		/* webnet version string */
#define WEBNET_THREAD_NAME      "webnet"	/* webnet thread name */

#define WEBNET_PRIORITY         20			/* webnet thread priority */
#define WEBNET_THREAD_STACKSIZE (4 * 1024)		/* webnet thread stack size */
#define WEBNET_PORT				80			/* webnet server listen port */
//#define WEBNET_ROOT				"/flash/webnet"	/* webnet server root directory */
#define WEBNET_SERVER			"Server: webnet "WEBNET_VERSION"\r\n"
#define WEBNET_PATH_MAX			256			/* maxiaml path length in webnet */
#define WEBNET_CACHE_LEVEL      0           /* 0:disable, 1:use Last-Modified, 2: use Cache-Control. */
#define WEBNET_CACHE_MAX_AGE    1800        /* unit: second(valid when level = 2). */
#define WEBNET_DEFLATE_GZIP     1           /* 0:disable, 1:enable. */

#ifdef WEBNET_USING_LOG
#define WEBNET_PRINTF   rt_kprintf("[WEBNET] ");rt_kprintf
#else
#define WEBNET_PRINTF(...)
#endif /* WEBNET_USING_LOG */

#include "wn_system.h"

void webnet_set_port(int port);
int webnet_get_port(void);
void webnet_set_root(const char* webroot_path);
const char* webnet_get_root(void);

void webnet_init(void);

/* Pre-declaration */
struct webnet_session;
/* webnet query item definitions */
struct webnet_query_item
{
    char* name;
    char* value;
};

/* WebNet APIs */
/* get mimetype according to URL */
const char* mime_get_type(const char* url);

/* add an ASP variable */
void webnet_asp_add_var(const char* name, void (*handler)(struct webnet_session* session));
void webnet_cgi_set_root(const char* root);
/* register a cgi */
void webnet_cgi_register(const char* name, void (*handler)(struct webnet_session* session));
void webnet_auth_set(const char* path, const char* username_password);

#endif
