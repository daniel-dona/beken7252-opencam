#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>


#include "voice_config.h"

#define DEBUG_PRINTF     rt_kprintf("[voice] ");rt_kprintf
#define SAMPLE_RATE         (16000)

#define malloc  rt_malloc
#define realloc rt_realloc
#define free    rt_free

#define codec_device_lock(...)
#define codec_device_unlock(...)

/************************ voice config start ***************************/
static unsigned char voice_config_ssid[32 + 1] = {0};
static unsigned char voice_config_password[64 + 1] = {0};


void *voice_malloc(int size)
{
    return rt_malloc(size);
}

void voice_free(void *mem)
{
    rt_free(mem);
}

int voice_read(void *device, void *buffer, int size)
{
    struct rt_device *dev = (struct rt_device *)device;
    rt_size_t read_bytes = 0;

    while (read_bytes < size)
    {
        rt_size_t rb = rt_device_read(dev, 0, (void *)((char *)buffer + read_bytes), size - read_bytes);

        if (rb == 0)
            break;

        read_bytes += rb;
    }

    return read_bytes;
}

#include <finsh.h>
#include <msh.h>
static void station_connect(const char *ssid, const char *passwd)
{
    char argv[64];

    memset(argv, 0, sizeof(argv));
    sprintf(argv, "wifi %s join %s %s", "w0", ssid, passwd);
    msh_exec(argv, strlen(argv));
}

static rt_thread_t tid = RT_NULL;
static void cmd_voice_config_thread(void *parameter)
{
    rt_device_t device = 0;
    struct voice_config_result result={0};
    int res;

    DEBUG_PRINTF("voice config version: %s\r\n", voice_config_version());

    /* open audio device and set tx done call back */
    device = rt_device_find("mic");
    if (device == RT_NULL)
    {
        DEBUG_PRINTF("audio device not found!\r\n");
        goto _err;
    }

    codec_device_lock();
    res = rt_device_open(device, RT_DEVICE_OFLAG_RDWR);
    /* set samplerate */
    if (RT_EOK == res)
    {
        int SamplesPerSec = SAMPLE_RATE;
        if (rt_device_control(device, CODEC_CMD_SAMPLERATE, &SamplesPerSec)
                != RT_EOK)
        {
            rt_kprintf("[record] audio device doesn't support this sample rate: %d\r\n",
                       SamplesPerSec);
            goto _err;
        }
    }
	else
	{
		goto _err;
	}

    rt_device_write(device, 0, 0, 100); // start to record
    res = voice_config_work(device, SAMPLE_RATE, 1000 * 60 * 1, &result);
    if(res == 0)
    {
        rt_kprintf("ssid len=%d, [%s]\n", result.ssid_len, result.ssid);
        rt_kprintf("passwd L=%d, [%s]\n", result.passwd_len, result.passwd);
        rt_kprintf("custom L=%d, [%s]\n", result.custom_len, result.custom);
		
		station_connect(result.ssid,result.passwd);
    }
    else
    {
        rt_kprintf("voice_config res:%d\n", res);
    }

_err:
    if (device)
    {
        rt_device_close(device);
        codec_device_unlock();
    }

    tid = RT_NULL;

    return;
}

int voice_config(int argc, char *argv[])
{
    if (tid)
    {
        rt_kprintf("voice config already init.\n");
        return -1;
    }

    tid = rt_thread_create("voice_config",
                           cmd_voice_config_thread,
                           RT_NULL,
                           1024 * 6,
                           20,
                           10);

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    return 0;
}

#ifdef FINSH_USING_MSH
#include "finsh.h"

MSH_CMD_EXPORT(voice_config, start voice config);
MSH_CMD_EXPORT(voice_config_stop, stop voice config);

#endif /* FINSH_USING_MSH */
