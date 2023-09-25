#include <rtthread.h>
#include <stdlib.h> /* atoi */
#include <rtdevice.h>
#include <webnet.h>
#include <module.h>

#include "http_api.h"

static res_info_t res_info[] = 
{
    {0, "OK"},
    {-1, "argument not found"},
    {-2, "api argument error"},
};

void control_cgi_handler(struct webnet_session* session)
{
    struct webnet_request* request;
    const char *arg = RT_NULL;
    const char *key,*value;
    int err_index = 0, res = 0;
    static const char* content = "{\"res\":%d,\"msg\":\"%s\"%s}";
    char result[256] = "";

    RT_ASSERT(session != RT_NULL);      
    request = session->request;
    RT_ASSERT(request != RT_NULL);

    if (request->query_counter)
    {
        key = webnet_request_get_query(request, "key");
        arg = webnet_request_get_query(request, "value");
        value = atoi(arg);

        if (rt_strcmp(key,"OPEN") == 0)
        {
            /* led 引脚为输出模式*/
	        rt_pin_mode(value, PIN_MODE_OUTPUT);
            rt_pin_write(value,PIN_LOW);
            rt_kprintf("R: %s %d \r\n",key,value);
        }
        else if(rt_strcmp(key,"CLOSE") == 0)
        {
            /* led 引脚为输出模式*/
	        rt_pin_mode(value, PIN_MODE_OUTPUT); 
            rt_pin_write(value,PIN_HIGH);
            rt_kprintf("R: %s %d \r\n",key,value);
        }
        else
        {
            err_index = 1;
            goto __resp;
        }
    }
    else
    {
        err_index = 2;
        goto __resp;
    }




__resp:
    /* set http response */
    session->request->result_code = 200;
    webnet_session_set_header(session, "application/json", 200, "OK", -1);
    webnet_session_printf(session, content, res_info[err_index].res, res_info[err_index].msg);

}