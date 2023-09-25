#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <webnet.h>
#include <module.h>

void cgi_calc_handler(struct webnet_session* session)
{
    int a, b;
    const char* mimetype;
    struct webnet_request* request;
    static const char* header = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" /><title> calc </title></head>";
    static const char* body = "<body><form method=\"post\" action=\"/cgi-bin/calc\"><input type=\"text\" name=\"a\" value=\"%d\"> + <input type=\"text\" name=\"b\" value=\"%d\"> = %d<br><input type=\"submit\" value=\"\xBC\xC6\xCB\xE3\"></form></body></html>\r\n";

    RT_ASSERT(session != RT_NULL);
    request = session->request;
    RT_ASSERT(request != RT_NULL);

    /* get mimetype */
    mimetype = mime_get_type(".html");

    a = 1;
    b = 1;
    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", -1);

    webnet_session_write(session, (const rt_uint8_t*)header, rt_strlen(header));
    if (request->query_counter)
    {
        const char *a_value, *b_value;
        a_value = webnet_request_get_query(request, "a");
        b_value = webnet_request_get_query(request, "b");

        a = atoi(a_value);
        b = atoi(b_value);
    }

    webnet_session_printf(session, body, a, b, a + b);
    return;
}

