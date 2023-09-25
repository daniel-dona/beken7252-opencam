#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>

#include <drivers/audio.h>

#include <rtthread.h>

#include "record_common.h"

struct mic_dac_loop_manager
{
    int action;
    struct rt_mempool mp;
    int sample_rate;
    int n_channel;
    int mp_block_size; /* sample / 50 * 2 ==> 8k:320  16k:640*/
    int mp_cnt;

    rt_device_t adc_device;
    rt_device_t dac_device;    
};

static struct mic_dac_loop_manager *micdac_loop;

static rt_err_t mic_dac_loop_write_done(struct rt_device *device, void *ptr)
{
    if (!ptr)
    {
        rt_kprintf("device buf_release NULL\n");
        return -RT_ERROR;
    }

    //rt_kprintf("[micdac]:write_done \n");
    rt_mp_free(ptr);
    return RT_EOK;
}

static void mic_dac_record_thread_entry(void *parameter)
{
    int ret = RT_EOK;
    uint8_t *mempool;
    rt_uint8_t *buffer;
    rt_uint32_t read_bytes = 0;
    int rate, channel;

    /* initialize mempool */
    micdac_loop->mp_block_size = 1024;
    micdac_loop->mp_cnt = 8;
    mempool = rt_malloc(micdac_loop->mp_block_size * micdac_loop->mp_cnt);
    rt_mp_init(&(micdac_loop->mp), "micdac_mp", mempool, 
        micdac_loop->mp_block_size * micdac_loop->mp_cnt, micdac_loop->mp_block_size);

    micdac_loop->adc_device = rt_device_find("mic");
    if (!micdac_loop->adc_device)
    {
        rt_kprintf("audio mic not found \n");
        goto mic_dac_exit;
    }
    
    rate = micdac_loop->sample_rate;
    channel = micdac_loop->n_channel;
    rt_device_control(micdac_loop->adc_device, CODEC_CMD_SAMPLERATE, (void *)&rate); 
    rt_device_control(micdac_loop->adc_device, CODEC_CMD_SET_CHANNEL, (void *)&channel); 
    
    micdac_loop->dac_device = rt_device_find("sound");
    if (!micdac_loop->dac_device)
    {
        rt_kprintf("audio dac not found \n");
        goto mic_dac_exit;
    }
    rt_device_control(micdac_loop->dac_device, CODEC_CMD_SAMPLERATE, (void *)&rate);
    rt_device_set_tx_complete(micdac_loop->dac_device, mic_dac_loop_write_done);   
    
    rt_device_open(micdac_loop->dac_device, RT_DEVICE_OFLAG_WRONLY);
    rt_device_open(micdac_loop->adc_device, RT_DEVICE_OFLAG_RDONLY);

    rt_kprintf("[micdac]:start loop back, tick %d \n", rt_tick_get());
    
    while (1)
    {
        buffer = rt_mp_alloc(&(micdac_loop->mp), RT_WAITING_NO);
        if(!buffer)
        {
            // rt_kprintf("[record]：malloc memory for mempool failed \n");
            rt_thread_mdelay(20);
        }
        else
        {
            /* read data from sound device */
            if(micdac_loop->n_channel == 2) 
            {
                read_bytes = rt_sound_read(micdac_loop->adc_device, 0, buffer, micdac_loop->mp_block_size);
                rt_device_write(micdac_loop->dac_device, 0, buffer, read_bytes);
            } 
            else if(micdac_loop->n_channel == 1) 
            {
                int i;
                int16_t *src, *dst;
                read_bytes = rt_sound_read(micdac_loop->adc_device, 0, buffer + micdac_loop->mp_block_size/2, 
                    micdac_loop->mp_block_size/2);

                // convert to two channel
                src = (int16_t *)(buffer + micdac_loop->mp_block_size/2);
                dst = (int16_t *)(buffer);
                for(i=0; i < read_bytes/2; i++) 
                {
                    dst[2*i] = src[i];
                    dst[2*i + 1] = src[i];
                }
                rt_device_write(micdac_loop->dac_device, 0, buffer, read_bytes*2);   
            }
            //rt_kprintf("%02x,%d\r\n", *((uint16_t*)&buffer[0]), read_bytes);
            //rt_mp_free(buffer);
        }
        
        /* send stop cmd */
        if (micdac_loop->action == 0)
        {
            int cmd;

            cmd = 0;
            //mic_dac_loop_msg_send(micdac_loop, 0, RECORD_MSG_CMD, 1);
            /* wait ack */
            rt_kprintf("[micdac]:stop loop back, tick = %d \n", rt_tick_get());
            break;
        }
    }
    rt_device_close(micdac_loop->adc_device);
    rt_device_close(micdac_loop->dac_device);    
mic_dac_exit:

    micdac_loop->dac_device = RT_NULL;
    micdac_loop->adc_device = RT_NULL;    
    rt_mp_detach(&(micdac_loop->mp));
    rt_free(mempool);

    rt_kprintf("[micdac]:exit loop back, tick = %d \n", rt_tick_get());
}

static int mic_dac_loop(int argc, char **argv)
{
    rt_thread_t tid = RT_NULL;
    int result;

    if(micdac_loop == RT_NULL)
    {
        micdac_loop = rt_malloc(sizeof(struct mic_dac_loop_manager));
        if(!micdac_loop)
        {
            rt_kprintf("[micdac]:malloc memory for mic_dac_loop \n");
            return -RT_ERROR;
        }
        memset(micdac_loop, 0, sizeof(struct mic_dac_loop_manager));
        
        rt_kprintf("L%d, mic_dac_loop done \n", __LINE__);
    }

    rt_kprintf("L%d, mic_dac_loop enter \n", __LINE__);
    if (strcmp(argv[1], "stop") == 0)
    {
        micdac_loop->action = 0;
    }
    else if (strcmp(argv[1], "start") == 0)
    {
        /* record start format samplerate url port */
        micdac_loop->action = 1;

        micdac_loop->sample_rate = atoi(argv[2]);
        micdac_loop->n_channel = atoi(argv[3]);

        micdac_loop->adc_device = RT_NULL;
        micdac_loop->dac_device = RT_NULL;        

        rt_kprintf("[micdac]:samplerate = %d \n", micdac_loop->sample_rate);
        rt_kprintf("[micdac]:n_channel = %d \n", micdac_loop->n_channel);

        /* create record thread */
        tid = rt_thread_create("mic_dac",
                               mic_dac_record_thread_entry,
                               RT_NULL,
                               1024 * 4,
                               27,
                               10);
        if (tid != RT_NULL)
            rt_thread_startup(tid);
    }
    else if(strcmp(argv[1], "volume") == 0)
    {
        int volume = atoi(argv[2]);
        
        rt_kprintf("[micdac]:volume = %d \n", volume);
        // print_record_usage(); cmd_record_pcm_tcp
        if(micdac_loop->adc_device)
            rt_device_control(micdac_loop->adc_device, CODEC_CMD_SET_VOLUME, (void *)&volume);
    }
    else if(strcmp(argv[1], "rate") == 0) 
    {
        int rate = atoi(argv[2]);

        rt_kprintf("[micdac]:sample rate = %d \n", rate);

        if(micdac_loop->adc_device)
            rt_device_control(micdac_loop->adc_device, CODEC_CMD_SAMPLERATE, (void *)&rate);

        if(micdac_loop->dac_device)
            rt_device_control(micdac_loop->dac_device, CODEC_CMD_SAMPLERATE, (void *)&rate);
    }
    else if(strcmp(argv[1], "channel") == 0) 
    {
        int channel = atoi(argv[2]);

        rt_kprintf("[micdac]:channel = %d \n", channel);

        micdac_loop->n_channel = channel;
        if(micdac_loop->adc_device)
            rt_device_control(micdac_loop->adc_device, CODEC_CMD_SET_CHANNEL, (void *)&channel);

    }
}
FINSH_FUNCTION_EXPORT_ALIAS(mic_dac_loop, __cmd_mic_dac_loop, mic dac loop);
