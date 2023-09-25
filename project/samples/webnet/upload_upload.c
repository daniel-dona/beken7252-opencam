#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <webnet.h>
#include <module.h>

#include <dfs_posix.h>

/**
 * upload file.
 */
static const char * sd_upload = "/webnet/SD";

static const char * upload_dir = "upload"; /* e.g: "upload" */
static int file_size = 0;
static int upload_open (struct webnet_session* session)
{
    int fd;

    rt_kprintf("Upload FileName: %s\n", webnet_upload_get_filename(session));
    rt_kprintf("Content-Type   : %s\n", webnet_upload_get_content_type(session));

    if (webnet_upload_get_filename(session) != RT_NULL)
    {
        int path_size;
        char * file_path;

        path_size = strlen(sd_upload) + strlen(upload_dir)
                    + strlen(webnet_upload_get_filename(session));
		//path_size = strlen(webnet_get_root()) + strlen(upload_dir)
        //            + strlen(webnet_upload_get_filename(session));

        path_size += 4;
        file_path = (char *)rt_malloc(path_size);

        if(file_path == RT_NULL)
        {
            fd = -1;
            goto _exit;
        }

        sprintf(file_path,
                "%s/%s/%s",
                sd_upload,
                upload_dir,
                webnet_upload_get_filename(session));
        rt_kprintf("save to: %s\r\n", file_path);

        fd = open(file_path, O_WRONLY | O_CREAT, 0);
        if (fd < 0)
        {
            webnet_session_close(session);
            rt_free(file_path);

            fd = -1;
            goto _exit;
        }
    }

    file_size = 0;

_exit:
    return (int)fd;
}

static int upload_close(struct webnet_session* session)
{
    int fd;

    fd = (int)webnet_upload_get_userdata(session);
    if (fd < 0) return 0;

    close(fd);
    rt_kprintf("Upload FileSize: %d\n", file_size);
    return 0;
}

static int upload_write(struct webnet_session* session, const void* data, rt_size_t length)
{
    int fd;

    fd = (int)webnet_upload_get_userdata(session);
    if (fd < 0) return 0;

    rt_kprintf("write: length %d\n", length);

    write(fd, data, length);
    file_size += length;

    return length;
}

static int upload_done (struct webnet_session* session)
{
    const char* mimetype;
    static const char* status = "<html><head><title>Upload OK </title>"
                                "</head><body>Upload OK, length=%d "
                                "</br><a href=\"javascript:history.go(-1);\">"
                                "Continue Upload</a></body></html>\r\n";

    /* get mimetype */
    mimetype = mime_get_type(".html");

    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", rt_strlen(status));
    webnet_session_printf(session, status, file_size);

    return 0;
}

const struct webnet_module_upload_entry upload_entry_upload =
{
    "/upload",
    upload_open,
    upload_close,
    upload_write,
    upload_done
};
