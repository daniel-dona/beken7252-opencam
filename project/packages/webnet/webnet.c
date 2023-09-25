/*
 * File      : webnet.c
 * This file is part of RT-Thread RTOS/WebNet Server
 * COPYRIGHT (C) 2011, Shanghai Real-Thread Technology Co., Ltd
 *
 * All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-08-02     Bernard      the first version
 * 2012-07-03     Bernard      Add webnet port and webroot setting interface.
 */

#include <rtthread.h>
#include <stdint.h>
#include <string.h>

#include "webnet.h"
#include "wn_system.h"
#include "session.h"
#include "module.h"

static rt_uint16_t webnet_port = WEBNET_PORT;
static char webnet_root[64] = WEBNET_ROOT;
void webnet_set_port(int port)
{
    webnet_port = port;
}

int webnet_get_port(void)
{
    return webnet_port;
}

void webnet_set_root(const char* webroot_path)
{
    strncpy(webnet_root, webroot_path, sizeof(webnet_root) - 1);
    webnet_root[sizeof(webnet_root) - 1] = '\0';
}

const char* webnet_get_root(void)
{
    return webnet_root;
}

/**
 * webnet thread entry
 */
static void webnet_thread(void *parameter)
{
    int listenfd = -1;
    fd_set readset, tempfds;
    fd_set writeset, tempwrtfds;
    int sock_fd, maxfdp1;
    struct sockaddr_in webnet_saddr;

    /* First acquire our socket for listening for connections */
    listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenfd < 0)
    {
        rt_kprintf("webnet: create socket failed.\n");
        goto __exit;
    }

    memset(&webnet_saddr, 0, sizeof(webnet_saddr));
    webnet_saddr.sin_family = AF_INET;
    webnet_saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    webnet_saddr.sin_port = htons(webnet_port); /* webnet server port */

    if (bind(listenfd, (struct sockaddr *) &webnet_saddr,
            sizeof(webnet_saddr)) == -1)
    {
        rt_kprintf("webnet: bind socket failed, errno=%d\n", errno);
        goto __exit;
    }

    /* Put socket into listening mode */
    if (listen(listenfd, MAX_SERV) == -1)
    {
        rt_kprintf("webnet: socket listen failed.\n");
        goto __exit;
    }
	
#ifdef RT_USING_DFS
    /* initialize module (no session at present) */
    webnet_module_handle_event(RT_NULL, WEBNET_EVENT_INIT);
#endif

    /* Wait forever for network input: This could be connections or data */
    for (;;)
    {
        /* Determine what sockets need to be in readset */
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(listenfd, &readset);

        /* set fds in each sessions */
        maxfdp1 = webnet_sessions_set_fds(&readset, &writeset);
        if (maxfdp1 < listenfd + 1)
            maxfdp1 = listenfd + 1;

        /* use temporary fd set in select */
        tempfds = readset;
        tempwrtfds = writeset;
        /* Wait for data or a new connection */
        sock_fd = select(maxfdp1, &tempfds, &tempwrtfds, 0, 0);
        if (sock_fd == 0)
            continue;

        /* At least one descriptor is ready */
        if (FD_ISSET(listenfd, &tempfds))
        {
            struct webnet_session* accept_session;
            /* We have a new connection request */
            accept_session = webnet_session_create(listenfd);
            if (accept_session == RT_NULL)
            {
                /* create session failed, just accept and then close */
                int sock;
                struct sockaddr cliaddr;
                socklen_t clilen;

                clilen = sizeof(struct sockaddr_in);
                sock = accept(listenfd, &cliaddr, &clilen);
                if (sock >= 0)
                    closesocket(sock);
            }
            else
            {
                /* add read fdset */
                FD_SET(accept_session->socket, &readset);
            }
        }

        webnet_sessions_handle_fds(&tempfds, &writeset);
    }

__exit:
    if (listenfd >= 0)
        closesocket(listenfd);
}

void webnet_init(void)
{
#if 0
    rt_err_t result;
    char * stack;
    int stack_size = WEBNET_THREAD_STACKSIZE;
	static struct rt_thread webnet_tid;

    stack = wn_malloc(stack_size);
    RT_ASSERT(stack != RT_NULL);

    result = rt_thread_init(&webnet_tid,
                WEBNET_THREAD_NAME,
                webnet_thread,
                RT_NULL,
                stack,
                stack_size,
                WEBNET_PRIORITY,
                5);

    if (result == RT_EOK)
        rt_thread_startup(&webnet_tid);
    else
        rt_kprintf("create webnet thread failed!\n");
#else
    rt_thread_t tid;

    tid = rt_thread_create(WEBNET_THREAD_NAME,
                           webnet_thread, RT_NULL,
                           WEBNET_THREAD_STACKSIZE, WEBNET_PRIORITY, 5);

    if (tid != RT_NULL) rt_thread_startup(tid);
#endif
}


#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(webnet_init, webnet server init);
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(webnet_init, webnet server init);
#endif
#endif