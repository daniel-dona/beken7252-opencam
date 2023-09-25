/*
 * File      : module_index.c
 * This file is part of RT-Thread RTOS/WebNet Server
 * COPYRIGHT (C) 2011, Shanghai Real-Thread Technology Co., Ltd
 *
 * All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-08-02     Bernard      the first version
 * 2012-12-21     aozima       fixed version print.
 */
#include <webnet.h>
#include <module.h>
#include <util.h>

#ifdef WEBNET_USING_INDEX
#include <dfs_posix.h>

int webnet_module_dirindex(struct webnet_session* session, int event)
{
    if( (session->request->method != WEBNET_GET)
            && (session->request->method != WEBNET_POST) )
    {
        return WEBNET_MODULE_CONTINUE;
    }

    if (event == WEBNET_EVENT_URI_POST)
    {
        DIR *dir;
        struct webnet_request *request;
        static const char* header = "<html><head><title>Index of %s</title></head><body bgcolor=\"white\"><h1>Index of %s</h1><hr><pre>";
        static const char* foot = "</pre><hr>WebNet/%s (RT-Thread)</body></html>";

        RT_ASSERT(session != RT_NULL);
        request = session->request;
        RT_ASSERT(request != RT_NULL);

        dir = opendir(request->path);
        if (dir != RT_NULL)
        {
            struct stat s;
            struct dirent* dirent;
            const char* sub_path;
            char *fullpath;

            dirent = RT_NULL;
            fullpath = wn_malloc (WEBNET_PATH_MAX);
            if (fullpath == RT_NULL)
            {
                request->result_code = 500;
                return WEBNET_MODULE_FINISHED;
            }

            webnet_session_set_header(session, "text/html", 200, "OK", -1);
            /* get sub path */
            sub_path = request->path + strlen(webnet_get_root());
            webnet_session_printf(session, header, sub_path, sub_path);
            /* display parent directory */
            webnet_session_printf(session, "<a href=\"../\">..</a>\n", dirent->d_name, dirent->d_name);

            /* list directory */
            do
            {
                dirent = readdir(dir);
                if (dirent == RT_NULL) break;

                rt_memset(&s, 0, sizeof(struct stat));

                /* build full path for each file */
                rt_sprintf(fullpath, "%s/%s", request->path, dirent->d_name);
                str_normalize_path(fullpath);

                stat(fullpath, &s);
                if ( s.st_mode & S_IFDIR )
                {
                    webnet_session_printf(session, "<a href=\"%s/\">%s/</a>\n", dirent->d_name, dirent->d_name);
                }
                else
                {
                    webnet_session_printf(session, "<a href=\"%s\">%s</a>\t\t\t\t\t%d\n", dirent->d_name, dirent->d_name, s.st_size);
                }
            }
            while (dirent != RT_NULL);

            closedir(dir);
            wn_free(fullpath);

            /* set foot */
            webnet_session_printf(session, foot, WEBNET_VERSION);

            return WEBNET_MODULE_FINISHED;
        }
    }

    return WEBNET_MODULE_CONTINUE;
}
#endif

