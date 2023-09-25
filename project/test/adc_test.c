#include "error.h"
#include "include.h"

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include <stdint.h>
#include <stdlib.h>
#include <finsh.h>
#include <rtdef.h>

#include "saradc_intf.h"
#include "sys_ctrl_pub.h"

#define CONFIG_ADC_TEST
#ifdef CONFIG_ADC_TEST

static ADC_OBJ test_adc;

/****channel 1 - 7***/
static void adc_detect_callback(int new_mv, void *user_data)
{
	static int cnt = 0;
	test_adc.user_data = (void*)new_mv;

	if(cnt++ >= 100)
	{	
		cnt = 0;
    	rt_kprintf("adc channel%d voltage:%d,%x\r\n",test_adc.channel,new_mv,test_adc.user_data);
	}
}


void adc_channel_test(int argc,char *argv[])
{
	int channel;
	
	if (strcmp(argv[1], "start") == 0)
	{
		if(argc == 3)
		{
			channel = atoi(argv[2]);
			rt_kprintf("---adc channel:%d---\r\n",channel);
			saradc_work_create(20);
			adc_obj_init(&test_adc, adc_detect_callback, channel, &test_adc);
			adc_obj_start(&test_adc);
		}
		else
		{
			rt_kprintf("input param error\r\n");
		}
	}
	if(strcmp(argv[1], "stop") == 0)
	{
		adc_obj_stop(&test_adc);
	}
}

MSH_CMD_EXPORT(adc_channel_test,adc test);

#endif
