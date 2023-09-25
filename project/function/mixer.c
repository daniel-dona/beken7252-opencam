#include "rtthread.h"
#include <rtdevice.h>
#include "include.h"
#include <drivers/audio.h>
#include "mixer.h"
#include "sound_delay.h"
#include "audio.h"

#if CONFIG_SOUND_MIXER
static rt_size_t mixer_sound_read(rt_device_t, rt_off_t, void *, rt_size_t);
static void mixer_entry(void *pv);
static void mixer_deinit(void);

MIXER_T *g_mixer = NULL;

#if MIXER_MUX_NEW_STRATEGY
uint32_t mixer_outside_src_buf_init(void)
{
    char *ptr;

    ptr = (char *)rt_malloc(OUTSIDE_SRC_EXCHANGE_BUF_SIZE);
    if(0 == ptr)
    {
        return MIXER_FAILURE;
    }

    rt_memset(ptr, 0, OUTSIDE_SRC_EXCHANGE_BUF_SIZE);
    g_mixer->outside_src_buf = ptr;
    g_mixer->outside_src_pos = 0;

    MIXER_PRINTF("[mixer] mixer_outside_src_buf_init\r\n");

    return MIXER_SUCCESS;
}

uint32_t mixer_outside_src_buf_deinit(void)
{
    MIXER_PRINTF("[mixer] mixer_outside_src_buf_deinit\r\n");

    if(g_mixer->outside_src_buf)
    {
        rt_free(g_mixer->outside_src_buf);
        g_mixer->outside_src_buf = 0;
    }

    return MIXER_SUCCESS;
}

uint32_t mixer_outside_mp_init(void)
{
    rt_err_t ret;

    MIXER_PRINTF("[mixer] mixer_outside_mp_init\r\n");
    RT_ASSERT(g_mixer->outside_src_buf);

    ret = rt_mp_init(&(g_mixer->outside_mp.pool), "outside_mp",
                     g_mixer->outside_src_buf,
                     (OUTSIDE_MP_BLOCK_SIZE + 4) * OUTSIDE_MP_BLOCK_COUNT,
                     OUTSIDE_MP_BLOCK_SIZE);
    RT_ASSERT(RT_EOK == ret);

    g_mixer->outside_mp.blk_count = OUTSIDE_MP_BLOCK_COUNT;
    g_mixer->outside_mp.blk_size = OUTSIDE_MP_BLOCK_SIZE;

    return MIXER_SUCCESS;
}

uint32_t mixer_outside_mp_recycle_all(void)
{
    uint32_t i;
    uint32_t ret;
    void *mp_ptr;
    uint32_t *mp_addr_array;

    MIXER_PRINTF("[mixer] mixer_outside_mp_recycle_all\r\n");
    RT_ASSERT(g_mixer);

    ret = MIXER_SUCCESS;
    mp_addr_array = rt_malloc(sizeof(*mp_addr_array) * OUTSIDE_MP_BLOCK_COUNT);
    if(NULL == mp_addr_array)
    {
        MIXER_WARNING_LOG("mixer_recycle_mp_malloc_failed\r\n");
        ret = MIXER_FAILURE;
        goto recycle_exit;
    }

    /* alloc all memory nodes at the mempool*/
    for(i = 0; i < OUTSIDE_MP_BLOCK_COUNT; i ++)
    {
        mp_ptr = rt_mp_alloc(&g_mixer->outside_mp.pool, RT_WAITING_FOREVER);
        if(NULL == mp_ptr)
        {
            MIXER_WARNING_LOG("mixer_mp_alloc_failed\r\n");
            ret = MIXER_FAILURE;
        }

        MIXER_PRINTF("[mixer] mp_alloc:0x%x\r\n", mp_ptr);
        mp_addr_array[i] = (uint32_t)mp_ptr;
    }

    /* alloc all memory nodes at the mempool*/
    for(i = 0; i < OUTSIDE_MP_BLOCK_COUNT; i ++)
    {
        MIXER_PRINTF("[mixer] mp_free:0x%x\r\n", mp_addr_array[i]);
        rt_mp_free((void *)mp_addr_array[i]);
    }

    rt_free(mp_addr_array);
    mp_addr_array = NULL;

    MIXER_PRINTF("[mixer] mp_recycling_over\r\n");

recycle_exit:
    return ret;
}

uint32_t mixer_outside_mp_deinit(void)
{
    rt_err_t ret;

    MIXER_PRINTF("[mixer] mixer_outside_mp_deinit\r\n");
    ret = rt_mp_detach(&(g_mixer->outside_mp.pool));
    RT_ASSERT(RT_EOK == ret);

    return MIXER_SUCCESS;
}
#else
uint32_t mixer_mp_init(void)
{
    uint32_t ret;

    RT_ASSERT(g_mixer);
    MIXER_PRINTF("[mixer] mixer_mp_init\r\n");

    ret = MIXER_SUCCESS;
    g_mixer->mp.blk_size = MP_BLOCK_SIZE;
    g_mixer->mp.blk_count = MP_BLOCK_COUNT;

    g_mixer->mp.pool = rt_mp_create("mixer_mp",
                                    g_mixer->mp.blk_count,
                                    g_mixer->mp.blk_size);
    if(RT_NULL == g_mixer->mp.pool)
    {
        MIXER_WARNING_LOG("mixer_mp_create_failed\r\n");
        ret = MIXER_FAILURE;
    }

    return ret;
}

void mixer_mp_deinit(void)
{
    rt_err_t ret;

    MIXER_PRINTF("[mixer] mixer_mp_deinit\r\n");
    ret = rt_mp_delete(g_mixer->mp.pool);
    RT_ASSERT(RT_EOK == ret);

    g_mixer->mp.pool = RT_NULL;
    g_mixer->mp.blk_count = 0;
    g_mixer->mp.blk_size = 0;
}
#endif

uint32_t mixer_init(void)
{
    rt_err_t ret;
    MIXER_PRINTF("[mixer] mixer_init\r\n");

    sdly_init();
    sdly_start_play();

    g_mixer = rt_malloc(sizeof(*g_mixer));
    if(NULL == g_mixer)
    {
        MIXER_WARNING_LOG("mixer_init_failed\r\n");
        goto init_exit;
    }
    rt_memset(g_mixer, 0, sizeof(*g_mixer));

    g_mixer->mx_msg_queue = rt_mq_create(MX_MSG_QUEUE_NAME,
                                         sizeof(MIXER_MSG_T),
                                         MX_MSG_QUEUE_COUNT,
                                         RT_IPC_FLAG_FIFO);
    if(RT_NULL == g_mixer->mx_msg_queue)
    {
        goto init_exit;
    }

    g_mixer->mx_mutex = rt_mutex_create(MX_MUTEX_NAME, RT_IPC_FLAG_FIFO);;
    if(RT_NULL == g_mixer->mx_mutex)
    {
        goto init_exit;
    }

    g_mixer->wait_end = rt_sem_create("mixer_sema", 0,  RT_IPC_FLAG_FIFO);
    if(RT_NULL == g_mixer->wait_end)
    {
        goto init_exit;
    }

#if MIXER_MUX_NEW_STRATEGY
    if(MIXER_FAILURE == mixer_outside_src_buf_init())
    {
        goto init_exit;
    }
    mixer_outside_mp_init();
#else
    if(MIXER_FAILURE == mixer_mp_init())
    {
        goto init_exit;
    }
#endif

    INIT_LIST_HEAD(&g_mixer->music_src_list);

    ret = rt_thread_init(&g_mixer->mx_thread, MX_THREAD_NAME,
                         mixer_entry, NULL,
                         g_mixer->mx_stack,
                         sizeof(g_mixer->mx_stack),
                         MIXER_THREAD_PRIORITY, 10);
    RT_ASSERT(RT_EOK == ret);

    g_mixer->is_thd_running = 1;
    g_mixer->is_thd_pause   = 0;
    g_mixer->audio_sample_rate = MIXER_DEFAULT_SAMPLE_RATE;
    ret = rt_thread_startup(&g_mixer->mx_thread);
    RT_ASSERT(RT_EOK == ret);

    return MIXER_SUCCESS;

init_exit:
    mixer_deinit();

    return MIXER_FAILURE;
}

static uint32_t mixer_send_msg(uint32_t sig, uint32_t detail, uint32_t len)
{
    rt_err_t ret;
    uint32_t val;
    MIXER_MSG_T msg;

    msg.sig = sig;
    msg.detail = detail;
    msg.len = len;

    RT_ASSERT(g_mixer->mx_msg_queue);
    ret = rt_mq_send(g_mixer->mx_msg_queue, &msg, sizeof(msg));
    if(RT_EOK == ret)
    {
        MIXER_LOG("mixer_send_msg:0x%x\r\n", detail);
        val = MIXER_SUCCESS;
    }
    else
    {
        MIXER_PRINTF("[mixer] mixer_send_msg_failed:0x%x\r\n", detail);
        val = MIXER_FAILURE;
    }

    return val;
}

uint32_t mixer_send_data_msg(void *node_ptr, uint32_t len)
{
    uint32_t ret;

    ret = mixer_send_msg(DATA_SIGNATURE, (uint32_t)node_ptr, len);
    if(ret == MIXER_FAILURE) 
    {
        RT_ASSERT(ret != MIXER_FAILURE);
        rt_mp_free(node_ptr);
    }

    return MIXER_SUCCESS;
}

uint32_t mixer_send_msg_audio_src_flow(void)
{
    uint32_t ret;
    MIXER_PRINTF("[mixer] audio_src_flow\r\n");
    
    ret = mixer_send_msg(MSG_SIGNATURE, MSG_TYPE_AUDIO_SRC_FLOW, 8);
    RT_ASSERT(ret == MIXER_SUCCESS);

    ret = rt_sem_take(g_mixer->wait_end, 5000);
    if(ret == -RT_ETIMEOUT)
    {
        MIXER_WARNING_LOG("[mixer] wait_end timeout\r\n");
    }
    /* maybe wait_end may release many times*/
    while(rt_sem_trytake(g_mixer->wait_end) != -RT_ETIMEOUT );
    
    MIXER_PRINTF("[mixer] audio_src_flow ed\r\n");
    
    return 0;
}

uint32_t mixer_send_msg_audio_src_static(void)
{
    uint32_t ret;
    MIXER_PRINTF("[mixer] audio_src_static\r\n");
    
    ret = mixer_send_msg(MSG_SIGNATURE, MSG_TYPE_AUDIO_SRC_STATIC, 8);
    RT_ASSERT(ret == MIXER_SUCCESS);

    ret = rt_sem_take(g_mixer->wait_end, 5000);
    if(ret == -RT_ETIMEOUT)
    {
        MIXER_WARNING_LOG("[mixer] wait_end timeout\r\n");
    }
    /* maybe wait_end may release many times*/
    while(rt_sem_trytake(g_mixer->wait_end) != -RT_ETIMEOUT );
    
    MIXER_PRINTF("[mixer] audio_src_static ed\r\n");
    
    return 0;
}

void mixer_run(void)
{
    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    g_mixer->is_thd_running = 1;
    rt_mutex_release(g_mixer->mx_mutex);
}

void mixer_stop(void)
{
    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    g_mixer->is_thd_running = 0;
    rt_mutex_release(g_mixer->mx_mutex);
}

uint32_t mixer_is_running(void)
{
    uint32_t ret;

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    ret = g_mixer->is_thd_running;
    rt_mutex_release(g_mixer->mx_mutex);

    return ret;
}

void mixer_pause(void)
{
    int ret;
    
    MIXER_PRINTF("[mixer] mixer_pause\r\n");

    if(g_mixer->is_thd_pause == MIX_ACT_PAUSE) 
        return;
    
    ret = mixer_send_msg(MSG_SIGNATURE, MSG_TYPE_PAUSE_MIXER, 8);
    RT_ASSERT(ret == MIXER_SUCCESS)

    ret = rt_sem_take(g_mixer->wait_end, 5000);
    if(ret == -RT_ETIMEOUT)
    {
        MIXER_WARNING_LOG("[mixer] wait_end timeout\r\n");
    }
    /* maybe wait_end may release many times*/
    while(rt_sem_trytake(g_mixer->wait_end) != -RT_ETIMEOUT );

    MIXER_PRINTF("[mixer] mixer_pause d \r\n");
}


void mixer_replay(void)
{
    int ret;
    
    MIXER_PRINTF("[mixer] mixer_replay\r\n");
    
    if(g_mixer->is_thd_pause == MIX_ACT_PLAYING) 
        return;
    
    ret = mixer_send_msg(MSG_SIGNATURE, MSG_TYPE_REPLAY_MIXER, 8);
    RT_ASSERT(ret == MIXER_SUCCESS)

    ret = rt_sem_take(g_mixer->wait_end, 5000);
    if(ret == -RT_ETIMEOUT)
    {
        MIXER_WARNING_LOG("[mixer] wait_end timeout\r\n");
    }
    /* maybe wait_end may release many times*/
    while(rt_sem_trytake(g_mixer->wait_end) != -RT_ETIMEOUT );
    
    MIXER_PRINTF("[mixer] mixer_replayed \r\n");
}

static uint32_t mixer_adc_open(void)
{
    uint32_t ret;
    rt_err_t val;
    rt_device_t dev;
    uint32_t sample_rate;
    uint32_t channel_cnt;
    ADC_DEV_T *adc = &(g_mixer->adc_dev);

    RT_ASSERT(g_mixer);
    MIXER_PRINTF("[mixer] mixer_adc_open, %p\r\n", adc->dev);

    if(adc->dev == NULL) 
    {
        dev = rt_device_find("mic");
        if(NULL == dev)
        {
            ret = MIXER_FAILURE;
            goto adco_exit;
        }
    } 
    else 
    {
        dev = adc->dev;
    }

    if(dev->open_flag & RT_DEVICE_OFLAG_OPEN)
    {
        MIXER_PRINTF("[mixer] mixer adc aready open\r\n");
        return MIXER_SUCCESS;
    }
    
    ret = MIXER_SUCCESS;
    val = rt_device_open(dev, RT_DEVICE_OFLAG_RDONLY);
    RT_ASSERT(RT_EOK == val);
    adc->dev = dev;
    MIXER_PRINTF("[mixer] mixer_adc_opened\r\n");

adco_exit:
    return ret;
}

static uint32_t mixer_adc_set_rate(uint32_t rate)
{
    rt_err_t val;
    rt_device_t dev;
    uint32_t sample_rate;

    ADC_DEV_T *adc = &(g_mixer->adc_dev);

    RT_ASSERT(g_mixer);
    RT_ASSERT(adc->dev);
    
    MIXER_PRINTF("[mixer] mixer_adc_set_rate, %d\r\n", rate);

    dev = adc->dev;

    sample_rate = rate;
    val = rt_device_control(dev, CODEC_CMD_SAMPLERATE, (void *)&sample_rate);
    RT_ASSERT(RT_EOK == val);
    adc->rate = rate;

    sdly_config(sample_rate);
    sdly_init_forepart_mute();
    sdly_start_play();

    return MIXER_SUCCESS;
}

static uint32_t mixer_adc_close(void)
{
    ADC_DEV_T *adc = &(g_mixer->adc_dev);
    rt_err_t val;

    RT_ASSERT(g_mixer);
    RT_ASSERT(adc->dev);

    MIXER_PRINTF("[mixer] mixer_adc_close\r\n");

    if (!(adc->dev->open_flag & RT_DEVICE_OFLAG_OPEN))
    {
        return MIXER_SUCCESS;
    }
    
    val = rt_device_close(adc->dev);
    RT_ASSERT(RT_EOK == val);

    return MIXER_SUCCESS;
}

static rt_err_t mixer_dac_write_done(struct rt_device *device, void *ptr)
{
    if (!ptr)
    {
        MIXER_PRINTF("[mixer] device buf_release NULL\n");
        return -RT_ERROR;
    }

    rt_mp_free(ptr);

    return RT_EOK;
}

static uint32_t mixer_dac_open(void)
{
    uint32_t ret;
    rt_err_t val;
    rt_device_t dev;
    uint32_t sample_rate;
    DAC_DEV_T *dac = &(g_mixer->dac_dev);

    RT_ASSERT(g_mixer);
    MIXER_PRINTF("[mixer] mixer_dac_open\r\n");

    if(dac->dev == NULL) 
    {
        dev = rt_device_find("sound");
        if(NULL == dev)
        {
            ret = MIXER_FAILURE;
            goto daco_exit;
        }
    } 
    else 
    {
        dev = dac->dev;
    }

    if(dev->open_flag & RT_DEVICE_OFLAG_OPEN)
    {
        MIXER_PRINTF("[mixer] mixer dac aready open\r\n");
        return MIXER_SUCCESS;
    }

    ret = MIXER_SUCCESS;
    rt_device_set_tx_complete(dev, mixer_dac_write_done);

    val = rt_device_open(dev, RT_DEVICE_OFLAG_WRONLY);
    RT_ASSERT(RT_EOK == val);
    dac->dev = dev;
    MIXER_PRINTF("[mixer] mixer_dac_opened\r\n");

daco_exit:
    return ret;
}

static uint32_t mixer_dac_set_rate(uint32_t rate)
{
    rt_err_t val;
    rt_device_t dev;
    uint32_t sample_rate;

    DAC_DEV_T *dac = &(g_mixer->dac_dev);

    RT_ASSERT(g_mixer);
    RT_ASSERT(dac->dev);
    
    MIXER_PRINTF("[mixer] mixer_dac_set_rate, %d\r\n", rate);

    dev = dac->dev;

    sample_rate = rate;
    val = rt_device_control(dev, CODEC_CMD_SAMPLERATE, (void *)&sample_rate);
    RT_ASSERT(RT_EOK == val);
    dac->rate = rate;

    return MIXER_SUCCESS;
}

static uint32_t mixer_dac_close(void)
{
    DAC_DEV_T *dac = &(g_mixer->dac_dev);
    rt_err_t val;

    RT_ASSERT(g_mixer);
    RT_ASSERT(dac->dev);

    MIXER_PRINTF("[mixer] mixer_dac_close\r\n");

    if (!(dac->dev->open_flag & RT_DEVICE_OFLAG_OPEN))
    {
        return MIXER_SUCCESS;
    }
    
    val = rt_device_close(dac->dev);
    RT_ASSERT(RT_EOK == val);

    return MIXER_SUCCESS;
}

static uint32_t mixer_music_src_in(uint8_t *buf, uint32_t len)
{
    uint32_t ret;
    MUSIC_SRC_NODE_T *node;

    ret = MIXER_SUCCESS;
    if((0 == len) || (!g_mixer->is_mux_src))
    {
        goto in_exit;
    }

    MIXER_LOG("mixer_msrc_in:%d\r\n", len);

    node = (MUSIC_SRC_NODE_T *)rt_malloc(sizeof(*node));
    if(NULL == node)
    {
        ret = MIXER_FAILURE;
        goto in_exit;
    }
    node->buf = buf;
    node->len = len;
    node->use_pos = 0;

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    list_add_tail(&node->hdr, &g_mixer->music_src_list);
    rt_mutex_release(g_mixer->mx_mutex);

    return MIXER_SUCCESS;

in_exit:
    rt_mp_free(buf);

    return ret;
}

#if MIXER_MUX_NEW_STRATEGY
uint32_t mixer_music_src_peek_current_node_len(void)
{
    LIST_HEADER_T *tmp;
    LIST_HEADER_T *pos;
    uint32_t length = 0;
    MUSIC_SRC_NODE_T *mnode = NULL;

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    list_for_each_safe(pos, tmp, &g_mixer->music_src_list)
    {
        mnode = list_entry(pos, MUSIC_SRC_NODE_T, hdr);
        length = mnode->len;
        break;
    }
    rt_mutex_release(g_mixer->mx_mutex);

    return length;
}

void *mixer_music_src_peek_current_node_buf(void)
{
    LIST_HEADER_T *tmp;
    LIST_HEADER_T *pos;
    void *node_buf_ptr = NULL;
    MUSIC_SRC_NODE_T *mnode = NULL;

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    list_for_each_safe(pos, tmp, &g_mixer->music_src_list)
    {
        mnode = list_entry(pos, MUSIC_SRC_NODE_T, hdr);
        node_buf_ptr = (void *)mnode->buf;
        break;
    }
    rt_mutex_release(g_mixer->mx_mutex);

    return node_buf_ptr;
}

uint32_t mixer_music_src_out2(void)
{
    LIST_HEADER_T *tmp;
    LIST_HEADER_T *pos;
    MUSIC_SRC_NODE_T *mnode = NULL;

    MIXER_LOG("mixer_music_src_out2\r\n");

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    list_for_each_safe(pos, tmp, &g_mixer->music_src_list)
    {
        mnode = list_entry(pos, MUSIC_SRC_NODE_T, hdr);
        list_del(&mnode->hdr);
        break;
    }
    rt_mutex_release(g_mixer->mx_mutex);

    RT_ASSERT(mnode);
    rt_free(mnode);

    return MIXER_SUCCESS;
}

uint32_t mixer_music_src_flush2(void)
{
    LIST_HEADER_T *tmp;
    LIST_HEADER_T *pos;
    MUSIC_SRC_NODE_T *node = NULL;

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    list_for_each_safe(pos, tmp, &g_mixer->music_src_list)
    {
        node = list_entry(pos, MUSIC_SRC_NODE_T, hdr);

        MIXER_PRINTF("[mixer] mixer_music_src_flush2:0x%x\r\n", node);

        list_del(&node->hdr);
        rt_mp_free(node->buf);
        rt_free(node);
    }
    rt_mutex_release(g_mixer->mx_mutex);

    return MIXER_SUCCESS;
}

#else
uint32_t mixer_music_src_out(MUSIC_SRC_NODE_T *node)
{
    void *ptr;

    MIXER_LOG("mixer_music_src_out\r\n");

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    ptr = (void *)node->buf;
    list_del(&node->hdr);
    rt_mutex_release(g_mixer->mx_mutex);

    rt_mp_free(ptr);
    rt_free(node);

    return MIXER_SUCCESS;
}

uint32_t mixer_music_src_flush(void)
{
    LIST_HEADER_T *tmp;
    LIST_HEADER_T *pos;
    MUSIC_SRC_NODE_T *node = NULL;

    MIXER_PRINTF("[mixer] mixer_music_src_flush\r\n");

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    list_for_each_safe(pos, tmp, &g_mixer->music_src_list)
    {
        node = list_entry(pos, MUSIC_SRC_NODE_T, hdr);
        mixer_music_src_out(node);
    }
    rt_mutex_release(g_mixer->mx_mutex);

    return MIXER_SUCCESS;
}

uint32_t mixer_music_src_get_data(uint8_t *buf, uint32_t expected_len)
{
    uint32_t count;
    uint8_t *data_ptr;
    uint32_t expected_cnt;
    uint32_t length = 0;
    LIST_HEADER_T *tmp;
    LIST_HEADER_T *pos;
    MUSIC_SRC_NODE_T *node = NULL;

    data_ptr = buf;
    expected_cnt = expected_len;
    if(!(expected_cnt && data_ptr))
    {
        goto get_exit;
    }

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    list_for_each_safe(pos, tmp, &g_mixer->music_src_list)
    {
        node = list_entry(pos, MUSIC_SRC_NODE_T, hdr);

        RT_ASSERT(node->len >= node->use_pos);

        count = node->len - node->use_pos;
        if(count >= expected_cnt)
        {
            rt_memcpy(data_ptr, &node->buf[node->use_pos], expected_cnt);
            node->use_pos += expected_cnt;

            length += expected_cnt;

            if(node->len == node->use_pos)
            {
                mixer_music_src_out(node);
            }
            break;
        }
        else
        {
            rt_memcpy(data_ptr, &node->buf[node->use_pos], count);
            node->use_pos = node->len;

            data_ptr = &data_ptr[count];
            length += count;
            expected_cnt -= count;
        }

        if(node->len == node->use_pos)
        {
            mixer_music_src_out(node);
        }
    }
    rt_mutex_release(g_mixer->mx_mutex);

get_exit:
    return length;
}
#endif

static uint32_t mixer_music_src_get_data_len(void)
{
    uint32_t length = 0;
    LIST_HEADER_T *tmp;
    LIST_HEADER_T *pos;
    MUSIC_SRC_NODE_T *node = NULL;

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    list_for_each_safe(pos, tmp, &g_mixer->music_src_list)
    {
        node = list_entry(pos, MUSIC_SRC_NODE_T, hdr);

        RT_ASSERT(node->len >= node->use_pos);
        length += node->len - node->use_pos;
    }
    rt_mutex_release(g_mixer->mx_mutex);

    return length;
}

static MUSIC_SRC_NODE_T *mixer_music_src_peek(void)
{
    LIST_HEADER_T *tmp;
    LIST_HEADER_T *pos;
    MUSIC_SRC_NODE_T *ret = NULL;

    rt_mutex_take(g_mixer->mx_mutex, RT_WAITING_FOREVER);
    list_for_each_safe(pos, tmp, &g_mixer->music_src_list)
    {
        ret = list_entry(pos, MUSIC_SRC_NODE_T, hdr);
        break;
    }
    rt_mutex_release(g_mixer->mx_mutex);

    return ret;
}

static uint32_t _mixer_single_channel_data_double_padding(void *start, void *buf, uint32_t len)
{
    uint32_t i;
    uint16_t *dst, *src;

    dst = start;
    src = buf;
    RT_ASSERT(0 == ((uint32_t)dst & 0x01));
    RT_ASSERT(0 == ((uint32_t)src & 0x01));

    for(i = 0; i < (len >> 1); i++)
    {
        dst[2 * i] = src[i];
        dst[2 * i + 1] = src[i];
    }

    return len * 2;
}

#if MIXER_MUX_NEW_STRATEGY
uint32_t mixer_mux2_acoustics(void)
{
    uint32_t ret;
    uint32_t expected_len;
    uint32_t inside_data_len;
    rt_uint8_t *inside_src_buf;
    rt_uint8_t *outside_src_buf;
    rt_size_t read_bytes, wr_bytes;
    rt_device_t adc_device;
    rt_device_t dac_device;

    MIXER_LOG("mixer_mux2_acoustics\r\n");

    RT_ASSERT(g_mixer->adc_dev.dev);
    RT_ASSERT(g_mixer->dac_dev.dev);
    adc_device = g_mixer->adc_dev.dev;
    dac_device = g_mixer->dac_dev.dev;

    outside_src_buf = g_mixer->outside_src_buf;
    RT_ASSERT(outside_src_buf);

#if CFG_SUPPORT_SINGLE_CHANNEL
    /*step 1: get music data of current node(inside audio source)*/
    expected_len = mixer_music_src_peek_current_node_len();
    MIXER_LOG("expected_len:%d\r\n", expected_len);
    if(0 == expected_len)
    {
        goto mux_exit;
    }
    RT_ASSERT(1 != (expected_len & 0x01));
    RT_ASSERT(expected_len >= 2);

    if(expected_len != g_mixer->outside_src_pos)
    {
        char *outside_buf_ptr = &outside_src_buf[g_mixer->outside_src_pos];
        char *outside_rd_ptr = &outside_src_buf[(g_mixer->outside_src_pos + expected_len) >> 1];;
        uint32_t outside_rd_len = (expected_len - g_mixer->outside_src_pos) >> 1;

        read_bytes = mixer_sound_read(adc_device, 0,
                                      outside_rd_ptr,
                                      outside_rd_len);
        read_bytes = _mixer_single_channel_data_double_padding(outside_buf_ptr,
                     outside_rd_ptr,
                     read_bytes);
        g_mixer->outside_src_pos += read_bytes;
    }

    if(expected_len != g_mixer->outside_src_pos)
    {
        /* maybe the thread need suspend a little time here*/
        goto mux_exit;
    }

    /*step 2: get music data(inside audio source)*/
    inside_src_buf = mixer_music_src_peek_current_node_buf();
    inside_data_len = expected_len;
    MIXER_LOG("inside_src:%x:%d\r\n", inside_src_buf, inside_data_len);
    RT_ASSERT(inside_src_buf);

    /*step 3: mixer*/
    sdly_mixer_multi_playing(outside_src_buf, inside_src_buf, inside_data_len);
#else
#error "it is not the single channel of mic!!!"
#endif

    sdly_forepart_mute(inside_src_buf, inside_data_len);

    g_mixer->outside_src_pos = 0;
    mixer_music_src_out2();

    wr_bytes = rt_device_write(dac_device, 0, inside_src_buf, inside_data_len);
    if(wr_bytes != inside_data_len)
    {
        MIXER_PRINTF("[mixer] WR_MISMATCH:%d:%d\r\n", inside_data_len, wr_bytes);
    }
    return MIXER_SUCCESS;

mux_exit:
    MIXER_LOG("mux_exit\r\n");
    return ret;
}


uint32_t mixer_single2_acoustics(void)
{
    rt_uint8_t *exchange_buf;
    rt_size_t read_bytes, wr_bytes;
    rt_device_t adc_device;
    rt_device_t dac_device;

    RT_ASSERT(g_mixer->adc_dev.dev);
    RT_ASSERT(g_mixer->dac_dev.dev);
    adc_device = g_mixer->adc_dev.dev;
    dac_device = g_mixer->dac_dev.dev;

    MIXER_LOG("mixer_single2_acoustics:0x%x:0x%x\r\n", adc_device, dac_device);

    exchange_buf = rt_mp_alloc(&g_mixer->outside_mp.pool, RT_WAITING_NO);
    if(!exchange_buf)
    {
        MIXER_WARNING_LOG(".");
        MIXER_LOG("mixer_mp_alloc_failed\r\n");
        return MIXER_FAILURE;
    }

#if CFG_SUPPORT_SINGLE_CHANNEL
    read_bytes = mixer_sound_read(adc_device, 0,
                                  &exchange_buf[(g_mixer->outside_mp.blk_size >> 1)],
                                  (g_mixer->outside_mp.blk_size >> 1));
    MIXER_LOG("single_rd:0x%x:%d\r\n", exchange_buf, read_bytes);

    if(0 == read_bytes)
    {
        rt_mp_free(exchange_buf);
        return MIXER_SUCCESS;
    }

    read_bytes = _mixer_single_channel_data_double_padding(exchange_buf,
                 &exchange_buf[(g_mixer->outside_mp.blk_size >> 1)],
                 read_bytes);

    sdly_mixer_single_playing(exchange_buf, read_bytes);
#else
#error "it is not the single channel of mic!!!"
#endif

    sdly_forepart_mute(exchange_buf, read_bytes);

    wr_bytes = rt_device_write(dac_device, 0, exchange_buf, read_bytes);
    if(wr_bytes != read_bytes)
    {
        MIXER_WARNING_LOG("wr_mismatch:%d:%d\r\n", read_bytes, wr_bytes);
    }

    return MIXER_SUCCESS;
}

#else
uint32_t mixer_mux_acoustics(void)
{
    uint32_t ret;
    uint32_t expected_len;
    uint32_t inside_data_len;
    rt_uint8_t *inside_src_buf;
    rt_uint8_t *outside_src_buf;
    rt_size_t read_bytes, wr_bytes;
    rt_device_t adc_device;
    rt_device_t dac_device;

    MIXER_LOG("mixer_mux_acoustics\r\n");

    RT_ASSERT(g_mixer->adc_dev.dev);
    RT_ASSERT(g_mixer->dac_dev.dev);
    adc_device = g_mixer->adc_dev.dev;
    dac_device = g_mixer->dac_dev.dev;

    outside_src_buf = rt_mp_alloc(g_mixer->mp.pool, RT_WAITING_NO);
    inside_src_buf = rt_mp_alloc(g_mixer->mp.pool, RT_WAITING_NO);
    if((!outside_src_buf) || (!inside_src_buf))
    {
        ret = MIXER_FAILURE;

        goto mux_exit;
    }

#if CFG_SUPPORT_SINGLE_CHANNEL
    /*step 1: get mic data(outside audio source)*/
    expected_len = MIN(mixer_music_src_get_data_len(), g_mixer->mp.blk_size);
    MIXER_LOG("expected_len:%d:%d:%d\r\n", expected_len, mixer_music_src_get_data_len(), g_mixer->mp.blk_size);
    if(0 == expected_len)
    {
        goto mux_exit;
    }
    RT_ASSERT(1 != (expected_len & 0x01));
    RT_ASSERT(expected_len >= 2);

    read_bytes = mixer_sound_read(adc_device, 0,
                                  &outside_src_buf[(g_mixer->mp.blk_size >> 1)],
                                  (expected_len >> 1));
    read_bytes = _mixer_single_channel_data_double_padding(outside_src_buf,
                 &outside_src_buf[(g_mixer->mp.blk_size >> 1)],
                 read_bytes);
    /*clear adc data for noise*/
    sdly_forepart_mute(outside_src_buf, read_bytes);

    /*step 2: get music data(inside audio source)*/
    inside_data_len = mixer_music_src_get_data(inside_src_buf, read_bytes);
    MIXER_LOG("read_bytes:%d:%d\r\n", inside_data_len, read_bytes);
    RT_ASSERT(inside_data_len == read_bytes);

    /*step 3: mixer*/
    sdly_mixer_multi_playing(outside_src_buf, inside_src_buf, inside_data_len);
#else
#error "it is not the single channel of mic!!!"
#endif

    rt_mp_free(outside_src_buf);

    wr_bytes = rt_device_write(dac_device, 0, inside_src_buf, inside_data_len);
    if(wr_bytes != inside_data_len)
    {
        MIXER_PRINTF("[mixer] WR_MISMATCH:%d:%d\r\n", inside_data_len, wr_bytes);
        rt_mp_free(inside_src_buf);
    }
    return MIXER_SUCCESS;

mux_exit:
    if(outside_src_buf)
    {
        rt_mp_free(outside_src_buf);
    }

    if(inside_src_buf)
    {
        rt_mp_free(inside_src_buf);
    }

    return ret;
}


uint32_t mixer_single_acoustics(void)
{
    rt_uint8_t *exchange_buf;
    rt_size_t read_bytes, wr_bytes;
    rt_device_t adc_device;
    rt_device_t dac_device;

    RT_ASSERT(g_mixer->adc_dev.dev);
    RT_ASSERT(g_mixer->dac_dev.dev);
    adc_device = g_mixer->adc_dev.dev;
    dac_device = g_mixer->dac_dev.dev;

    MIXER_LOG("mixer_single_acoustics:0x%x:0x%x\r\n", adc_device, dac_device);

    exchange_buf = rt_mp_alloc(g_mixer->mp.pool, RT_WAITING_NO);
    if(!exchange_buf)
    {
        //MIXER_WARNING_LOG(".");
        MIXER_LOG("mixer_mp_alloc_failed\r\n");
        return MIXER_FAILURE;
    }

#if CFG_SUPPORT_SINGLE_CHANNEL
    read_bytes = mixer_sound_read(adc_device, 0,
                                  &exchange_buf[(g_mixer->mp.blk_size >> 1)],
                                  (g_mixer->mp.blk_size >> 1));
    MIXER_LOG("single_rd:0x%x:%d\r\n", exchange_buf, read_bytes);
    if(0 == read_bytes)
    {
        rt_mp_free(exchange_buf);
        return MIXER_SUCCESS;
    }

    read_bytes = _mixer_single_channel_data_double_padding(exchange_buf,
                 &exchange_buf[(g_mixer->mp.blk_size >> 1)],
                 read_bytes);

    /*clear adc data for noise*/
    sdly_forepart_mute(exchange_buf, read_bytes);

    sdly_mixer_single_playing(exchange_buf, read_bytes);
#else
#error "it is not the single channel of mic!!!"
#endif

    wr_bytes = rt_device_write(dac_device, 0, exchange_buf, read_bytes);
    if(wr_bytes != read_bytes)
    {
        MIXER_WARNING_LOG("wr_mismatch:%d:%d\r\n", read_bytes, wr_bytes);
        rt_mp_free(exchange_buf);
    }

    return MIXER_SUCCESS;
}

#endif

static uint32_t mixer_device_open(void)
{
    MIXER_PRINTF("[mixer] mixer_device_open\r\n");

    audio_adc_enable_linein();
    mixer_adc_open();
    mixer_adc_set_rate(MIXER_DEFAULT_SAMPLE_RATE);
    
    mixer_dac_open();
    mixer_dac_set_rate(MIXER_DEFAULT_SAMPLE_RATE);

    g_mixer->is_mux_src = 0;
    g_mixer->audio_sample_rate = MIXER_DEFAULT_SAMPLE_RATE;

    return MIXER_SUCCESS;
}

static uint32_t mixer_device_close(void)
{
    MIXER_PRINTF("[mixer] mixer_device_close\r\n");

    mixer_adc_close();
    mixer_dac_close();

    audio_adc_disable_linein();
    g_mixer->is_mux_src = 0;

    return MIXER_SUCCESS;
}

uint32_t mixer_device_set_rate(uint32_t sample_rate)
{
    int ret;
    
    MIXER_PRINTF("[mixer] set_rate:%d-%d\r\n", sample_rate, g_mixer->audio_sample_rate);
    
    if(g_mixer->audio_sample_rate == sample_rate) 
        return 0;
    
    ret = mixer_send_msg(MSG_SIGNATURE, MSG_TYPE_AUDIO_SET_SAMPLE_RATE, sample_rate);
    RT_ASSERT(ret == MIXER_SUCCESS)

    ret = rt_sem_take(g_mixer->wait_end, 5000);
    if(ret == -RT_ETIMEOUT)
    {
        MIXER_WARNING_LOG("[mixer] wait_end timeout\r\n");
    }
    /* maybe wait_end may release many times*/
    while(rt_sem_trytake(g_mixer->wait_end) != -RT_ETIMEOUT );
    
    MIXER_PRINTF("[mixer] set_rate ed\r\n", sample_rate);
    
    return 0;
}

uint32_t mixer_device_get_rate(void)
{
    return g_mixer->audio_sample_rate;
}

static void mixer_entry(void *pv)
{
    rt_err_t ret;
    MIXER_MSG_T msg;
    uint32_t is_mux_src;

    RT_ASSERT(g_mixer);
    RT_ASSERT(g_mixer->mx_msg_queue);

    /*default setting: no music, only linein(outside mic device)*/
    mixer_device_open();

    while(g_mixer->is_thd_running)
    {
        msg.sig = 0;
        ret = rt_mq_recv(g_mixer->mx_msg_queue, &msg, sizeof(msg), MX_MSG_QUEUE_TIMEOUT);
        
        switch(msg.detail)
        {
        case MSG_TYPE_AUDIO_SRC_FLOW:
            if(MSG_SIGNATURE == msg.sig)
            {
                MIXER_PRINTF("[mixer] AUDIO_SRC_FLOW:%d\r\n", g_mixer->is_thd_pause);

                if(g_mixer->is_thd_pause == MIX_ACT_PLAYING)
                {
                    g_mixer->is_mux_src = 1;
                }
                else 
                {
                    mixer_dac_open();
                }

                ret = rt_sem_release(g_mixer->wait_end);
                RT_ASSERT(ret == RT_EOK);
            }
            break;

        case MSG_TYPE_AUDIO_SRC_STATIC:
            if(MSG_SIGNATURE == msg.sig)
            {
                MIXER_PRINTF("[mixer] AUDIO_SRC_STATIC:%d\r\n", g_mixer->is_thd_pause);

                if(g_mixer->is_thd_pause == MIX_ACT_PLAYING)
                {
                    g_mixer->is_mux_src = 0;

                    #if !MIXER_MUX_NEW_STRATEGY
                    mixer_music_src_flush();
                    #endif
                }
                else
                {
                    mixer_dac_close();
                }

                ret = rt_sem_release(g_mixer->wait_end);
                RT_ASSERT(ret == RT_EOK);
            }
            break;

        case MSG_TYPE_AUDIO_SET_SAMPLE_RATE:
            if(MSG_SIGNATURE == msg.sig)
            {
                MIXER_PRINTF("[mixer] MSG_TYPE_AUDIO_SET_SAMPLE_RATE:%d - %d\r\n", 
                    msg.len,  g_mixer->audio_sample_rate);

                if(g_mixer->is_thd_pause == MIX_ACT_PLAYING)
                {
                    if(g_mixer->audio_sample_rate != msg.len)
                    {
                        g_mixer->audio_sample_rate = msg.len;
                        mixer_adc_set_rate(g_mixer->audio_sample_rate);
                        //rt_thread_delay(1000);
                        mixer_dac_set_rate(g_mixer->audio_sample_rate);
                    }
                }
                else
                {
                    mixer_dac_set_rate(msg.len);
                }
                
                ret = rt_sem_release(g_mixer->wait_end);
                RT_ASSERT(ret == RT_EOK);
            }
            break;

        case MSG_TYPE_PAUSE_MIXER:
            if(MSG_SIGNATURE == msg.sig)
            {
                MIXER_PRINTF("[mixer] MSG_TYPE_PAUSE_MIXER\r\n");

                g_mixer->is_thd_pause = MIX_ACT_PAUSE;
                
                mixer_adc_close();
                
                if(g_mixer->is_mux_src == 0)
                    mixer_dac_close();

                audio_adc_disable_linein();

                #if !MIXER_MUX_NEW_STRATEGY
                mixer_music_src_flush();
                #endif

                ret = rt_sem_release(g_mixer->wait_end);
                RT_ASSERT(ret == RT_EOK);
            }
            break;

        case MSG_TYPE_REPLAY_MIXER:
            if(MSG_SIGNATURE == msg.sig)
            {
                MIXER_PRINTF("[mixer] MSG_TYPE_REPLAY_MIXER:%d,%d\r\n", 
                    g_mixer->audio_sample_rate, g_mixer->is_mux_src);

                audio_adc_enable_linein();
                mixer_adc_open();
                mixer_adc_set_rate(g_mixer->audio_sample_rate);
                
                mixer_dac_open();
                mixer_dac_set_rate(g_mixer->audio_sample_rate);

                g_mixer->is_thd_pause = MIX_ACT_PLAYING;
                
                ret = rt_sem_release(g_mixer->wait_end);
                RT_ASSERT(ret == RT_EOK);
            }
            break;

        default:
            if(DATA_SIGNATURE == msg.sig)
            {
                uint8_t *buf = (uint8_t *)msg.detail;
                uint32 len = msg.len;

                MIXER_LOG("DATA_SIGNATURE\r\n");
                mixer_music_src_in(buf, len);
            }
            break;
        }

        if(g_mixer->is_thd_pause == MIX_ACT_PAUSE)
            continue;

        if(g_mixer->is_mux_src)
        {
#if MIXER_MUX_NEW_STRATEGY
            mixer_mux2_acoustics();
#else
            mixer_mux_acoustics();
#endif
        }
        else
        {
#if MIXER_MUX_NEW_STRATEGY
            mixer_single2_acoustics();
#else
            mixer_single_acoustics();
#endif
        }
    }

    mixer_device_close();

    mixer_deinit();
}

static void mixer_deinit(void)
{
    rt_err_t ret;

    MIXER_PRINTF("[mixer] mixer_deinit\r\n");
    if(NULL == g_mixer)
    {
        return;
    }

    if(g_mixer->mx_msg_queue)
    {
        rt_mq_detach(g_mixer->mx_msg_queue);
        ret = rt_mq_delete(g_mixer->mx_msg_queue);
        RT_ASSERT(RT_EOK == ret);

        g_mixer->mx_msg_queue = NULL;
    }

    if(g_mixer->mx_mutex)
    {
        rt_mutex_detach(g_mixer->mx_mutex);
        ret = rt_mutex_delete(g_mixer->mx_mutex);
        RT_ASSERT(RT_EOK == ret);

        g_mixer->mx_mutex = NULL;
    }
    if(g_mixer->wait_end)
    {
        rt_sem_detach(g_mixer->wait_end);
        ret = rt_sem_delete(g_mixer->wait_end);
        RT_ASSERT(RT_EOK == ret);

        g_mixer->wait_end = NULL;
    }

#if MIXER_MUX_NEW_STRATEGY
    mixer_outside_mp_deinit();
    mixer_outside_src_buf_deinit();
#else
    mixer_mp_deinit();
#endif

    rt_free(g_mixer);
    g_mixer = NULL;

    sdly_deinit();
}

static rt_size_t mixer_sound_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_size_t read_bytes = 0;

    while (read_bytes < size)
    {
        rt_size_t rb = rt_device_read(dev, pos, (void *)((char *)buffer + read_bytes), size - read_bytes);

        if (rb == 0)
            break;

        read_bytes += rb;
    }

    return read_bytes;
}

uint32_t mixer_device_write(void *buffer, int size)
{
    if(g_mixer->is_thd_pause == MIX_ACT_PAUSE)
        return 1;

    mixer_send_data_msg(buffer, size);

    return 0;
}

#endif // CONFIG_SOUND_MIXER
// eof

