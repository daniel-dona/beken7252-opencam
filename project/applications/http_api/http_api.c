#include "rtthread.h"

#include <webnet.h>
#include <module.h>

static int http_api_init(void)
{
    extern void cgi_ssdp_xml_handler(struct webnet_session* session);
    extern void control_cgi_handler(struct webnet_session* session);

    webnet_cgi_register("cgi_ssdp_xml",cgi_ssdp_xml_handler);
    webnet_cgi_register("control_cgi",control_cgi_handler);
}
INIT_APP_EXPORT(http_api_init);