/*
 * File      : module_dav.c
 * This file is part of RT-Thread RTOS/WebNet Server
 * COPYRIGHT (C) 2011, Shanghai Real-Thread Technology Co., Ltd
 *
 * All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-9-9       Coing        the first version
 */
#include <webnet.h>
#include <dfs_posix.h>
#include "util.h"

#include "session.h"
#include "module.h"

#ifdef WEBNET_USING_DAV

// #define RT_DBG_DAV

#ifdef RT_DBG_DAV
#define dav_dbg         rt_kprintf("[WebDAV] ");rt_kprintf
#else
#define dav_dbg(...)
#endif /* RT_DBG_DAV */

struct webnet_module_put_entry
{
    int (*put_open) (struct webnet_session* session);
    int (*put_close)(struct webnet_session* session);
    int (*put_write)(struct webnet_session* session, const void* data, rt_size_t length);
    int (*put_done) (struct webnet_session* session);
};

struct webnet_module_put_session
{
    rt_uint16_t file_opened;

    /* put entry */
    const struct webnet_module_put_entry* entry;

    rt_uint32_t user_data;
};

static const char*  propfind_element ="<d:response>"
                                       "<d:href>%s</d:href>"
                                       "<d:propstat>"
                                       "<d:prop>"
                                       "<d:resourcetype>%s</d:resourcetype>"
                                       "<d:getcontentlength>%d</d:getcontentlength>"
                                       "<d:getlastmodified>%s</d:getlastmodified>" // Sat, 05 Sep 2015 09:47:53 GMT
                                       "</d:prop>"
                                       "<d:status>HTTP/1.1 200 OK</d:status>"
                                       "</d:propstat>"
                                       "</d:response>\n";

static const void* webnet_put_get_userdata(struct webnet_session* session);
int webnet_module_put_method(struct webnet_session* session);
static void print_propfind_element(struct webnet_session *session, const char *uri,struct stat *stp);

int webnet_module_dav(struct webnet_session* session, int event)
{
    if (event != WEBNET_EVENT_URI_POST)
        return WEBNET_MODULE_CONTINUE;

    if(session->request->method == WEBNET_OPTIONS)
    {
        static const char* status = "Allow: GET, POST, HEAD, PUT, DELETE, OPTIONS, PROPFIND, MKCOL\r\nDAV: 1\r\n\r\n";

        dav_dbg("OPTIONS %s\n", session->request->path);

        session->request->result_code = 200;
        webnet_session_set_header_status_line(session, session->request->result_code, "OK");
        webnet_session_printf(session, status);

        return WEBNET_MODULE_FINISHED;
    }
    else if(session->request->method == WEBNET_PROPFIND)
    {
        int fd;
        DIR *dir;
        rt_uint8_t exit_file = 0;
        struct stat file_stat;

        const char* parent_path;

        static const char header[] = "HTTP/1.1 207 Multi-Status\r\n"
                                     "Connection: close\r\n"
                                     "Content-Type: text/xml; charset=utf-8\r\n\r\n"
                                     "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                     "<d:multistatus xmlns:d='DAV:'>\n";

        static const char footer[] = "</d:multistatus>";

        char *depth = session->request->depth;

        dav_dbg("PROPFIND %s depth: %s\n", session->request->path,
            session->request->depth?session->request->depth:"null");

        fd = open(session->request->path, O_RDONLY, 0);
        if (fd >= 0)
        {
            close(fd);
            exit_file = 1;
        }
        dir = opendir(session->request->path);
        if(dir != RT_NULL)
        {
            closedir(dir);
            exit_file = 1;
        }
        if(!exit_file)
        {
            rt_kprintf("no file\n");
            session->request->result_code = 404;
            return WEBNET_MODULE_FINISHED;
        }

        webnet_session_printf(session, header);

        parent_path = session->request->path + strlen(webnet_get_root());

        /* output the parent. */
        file_stat.st_size = 0;
        file_stat.st_mtime = 0;
        file_stat.st_mode = S_IFDIR;
        print_propfind_element(session, parent_path, &file_stat);

        /* depth: 0, 1, infinity. */
        if( (depth == NULL || (strcmp(depth, "0") != 0) ) )
        {
            struct dirent* dirent;
            char *fullpath;

            fullpath = wn_malloc (WEBNET_PATH_MAX);
            rt_memset(&file_stat, 0, sizeof(struct stat));

            //eg. dir = opendir("/webnet/SD");
            dir = opendir(session->request->path);

            while((dirent = readdir(dir)) != NULL)
            {
                /* build full path for each file */
                rt_sprintf(fullpath, "%s/%s",session->request->path, dirent->d_name);

                str_normalize_path(fullpath);

                stat(fullpath, &file_stat);

                print_propfind_element(session, dirent->d_name,&file_stat);
            }
            closedir(dir);
            wn_free(fullpath);
        }

        webnet_session_printf(session,footer);

        return WEBNET_MODULE_FINISHED;
    }
    else if(session->request->method == WEBNET_PUT)
    {
        dav_dbg("PUT %s\n", session->request->path);
        webnet_module_put_method(session);
        return WEBNET_MODULE_FINISHED;
    }
    else if(session->request->method == WEBNET_PROPPATCH)
    {
        int proppatch_length = 0;

        static const char Proppatch_header[] = "HTTP/1.1 207 Multi-Status\r\n"
                                               //"Date: Mon, 07 Sep 2015 01:50:42 GMT\r\n"
                                               "Server: %s %s\r\n"  //server名
                                               "Content-Length: %d \r\n"    //长度
                                               "Content-Type: text/xml; charset=\"utf-8\"\r\n\r\n";

        static const char Proppatch_ex[] =  "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                            "<D:multistatus xmlns:D=\"DAV:\" xmlns:ns0=\"DAV:\">"
                                            "<D:response>"
                                            "<D:href>%s</D:href>"   //地址
                                            "<D:propstat>"
                                            "<D:prop><ns0:getlastmodified/>"
                                            "</D:prop>"
                                            "<D:status>HTTP/1.1 409 (status)</D:status>"
                                            "<D:responsedescription>"
                                            "Property is read-only.</D:responsedescription>"
                                            "</D:propstat>"
                                            "</D:response>"
                                            "</D:multistatus>";

        dav_dbg("PROPPATCH %s\n", session->request->path);

        proppatch_length = strlen(Proppatch_ex)+strlen(session->request->path)-2;

        webnet_session_printf(session,Proppatch_header,WEBNET_THREAD_NAME,WEBNET_VERSION,proppatch_length);

        webnet_session_printf(session,Proppatch_ex,session->request->path);

        return WEBNET_MODULE_FINISHED;
    }
    else if(session->request->method == WEBNET_DELETE)
    {
        dav_dbg("DELETE %s\n", session->request->path);

        unlink(session->request->path);

        webnet_session_printf(session,"HTTP/1.1 204 No Content\r\n"
                              "Server: webnet\r\n"
                              "Content-Length: 0\r\n"
                              "Content-Type: text/plain\r\n\r\n");

        return WEBNET_MODULE_FINISHED;
    }
    else if(session->request->method == WEBNET_MKCOL)
    {
        int ret;

        dav_dbg("MKCOL %s\n", session->request->path);

        ret = mkdir(session->request->path,0x777);
        if(ret < 0)
        {
            session->request->result_code = 404;
            rt_kprintf("mkdir error !\n");
        }
        else
        {
            dav_dbg("mkdir ok !\n");
        }

        webnet_session_printf(session,"HTTP/1.1 201 Created\r\n"
                              "Server: webnet\r\n"
                              "Content-Length: 0\r\n"
                              "Content-Type: text/plain\r\n\r\n");

        return WEBNET_MODULE_FINISHED;
    }
    else if (session->request->method == WEBNET_MOVE)
    {
        char *path, *full_path;

        path = strstr(session->request->destination, session->request->host);
        if (path)
        {
            path += strlen(session->request->host);

            full_path = (char*) wn_malloc (WEBNET_PATH_MAX);
            if (full_path)
            {
                webnet_session_get_physical_path(session, path, full_path);

                dav_dbg("%s => %s\n", session->request->path, full_path);
                rename(session->request->path, full_path);

                wn_free(full_path);
            }
        }

        webnet_session_printf(session,"HTTP/1.1 200 OK\r\n"
                              "Server: webnet\r\n"
                              "Content-Length: 0\r\n"
                              "Content-Type: text/plain\r\n\r\n");

        return WEBNET_MODULE_FINISHED;
    }

    return WEBNET_MODULE_CONTINUE;
}

#include "time.h"

static void print_propfind_element(struct webnet_session *session, const char *uri, struct stat *file_stat)
{
    char ctime_str[64];

    time_t t = file_stat->st_mtime;

    rt_enter_critical();
    strftime(ctime_str, sizeof(ctime_str), "%a, %d %b %Y %H:%M:%S GMT", localtime(&t));
    rt_exit_critical();
    //dav_dbg("strftime:  %s\n", ctime_str);

    webnet_session_printf(session,
                          propfind_element,
                          uri, S_ISDIR(file_stat->st_mode) ? "<d:collection/>" : "",
                          file_stat->st_size, ctime_str);
}

static int put_open (struct webnet_session* session)
{
    int fd;
    if(session->request->path == RT_NULL)
    {
        fd = -1;
        goto _exit;
    }

    fd = open(session->request->path, O_WRONLY | O_CREAT | O_TRUNC, 0);
    if(fd < 0)
    {
        session->session_phase = WEB_PHASE_CLOSE;
        fd = -1;
        goto _exit;
    }

    return fd;

_exit:
    rt_kprintf(" %s failed  ,file:%s  ,line:%d\n",__FUNCTION__,__FILE__,__LINE__);
    return (int)fd;
}

static int put_close(struct webnet_session* session)
{

    int fd;

    fd = (int)webnet_put_get_userdata(session);
    if (fd < 0) return 0;

    close(fd);
    return 0;
}

static int put_done (struct webnet_session* session)
{
    int put_done_h_length = 0;

    static const char put_done_h[] = "HTTP/1.1 201 Created\r\n"
                                     "Date: Mon, 07 Sep 2015 01:50:42 GMT\r\n"
                                     "Server: %s %s\r\n"        //server名
                                     "Location: http://%s%s \r\n"       //文件网络地址
                                     "Content-Length: %d \r\n"  //put_done_t 长度  71
                                     "Content-Type: text/html; charset=ISO-8859-1\r\n\r\n";

    static const char put_done_t[] ="<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">"
                                    "<html><head>"
                                    "<title>";

    dav_dbg("put_done \n");

    put_done_h_length = strlen(put_done_t);

    webnet_session_printf(session,put_done_h,WEBNET_THREAD_NAME,WEBNET_VERSION,inet_ntoa(session->cliaddr.sin_addr),session->request->path,put_done_h_length);

    webnet_session_printf(session,put_done_t);


    return 0;
}

static int put_write(struct webnet_session* session, const void* data, rt_size_t length)
{
    int fd;

    dav_dbg("put_write \n");

    // get fd
    fd = (int)webnet_put_get_userdata(session);
    if (fd < 0) return 0;

    write(fd, data, length);

    return length;
}

static const void* webnet_put_get_userdata(struct webnet_session* session)
{
    struct webnet_module_put_session *put_session;

    /* get put session */
    put_session = (struct webnet_module_put_session *)session->user_data;
    if (put_session == RT_NULL) return RT_NULL;

    return (const void*) put_session->user_data;
}

static void _webnet_module_put_close(struct webnet_session* session)
{
    struct webnet_module_put_session *put_session;

    /* get put session */
    put_session = (struct webnet_module_put_session *)session->user_data;
    if (put_session == RT_NULL) return;
    /* close file */
    if (put_session->file_opened == 1)
    {
        put_session->entry->put_close(session);
        put_session->file_opened = 0;
    }

    wn_free(put_session);
    /* remove private data */
    session->user_data = 0;
    session->session_ops = RT_NULL;
}

static void _webnet_module_put_handle(struct webnet_session* session, int event)
{
    if(session->request->method == WEBNET_PUT)
    {
        int length = 0;
        static int read_bytes = 0;

        struct webnet_module_put_session *put_session;

        /* get put session    这个webnet_session的 user_data是 put_session*/
        put_session = (struct webnet_module_put_session *)session->user_data;

        //读数据流，创建文件(第一次)， 写入文件，(写完)关闭文件， 发送201，关闭连接ok
        /* read stream */
        length = webnet_session_read(session, session->buffer, sizeof(session->buffer) - 1);
        /* connection break out */
        if (length <= 0)
        {
            rt_kprintf(" %s failed  ,file:%s  ,line:%d\n",__FUNCTION__,__FILE__,__LINE__);

            /* read stream failed (connection break out), close this session */
            session->session_phase = WEB_PHASE_CLOSE;
            return;
        }
        read_bytes = read_bytes + length;

        session->buffer[length] = '\0';

        /* open file  第一次*/
        if (put_session->file_opened == 0)
        {
            /* open file */
            put_session->user_data = put_session->entry->put_open(session);
            put_session->file_opened = 1;
        }

        //write data
        if (length > 0 && session->buffer != NULL)
        {
            put_session->entry->put_write(session, (char*)session->buffer, length);
        }

        //判断是否写完,通过长度来判断
        if (read_bytes >= (session->request->content_length))
        {
            put_session->entry->put_done(session);
            _webnet_module_put_close(session);

            read_bytes = 0;
        }
    }
}

static const struct webnet_session_ops _put_ops =
{
    _webnet_module_put_handle,
    _webnet_module_put_close
};

const struct webnet_module_put_entry put_entry_put =
{
    put_open,
    put_close,
    put_write,
    put_done
};

int webnet_module_put_method(struct webnet_session* session)
{
    const struct webnet_module_put_entry *entry = &put_entry_put;
    struct webnet_module_put_session *put_session;

    /* create a uploading session */
    put_session = (struct webnet_module_put_session*) wn_malloc (sizeof (struct webnet_module_put_session));
    //TODO: null check.

    put_session->file_opened = 0;
    put_session->entry = entry;
    put_session->user_data = 0;

    /* add this put session into webnet session */
    session->user_data = (rt_uint32_t) put_session;
    /* set webnet session operations */
    session->session_ops = &_put_ops;

    return WEBNET_MODULE_FINISHED;
}

#endif /* WEBNET_USING_DAV */
