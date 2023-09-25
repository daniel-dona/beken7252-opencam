#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <webnet.h>
#include <module.h>

void cgi_hello_handler(struct webnet_session* session)
{
    const char* mimetype;
    static const char* status = "<html><head><title> SystemStatus </title>"
                                "</head><body><font size=\"+2\">hello world</font></br>System is running</body></html>\r\n";

    /* get mimetype */
    mimetype = mime_get_type(".html");

    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", strlen(status));

    webnet_session_write(session, (const rt_uint8_t*)status, rt_strlen(status));
    return;
}
