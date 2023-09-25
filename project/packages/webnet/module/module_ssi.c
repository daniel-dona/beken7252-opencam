/*
 * File      : module_ssl.c
 * This file is part of RT-Thread RTOS/WebNet Server
 * COPYRIGHT (C) 2011, Shanghai Real-Thread Technology Co., Ltd
 *
 * All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-06-25     Bernard      the first version
 */
#include <webnet.h>
#include <dfs_posix.h>
#include "session.h"
#include "module.h"

#if defined(WEBNET_USING_SSI)

#define SSI_INCLUDE_STRING	"<!--#include "
#define SSI_EXEC_STRING		"<!--#exec "
#define SSI_VIRTUAL_STRING	"virtual=\""
#define SSI_FILE_STRING		"file=\""
#define SSI_CGI_STRING		"cgi=\""
#define SSI_END_STRING		"\" -->"

static void _webnet_ssi_sendfile(struct webnet_session* session, const char* filename)
{
    int fd;
    int file_length;
    rt_size_t size, readbytes;

    fd = open(filename, O_RDONLY, 0);
    if (fd < 0) return; /* open file failed */

    /* get file size */
    file_length = lseek(fd, 0, SEEK_END);
    /* seek to beginning of file */
    lseek(fd, 0, SEEK_SET);
    if (file_length <= 0)
    {
        close(fd);
        return ;
    }
    while (file_length)
    {
        if (file_length > sizeof(session->buffer))
            size = (rt_size_t) sizeof(session->buffer);
        else
            size = file_length;

        readbytes = read(fd, session->buffer, size);
        if (readbytes <= 0)
            /* no more data */
            break;

        if (webnet_session_write(session, session->buffer, readbytes) == 0)
            break;

        file_length -= (long) readbytes;
    }

    /* close file */
    close(fd);
}

static void _webnet_ssi_dofile(struct webnet_session* session, int fd)
{
    char *ssi_begin, *ssi_end;
    char *offset, *end;
    char *buffer;
    char *path;
    rt_uint32_t length;

    ssi_begin = ssi_end = RT_NULL;
    offset = end = RT_NULL;
    buffer = path = RT_NULL;

    /* get file length */
    length = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    /* allocate ssi include file path */
    path = (char*) wn_malloc(WEBNET_PATH_MAX);
    /* allocate read buffer */
    buffer = (char*) wn_malloc (length);
    if (path == RT_NULL || buffer == RT_NULL)
    {
        session->request->result_code = 500;
        goto __exit;
    }

    /* write page header */
    webnet_session_set_header(session, "text/html", 200, "OK", -1);
    /* read file to buffer */
    if (read(fd, buffer, length) != length) /* read failed */
    {
        session->request->result_code = 500;
        close(fd);
        goto __exit;
    }
    /* close file */
    close(fd);

    offset = buffer;
    end = buffer + length;
    while (offset < end)
    {
        /* get beginning of ssi */
        ssi_begin = strstr(offset, SSI_INCLUDE_STRING);
        if (ssi_begin == RT_NULL)
        {
            /* write content directly */
            webnet_session_write(session, (const rt_uint8_t*)offset, end - offset);
            break;
        }

        /* get end of ssi */
        ssi_end = strstr(ssi_begin, "\" -->");
        if (ssi_end == RT_NULL)
        {
            /* write content directly */
            webnet_session_write(session, (const rt_uint8_t*)offset, end - offset);
            break;
        }
        else
        {
            char *include_begin, *include_end;
            char *filename;

            /* write content */
            webnet_session_write(session, (const rt_uint8_t*)offset, ssi_begin - offset);

            offset = ssi_begin + sizeof(SSI_INCLUDE_STRING) - 1;
            include_begin = strstr(ssi_begin, SSI_VIRTUAL_STRING);
            if (include_begin != RT_NULL)
            {
                filename = include_begin + sizeof(SSI_VIRTUAL_STRING) - 1;
                include_end = strstr(filename, "\"");
                *include_end = '\0';

                if (webnet_session_get_physical_path(session, filename, path) == 0)
                {
                    _webnet_ssi_sendfile(session, path);
                }
            }
            else
            {
                include_begin = strstr(ssi_begin, SSI_FILE_STRING);
                if (include_begin != RT_NULL)
                {
                    filename = include_begin + sizeof(SSI_FILE_STRING) - 1;
                    include_end = strstr(filename, "\"");
                    *include_end = '\0';

                    _webnet_ssi_sendfile(session, filename);
                }
            }
            offset = ssi_end + sizeof(SSI_END_STRING) - 1;
        }
    }

    /* exit and release buffer buffer */
__exit:
    if (path != RT_NULL) wn_free(path);
    if (buffer != RT_NULL) wn_free(buffer);
}

static const char* ssi_extension[] =
{
    ".shtm",
    ".SHTM",
    ".shtml",
    ".SHTML",
    ".stm",
    ".STM",
    RT_NULL
};

int webnet_module_ssi(struct webnet_session* session, int event)
{
    struct webnet_request* request;

    /* check parameter */
    RT_ASSERT(session != RT_NULL);
    request = session->request;
    RT_ASSERT(request != RT_NULL);

    if (event == WEBNET_EVENT_URI_POST)
    {
        int fd;
        int index;

        /* check whether a ssi file */
        index = 0;
        while (ssi_extension[index] != RT_NULL)
        {
            if (strstr(request->path, ssi_extension[index]) != RT_NULL)
            {
                /* try to open this file */
                fd = open(request->path, O_RDONLY, 0);
                if ( fd >= 0)
                {
                    _webnet_ssi_dofile(session, fd);
                    close(fd);
                    return WEBNET_MODULE_FINISHED;
                }
                else
                {
                    /* no this file */
                    request->result_code = 404;
                    return WEBNET_MODULE_FINISHED;
                }
            }

            index ++;
        }

        /* no this file, try index.shtm */
        {
            char *ssi_filename;

            ssi_filename = (char*) wn_malloc (WEBNET_PATH_MAX);
            if (ssi_filename != RT_NULL)
            {
                rt_snprintf(ssi_filename, WEBNET_PATH_MAX, "%s/index.shtm", request->path);
                fd = open(ssi_filename, O_RDONLY, 0);

                if (fd >= 0)
                {
                    wn_free(ssi_filename);
                    _webnet_ssi_dofile(session, fd);
                    close(fd);
                    return WEBNET_MODULE_FINISHED;
                }
            }
            wn_free(ssi_filename);
        }
    }

    return WEBNET_MODULE_CONTINUE;
}

#endif
