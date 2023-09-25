#include <rtthread.h>
#include "vc_config.h"

#define VOICE_CHANGER_SOFT_TIMER_HANDLER       1
#define VOICE_CHANGER_THREADT_TASK_HANDLER     2

#define VOICE_CHANGER_HANDLER                  VOICE_CHANGER_THREADT_TASK_HANDLER


#define VOICE_CHANGER_MIC_CFG                  1
#define VOICE_CHANGER_MIC_INIT_CFG             1

#define VOICE_CHANGER_DEFAULT_OUT_AUD          1
#define VOICE_CHANGER_AUD_INIT_CFG             1
#define VOICE_CHANGER_AUD_SINGLE_CH            1


#ifndef min
#define min(x, y)                  (((x) < (y)) ? (x) : (y))
#endif


#if CONFIG_VOICE_CHANGER 
#include "app_voice_changer.h"
#include "bk_rtos_pub.h"
#include "audio_device.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#define VC_BUFF_MAX_LEN    (256 * 4 *sizeof(unsigned int))
#define VC_HANDLER_INTERVAL_MS            5


static void *vctimer = NULL;
static char *vcbuff = NULL;
#if VOICE_PCM_VC_AUD_OUTPUT_TEST
#define PCM_LENGTH          35254
extern const unsigned char acnumber_pcm[];  ///35254
static unsigned int pc_offset = 0;
#endif
int voice_changer_read_pcm(char*outbuf,int len)
{
	int out_len = 0;
#if VOICE_CHANGER_MIC_CFG
	out_len = audio_device_mic_read(outbuf,len);
#endif

#if VOICE_PCM_VC_AUD_OUTPUT_TEST
	out_len = min(len,(PCM_LENGTH - pc_offset));
	memcpy(outbuf,acnumber_pcm+pc_offset,out_len);

	pc_offset += out_len;
	if(pc_offset >= PCM_LENGTH)
	{
		pc_offset = 0;
		rt_kprintf("restart\r\n");
	}
#endif	
	return out_len;
}

int voice_changer_write_pcm(char*outbuf,int len)
{
	int input_len = 0;
	
#if VOICE_CHANGER_DEFAULT_OUT_AUD
	int bufsz;
	uint16_t* aud_buf = (uint16_t *)audio_device_get_buffer(&bufsz);
	if((bufsz == 0) || (aud_buf == NULL))
	{
		if(aud_buf)
		{
			audio_device_put_buffer(aud_buf);
		}
		rt_kprintf("vc err L%d\r\n",__LINE__);
		return input_len;
	}

	input_len = min((bufsz>>1),len);
	if(len == 0)
	{
		goto exit;
	}
	#if VOICE_CHANGER_AUD_SINGLE_CH
		int16_t *src,*dst;
		int i;
		src = outbuf;
		dst = aud_buf;
		for(i=0;i<(len/2);i++)
		{
			dst[2 * i] = src[i];
			dst[2 * i + 1] = src[i];
		}

		audio_device_write((uint8_t *)aud_buf, input_len*2);	
	#else
		memcpy(aud_buf,outbuf,input_len);
		audio_device_write((uint8_t *)aud_buf, input_len);
	#endif
#endif
	return input_len;
exit:
	if(aud_buf) 
	{
		audio_device_put_buffer(aud_buf);
	}
	rt_kprintf("vc L%d err\r\n",__LINE__);
	return 0;
}

int voice_changer_shutoff(void)
{
	if (vctimer != RT_NULL) 
	{
	#if VOICE_CHANGER_HANDLER  == VOICE_CHANGER_SOFT_TIMER_HANDLER
		rt_timer_stop((rt_timer_t)vctimer);
	#elif VOICE_CHANGER_HANDLER  == VOICE_CHANGER_THREADT_TASK_HANDLER
		rt_thread_detach((rt_thread_t)vctimer);
	#endif
	}
	if(vcbuff) 
	{
		rt_free(vcbuff);
		vcbuff = NULL;
	}
	
	return 0;
}

int voice_changer_launch(unsigned int freq)
{
	if(vcbuff == NULL) 
	{
		vcbuff = (char*)rt_malloc(VC_BUFF_MAX_LEN);
	}
	if(vcbuff == NULL) 
	{
		rt_kprintf("vcbuff == null\r\n");
		return -1;
	}
#if (VOICE_CHANGER_MIC_CFG && VOICE_CHANGER_MIC_INIT_CFG)
	audio_device_init();
	
	audio_device_mic_open();
	audio_device_mic_set_channel(1);
	audio_device_mic_set_rate(freq);
#endif

#if VOICE_CHANGER_DEFAULT_OUT_AUD && VOICE_CHANGER_AUD_INIT_CFG
	audio_device_init();

	audio_device_open();
	audio_device_set_rate(freq);
	audio_device_set_volume(100);
#endif

	voice_changer_initial(freq);
	if (vctimer != RT_NULL) 
	{
	#if VOICE_CHANGER_HANDLER  == VOICE_CHANGER_SOFT_TIMER_HANDLER
		rt_timer_start((rt_timer_t)vctimer);
		voice_changer_start();
	#elif VOICE_CHANGER_HANDLER  == VOICE_CHANGER_THREADT_TASK_HANDLER
		rt_thread_startup((rt_thread_t)vctimer);
	#endif
		rt_kprintf("vc start\r\n");
	}

	return 0;
}

int voice_changer_handler(void)
{
	unsigned char* vc_out;
    int vc_out_len;
	int len;

	if(vcbuff == NULL)
	{
		rt_kprintf("vcbuff err\r\n");
		return -1;
	}

	len = voice_changer_get_need_mic_data();  
	if(len > 0) {
		len = (len > (VC_BUFF_MAX_LEN/4))?(VC_BUFF_MAX_LEN/4) : len;
	}
	else if(len < 0)
	{
		return -1;
	}
	else if(len == 0) 
	{
		return 0;
	}

	len = voice_changer_read_pcm(vcbuff,len);
	if(len <= 0)
	{
		rt_kprintf("origin pcm empty\r\n");
		return 0;
	}
	
	vc_out_len = voice_changer_data_handle((uint8*)vcbuff, len, &vc_out);
    if(vc_out_len == 0) 
	{
        // no enough data for vc, so vc return 0, no need do sm_playing
        return 0;
    }
	else if(vc_out_len > 0) 
	{  
    #if 1
    	len = voice_changer_write_pcm((char*)vc_out,vc_out_len);
	#else
		voice_changer_write_pcm(vcbuff,len);
		len = vc_out_len;
	#endif
		if(len > 0) 
		{
        	voice_changer_set_cost_data(len);
		}
    }
	return 0;
}

#if VOICE_CHANGER_HANDLER  == VOICE_CHANGER_SOFT_TIMER_HANDLER
void voice_changer_timer_handler(void *arg)
{
	voice_changer_handler();
}
#elif VOICE_CHANGER_HANDLER  == VOICE_CHANGER_THREADT_TASK_HANDLER
void voice_changer_task_handler(void *arg)
{
	voice_changer_start();
	
	while(1)
	{
		voice_changer_handler();
		rt_thread_delay(VC_HANDLER_INTERVAL_MS);
	}
}
#endif

int app_voice_changer_init(void)
{
#if VOICE_CHANGER_HANDLER  == VOICE_CHANGER_SOFT_TIMER_HANDLER
	if(vctimer == NULL)
	{
		vctimer = (void*)rt_timer_create("vc",
								voice_changer_timer_handler,
								NULL,
								VC_HANDLER_INTERVAL_MS,
								RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
	}
#elif VOICE_CHANGER_HANDLER  == VOICE_CHANGER_THREADT_TASK_HANDLER
	if(vctimer == NULL)
	{
		vctimer = (void*)rt_thread_create("vc",
											voice_changer_task_handler,
											NULL,
											4*1024,
											15,
											20);
		rt_kprintf("vctimer = %p\r\n",vctimer);
	}
#endif
	return 0;
}

INIT_APP_EXPORT(app_voice_changer_init);

static int voice_changer_sample(int argc, char *argv[])
{
	rt_err_t ret = RT_EOK;
	unsigned int freq = 16000;

	if(argc == 2)
	{
		if(strcmp(argv[1],"launch") == 0)
		{
			rt_kprintf("voice changer freq = %d\r\n",freq);
			voice_changer_launch(freq);
		}
		else if(strcmp(argv[1],"shutoff") == 0)
		{
			rt_kprintf("voice changer shutoff\r\n");
			voice_changer_shutoff();
		}
		else if(strcmp(argv[1],"next") == 0)
		{
			rt_kprintf("voice changer set next\r\n");
			voice_changer_set_change_flag();
		}
	}
	else if(argc == 3)
	{
		if(strcmp(argv[1],"launch") == 0)
		{
			freq = atoi(argv[2]);
			rt_kprintf("voice changer freq = %d\r\n",freq);
			voice_changer_launch(freq);
		}
	}
	return ret;
}


MSH_CMD_EXPORT(voice_changer_sample,vc sample);

#endif
