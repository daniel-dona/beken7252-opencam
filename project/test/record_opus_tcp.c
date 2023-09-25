#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>

#include <drivers/audio.h>

#include <rtthread.h>
#include <sys/socket.h> /* 使用BSD socket，需要包含sockets.h头文件 */
#include "netdb.h"

#include "record_common.h"
#include "opus.h"

#define RECORD_SAVE_BUF_SIZE (60 * 20 *2)

struct record_manager
{
    struct net_worker *net;
    rt_mq_t msg;

    int action;
    struct rt_mempool mp;
    int sample_rate;
    int mp_block_size; /* sample / 50 * 2 ==> 8k:320  16k:640*/
    int mp_cnt;

    char *save_buf;
    int save_len;

    rt_sem_t ack;
};

static struct record_manager *recorder;
static short in_short[320*2]; /* NB:8K 160, WB:16K 320 */

static int record_msg_send(struct record_manager *record, void *buffer, int type, int len)
{
    int ret = RT_EOK;
    struct record_msg msg;

    msg.type = type;
    msg.arg = (uint32_t)buffer;
    msg.len = len;

    ret = rt_mq_send(record->msg, (void *)&msg, sizeof(struct record_msg));
    if (ret != RT_EOK)
        rt_kprintf("[record]:send msg failed \n");
}


static void record_thread_entry(void *parameter)
{
    rt_device_t device = RT_NULL;
    int ret = RT_EOK;
    uint8_t *mempool;
    rt_uint8_t *buffer;
    rt_uint32_t read_bytes = 0;

    OpusEncoder *opus_enc = RT_NULL;
    int sample_rate, channels, errors, frame_size;
    int application, complexity;
    opus_int32 bitrate_bps;
    int enc_len;

    /* initialize mempool */
    recorder->mp_block_size = 120;
    recorder->mp_cnt = 10;
    mempool = rt_malloc(recorder->mp_block_size * recorder->mp_cnt);
    rt_mp_init(&(recorder->mp), "record_mp", mempool, recorder->mp_block_size * recorder->mp_cnt, recorder->mp_block_size);

    /* initialize msg queue */
    recorder->msg = rt_mq_create("net_msg", sizeof(struct record_msg), 12, RT_IPC_FLAG_FIFO);

    /* initialize tcp client */
    ret = tcp_client_init(recorder->net);
    if (ret != RT_EOK)
    {
        return;
    }

    device = rt_device_find("mic");
    if (!device)
    {
        rt_kprintf("mic device not found \n");
        return;
    }

    rt_device_open(device, RT_DEVICE_OFLAG_RDONLY);

    /* set samplerate */
    {
        int rate = recorder->sample_rate;
        rt_device_control(device, CODEC_CMD_SAMPLERATE, (void *)&rate);
    }

    {
        enc_len = opus_encoder_get_size(1);
        rt_kprintf("opus_encoder_get_size: 1 channel size: %d \n", enc_len);
        enc_len = opus_encoder_get_size(2);
        rt_kprintf("opus_encoder_get_size: 2 channel size: %d \n", enc_len);

        sample_rate = recorder->sample_rate;
        channels = 1;
        application = OPUS_APPLICATION_VOIP;
        complexity = 1; // 1 to 10

        opus_enc = opus_encoder_create(sample_rate, channels, application, &errors);
        if(errors != OPUS_OK)
        {
            rt_kprintf("[opus]:create opus encoder failed : %d! \n", errors);
        }

        frame_size = sample_rate / 50; // 20ms ==>
        opus_encoder_set_complexity(opus_enc, complexity);
		opus_encoder_get_bitrate(opus_enc,bitrate_bps );	
        rt_kprintf("[opus]:default bitrate %d\n", bitrate_bps);
        rt_kprintf("frame_size = %d \n", frame_size);
    }

    rt_kprintf("[record]:start record, tick %d \n", rt_tick_get());
    while (1)
    {
        buffer = rt_mp_alloc(&(recorder->mp), RT_WAITING_NO);
        if(!buffer)
        {
            rt_kprintf("[record]：malloc memory for mempool failed \n");
            rt_thread_mdelay(20);
        }
        else
        {
            /* read data from sound device */
            read_bytes = rt_sound_read(device, 0, in_short, frame_size * 2);
            /*encode ....*/
            {
                enc_len = opus_encode(opus_enc, in_short, frame_size, buffer + 8, recorder->mp_block_size - 8);

                /* write head */
                {
                    opus_uint32 enc_final_range;
                    int_to_char_big_endian(enc_len, buffer);

                    opus_encoder_get_final_range(opus_enc, enc_final_range);
                    int_to_char_big_endian(enc_final_range, buffer+4);
                }

                enc_len += 8;
            }
            record_msg_send(recorder, buffer, RECORD_MSG_DATA, enc_len);
        }
        
        /* send stop cmd */
        if (recorder->action == 0)
        {
            int cmd;

            cmd = 0;
            record_msg_send(recorder, 0, RECORD_MSG_CMD, 1);
            /* wait ack */
            rt_kprintf("[record]:stop record, tick = %d \n", rt_tick_get());
            break;
        }
    }
    rt_device_close(device);
    rt_mp_detach(&(recorder->mp));
    rt_free(mempool);
    rt_mq_delete(recorder->msg);
    {
        opus_encoder_destroy(opus_enc);
    }
    rt_kprintf("[record]:exit record thread, tick = %d \n", rt_tick_get());
}

static void net_transmit_thread_entry(void *parameter)
{
    int ret, cmd;
    struct record_msg msg;

    recorder->save_len = 0;
    while(1)
    {
        if (rt_mq_recv(recorder->msg, &msg, sizeof(struct record_msg), RT_WAITING_FOREVER) == RT_EOK)
        {
            if(msg.type == RECORD_MSG_DATA)
            {
                memcpy(recorder->save_buf + recorder->save_len, (void *)msg.arg, msg.len);
                recorder->save_len += msg.len;
                rt_mp_free((void *)msg.arg);

                if(recorder->save_len >= RECORD_SAVE_BUF_SIZE - recorder->mp_block_size)
                {
                    /*send data*/
                    send(recorder->net->sock, recorder->save_buf, recorder->save_len, 0);
                    recorder->save_len = 0;
                }
            }
            else if(msg.type = RECORD_MSG_CMD)
            {
                cmd = *(int *)msg.arg;
                if(cmd == 0)
                {
                    /* send remain data, and send ack */
                }
            }
        }
    }
}

static int record_opus_tcp(int argc, char **argv)
{
    rt_thread_t tid = RT_NULL;
    int result;

    if(recorder == RT_NULL)
    {
        recorder = rt_malloc(sizeof(struct record_manager));
        if(!recorder)
        {
            rt_kprintf("[record]:malloc memory for recorder manager \n");
            return -RT_ERROR;
        }
        memset(recorder, 0, sizeof(struct record_manager));

        {
            struct net_worker *net = RT_NULL;
            net = rt_malloc(sizeof(struct net_worker));
            if(!net)
            {
                rt_kprintf("[record]:malloc memory for net worker \n");
                return -RT_ERROR;
            }
            memset(net, 0, sizeof(struct net_worker));
            recorder->net = net;

            recorder->save_buf = rt_malloc(RECORD_SAVE_BUF_SIZE);
            memset(recorder->save_buf, 0, RECORD_SAVE_BUF_SIZE);
        }
        
        rt_kprintf("L%d, recorder_create done \n", __LINE__);
    }

    rt_kprintf("L%d, record enter \n", __LINE__);
    if (strcmp(argv[1], "stop") == 0)
    {
        recorder->action = 0;
    }
    else if (strcmp(argv[1], "start") == 0)
    {
        /* record start format samplerate url port */
        recorder->action = 1;

        if(recorder->net->url)
        {
            rt_free(recorder->net->url);
            recorder->net->url = RT_NULL;
        }

        recorder->sample_rate = atoi(argv[2]);
        recorder->net->url = rt_strdup(argv[3]);
        recorder->net->port = atoi(argv[4]);

        rt_kprintf("[record]:samplerate = %d \n", recorder->sample_rate);
        rt_kprintf("[record]:url = %s \n", recorder->net->url);
        rt_kprintf("[record]:port = %d \n", recorder->net->port);

        /* create record thread */
        tid = rt_thread_create("record",
                               record_thread_entry,
                               RT_NULL,
                               1024 * 32,
                               27,
                               10);
        if (tid != RT_NULL)
            rt_thread_startup(tid);

        /* create net send thread */
        tid = rt_thread_create("net_send",
                               net_transmit_thread_entry,
                               RT_NULL,
                               1024 * 8,
                               28,
                               10);
        if (tid != RT_NULL)
            rt_thread_startup(tid);
    }
    else
    {
        // print_record_usage();
    }
}
FINSH_FUNCTION_EXPORT_ALIAS(record_opus_tcp, __cmd_record_opus_tcp, record opus tcp);
