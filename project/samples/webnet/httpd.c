#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <webnet.h>
#include <module.h>

extern void cgi_calc_handler(struct webnet_session* session);
extern void cgi_hello_handler(struct webnet_session* session);
extern void cgi_led_handler(struct webnet_session* session);
extern void cgi_ipcfg_init(void);

extern void cgi_mem_handler(struct webnet_session* session);

extern void asp_var_version(struct webnet_session* session);

extern const struct webnet_module_upload_entry upload_entry_upload;

static void setting(void)
{
#ifdef WEBNET_USING_AUTH
    webnet_auth_set("/admin", "admin:admin");
#endif /* WEBNET_USING_AUTH */

#ifdef WEBNET_USING_CGI
    webnet_cgi_register("hello", cgi_hello_handler);
    webnet_cgi_register("calc", cgi_calc_handler);
    webnet_cgi_register("led", cgi_led_handler);
	webnet_cgi_register("info.cgi", cgi_mem_handler);
    cgi_ipcfg_init();
#endif /* WEBNET_USING_CGI */

#ifdef WEBNET_USING_ASP
    webnet_asp_add_var("version", asp_var_version);
#endif /* WEBNET_USING_ASP */

#ifdef WEBNET_USING_UPLOAD
    webnet_upload_add(&upload_entry_upload);
#endif /* WEBNET_USING_UPLOAD */
}

int httpd_init(void)
{
    webnet_set_port(80);
    webnet_set_root("/webnet");

    setting();
    webnet_init();
}
INIT_APP_EXPORT(httpd_init);
