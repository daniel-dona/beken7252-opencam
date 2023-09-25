#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <webnet.h>
#include <module.h>


#include <rtdevice.h>


/*for readability */
#define LED_ON  0
#define LED_OFF 1



#define LED_COUNT       (2)
static uint8_t led_value_buffer[LED_COUNT];

void cgi_led_handler(struct webnet_session* session)
{
    struct webnet_request* request = session->request;

	rt_device_t Pin_dev;
    //rt_led_hw_init();
    Pin_dev = rt_device_find("pin");
    if (Pin_dev == RT_NULL)
    {
        rt_kprintf("can not find the pin device!\n");
        return;
    }
    rt_pin_mode(0,PIN_MODE_OUTPUT);
	rt_pin_mode(1,PIN_MODE_OUTPUT);
	
	/* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, ".html", 200, "Ok", -1);

    if (request->query_counter != 0)
    {
        const char * led_no_str = webnet_request_get_query(request, "led_no");
        const char * value_str = webnet_request_get_query(request, "value");

        if(led_no_str != RT_NULL && value_str != RT_NULL)
        {
            rt_uint8_t led_no = atoi(led_no_str);
            rt_uint8_t led_value = atoi(value_str);

            if(led_no < LED_COUNT)
			{
				rt_pin_write(led_no, led_value);
			}
        }
    }

    /**< output led status */
    {
        rt_uint32_t led_no = LED_COUNT;
        rt_uint32_t index;

        /**< json begin */
        webnet_session_printf(session, "{");
        webnet_session_printf(session, "\"led_status\":[ ");

        for(index=0; index<led_no; index++)
        {
            rt_uint8_t led_value = led_value_buffer[index];
			led_value = rt_pin_read(index);
			
			if(led_value == 0)
			{
				led_value = LED_OFF;
			}
			else if(led_value == 1)
			{
				led_value = LED_ON;
			}
			
			
            webnet_session_printf(session, "{\"value\":\"%d\"},", led_value);
        }

        /**< json end */
        webnet_session_printf(session, " ] }");
    }
}
