/*
 * File      : session.h
 * This file is part of RT-Thread RTOS/WebNet Server
 * COPYRIGHT (C) 2011, Shanghai Real-Thread Technology Co., Ltd
 *
 * All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-08-02     Bernard      the first version
 */
#ifndef __WEBNET_SESSION_H__
#define __WEBNET_SESSION_H__

#include <rtthread.h>

#include <webnet.h>
#include <request.h>

#if defined(RT_USING_POSIX) && defined(RT_USING_DFS_NET)
#include <sys/socket.h>
#include <sys/select.h>
#else
#include <lwip/sockets.h>
#endif

#define WEBNET_SESSION_BUFSZ	(4 * 1024)

/* close session */
#define WEBNET_EVENT_CLOSE			(1 << 5)
/* read  session */
#define WEBNET_EVENT_READ			(1 << 6)
/* write session */
#define WEBNET_EVENT_WRITE			(1 << 7)


struct webnet_session_ops
{
    void (*session_handle)(struct webnet_session* session, int event);
    void (*session_close) (struct webnet_session* session);
};

enum webnet_session_phase
{
	WEB_PHASE_METHOD = 0, 	/* parse method */
	WEB_PHASE_HEADER, 		/* handle web request header */
	WEB_PHASE_QUERY,        /* handle web query */
	WEB_PHASE_RESPONSE,		/* handle web response */
	WEB_PHASE_CLOSE,		/* to close session */
};

struct webnet_session
{
    struct webnet_session *next;

    /* socket information */
    int socket;
    struct sockaddr_in cliaddr;

    /* webnet request */
    struct webnet_request* request;

    /* session buffer */
    rt_uint16_t buffer_length;
    rt_uint16_t buffer_offset;
    rt_uint8_t  buffer[WEBNET_SESSION_BUFSZ];

	/* session phase */
	rt_uint32_t  session_phase;

	rt_uint32_t  session_event_mask;
    const struct webnet_session_ops* session_ops;
    rt_uint32_t user_data;
};

struct webnet_session* webnet_session_create(int listenfd);

int  webnet_session_read(struct webnet_session *session, char *buffer, int length);
void webnet_session_close(struct webnet_session *session);

void webnet_session_printf(struct webnet_session *session, const char* fmt, ...);
int  webnet_session_write(struct webnet_session *session, const rt_uint8_t* data, rt_size_t size);
int  webnet_session_redirect(struct webnet_session *session, const char* url);
int  webnet_session_get_physical_path(struct webnet_session *session, const char* virtual_path, char* full_path);
void webnet_session_set_header(struct webnet_session *session, const char* mimetype, int code, const char* status, int length);
void webnet_session_set_header_status_line(struct webnet_session *session, int code, const char * reason_phrase);

int webnet_sessions_set_fds(fd_set *readset, fd_set *writeset);
void webnet_sessions_handle_fds(fd_set *readset, fd_set *writeset);

#endif

