//  The MIT License (MIT)
//  Copyright (c) 2018 liu2guang <liuguang@rt-thread.com>
 
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
 
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
 
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
//  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
//  OR OTHER DEALINGS IN THE SOFTWARE.

#include <rtthread.h>
#include <librws.h>
#include <string.h>
#include <stdlib.h>

#if (RTTHREAD_VERSION <= 30100) 
#define DBG_SECTION_NAME "[LIBRWS.Test] "
#else
#define DBG_SECTION_NAME "LIBRWS.Test"
#endif 
#define DBG_ENABLE
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

struct librws_app
{
    rt_bool_t init; 
    rws_socket *socket; 
}; 
typedef struct librws_app* librws_app_t; 

static struct librws_app _app = {0}; 
static librws_app_t app = &_app; 

static void onopen(rws_socket socket) 
{
    LOG_D("websocket connected. ");
}

static void onclose(rws_socket socket) 
{
    rws_error error = rws_socket_get_error(socket);

    if(error)
    {
        LOG_E("websocket disconnect, error: %i, %s ", rws_error_get_code(error), rws_error_get_description(error));
    }
    else
    {
        LOG_D("websocket disconnect! ");
    }
    
    rws_socket_disconnect_and_release(app->socket); 
    
    app->init = RT_FALSE; 
}

static void onmessage_text(rws_socket socket, const char *text, const unsigned int len) 
{
	char *buff = RT_NULL;
    
    buff = (char *)rt_malloc(2048); 
    
    rt_memset(buff, 0x00, 2048); 
    rt_memcpy(buff, text, len);

	LOG_D("message(txt), %d(byte): %s ", len, buff);
    
    if(buff != RT_NULL)
    { 
        rt_free(buff); 
    }
}

static void onmessage_bin(rws_socket socket, const void * data, const unsigned int len)
{
	char *buff = RT_NULL;
    
    buff = (char *)rt_malloc(2048); 
    
    rt_memset(buff, 0x00, 2048); 
    rt_memcpy(buff, data, len);

	LOG_D("message(bin), %d(byte): %s ", len, buff);
    
    if(buff != RT_NULL)
    {
        rt_free(buff); 
    }
}

// rws_connect ws echo.websocket.org 80
// rws_connect wss echo.websocket.org 443
// rws_send "hello tls!"
static int _rws_connect(int argc, char *argv[])
{
    int port = -1; 
    rws_bool ret = rws_false; 

    if(argc < 3)
    {
        LOG_E("the msh cmd format: rws_conn ws/wss host [port]. "); 
        return RT_EOK; 
    }
    
    if(app->init == RT_TRUE)
    {
        LOG_E("the websocket connection has been opened. "); 
        return (-RT_EBUSY); 
    }

    app->socket = rws_socket_create();
    if(app->socket == RT_NULL)
    {
        LOG_E("librws socket create failed. "); 
        return (-RT_ERROR); 
    }
    
    if(strcmp(argv[1], "ws") == 0)
    {
        port = ((argv[3] == RT_NULL) ? (80) : (atoi(argv[3]))); 
        rws_socket_set_url(app->socket, "ws", argv[2], port, "/"); 
    }
    else if(strcmp(argv[1], "wss") == 0)
    {
        port = ((argv[3] == RT_NULL) ? (443) : (atoi(argv[3]))); 
        rws_socket_set_url(app->socket, "wss", argv[2], port, "/"); 
    }
    else
    {
        LOG_E("protocol types are not supported, only support ws/wss. "); 
        return (-RT_EINVAL); 
    }
    
    rws_socket_set_on_connected(app->socket, &onopen);
    rws_socket_set_on_disconnected(app->socket, &onclose); 
    rws_socket_set_on_received_text(app->socket, &onmessage_text);
    rws_socket_set_on_received_bin(app->socket, &onmessage_bin);
    
    /* set custom mode */ 
    // rws_socket_set_custom_mode(app->socket);  
    
    ret = rws_socket_connect(app->socket); 
    if(ret == rws_false)
    {
        if(strcmp(argv[1], "ws") == 0)
        {
            LOG_E("connect %s://%s:%d/ failed. ", argv[1], argv[2], port); 
        }
        else if(strcmp(argv[1], "wss") == 0)
        {
            LOG_E("connect %s://%s:%d/ failed. ", argv[1], argv[2], port); 
        }
    }
    else
    {
        if(strcmp(argv[1], "ws") == 0)
        {
            LOG_D("try connect %s://%s:%d/ ", argv[1], argv[2], port); 
        }
        else if(strcmp(argv[1], "wss") == 0)
        {
            LOG_D("try connect %s://%s:%d/ ", argv[1], argv[2], port); 
        }
    }
    
    app->init = RT_TRUE; 

    return RT_EOK; 
}
MSH_CMD_EXPORT_ALIAS(_rws_connect, rws_connect, websocket connect); 

static int _rws_disconnect(int argc, char *argv[])
{
    if(app->init == RT_FALSE)
    {
        LOG_W("no websocket connection. "); 
        return (-RT_ERROR); 
    }
    
    rws_socket_disconnect_and_release(app->socket); 
    LOG_I("try disconnect websocket connection. "); 
    app->socket = RT_NULL; 
    
    app->init = RT_FALSE; 
    
    return RT_EOK; 
}
MSH_CMD_EXPORT_ALIAS(_rws_disconnect, rws_disconnect, websocket disconnect); 

static int _rws_send(int argc, char *argv[])
{
    if(argc == 1)
    {
        LOG_E("The command format: rws_send \"content\". "); 
        return RT_EOK; 
    }
    
    if(app->init == RT_FALSE)
    {
        LOG_W("no websocket connection. "); 
        return (-RT_ERROR); 
    }
    
    LOG_W("string = %s, len = %d", argv[1], rt_strlen(argv[1])); 

    rws_socket_send_text(app->socket, argv[1]); 
    
    return RT_EOK; 
}
MSH_CMD_EXPORT_ALIAS(_rws_send, rws_send, websocket send text); 
