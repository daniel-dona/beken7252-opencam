#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <drivers/audio.h>
#include <audio_pipe.h>
#include "board.h"
#include <string.h>

#include "include.h"
#if ((CFG_SOC_NAME == SOC_BK7221U) && (CFG_USE_AUD_ADC == 1))
#include "arm_arch.h"
#include "general_dma_pub.h"
#include "audio.h"
#include "audio_pub.h"

#define DBG_SECTION_NAME              "[MIC]"
#define DBG_LEVEL                     DBG_INFO
#include <rtdbg.h>

#define AUDIO_RECV_BUFFER_LEN         (160 * 2 * 4)
#define AUDIO_RECV_BUFFER_CNT         (4)
#define AUD_ADC_DEF_DMA_CHANNEL       GDMA_CHANNEL_2
#define AUDIO_ADC_DEF_WR_THRED        AUD_ADC_DEF_WR_THRED
#define AUDIO_ADC_DEF_GAIN            AUD_ADC_DEF_GAIN

#define AUDIO_ADC_DEF_SAMPLE_RATE     (8000)
#define AUDIO_ADC_DEF_VOLUME          (80)

/* sample numbers */
#define AUDIO_ADC_FIFO_SIZE           (AUD_ADC_MAX_THRED*4) // MAX_THRED * sizeof(int16) * channel

#define BEKEN_AUDIO_ADC_DMA           1

struct audio_mic_device
{
    /* inherit from rt_device */
    struct rt_device parent;
    rt_uint16_t *recv_fifo;
    rt_uint16_t *cur_ptr;
    rt_uint32_t recv_fifo_len;
    rt_uint32_t n_channel;
    rt_uint32_t stat;
    struct rt_audio_pipe record_pipe;
};

static struct audio_mic_device _g_audio_mic;

static void adc_dma_init(struct audio_mic_device *audio_adc);

static rt_err_t audio_adc_init(rt_device_t dev)
{
    struct audio_mic_device *audio_adc = RT_NULL;
    audio_adc = (struct audio_mic_device *)dev;

    audio_adc->n_channel = 1;
    audio_adc_set_enable_bit(0);
    audio_adc_set_sample_rate(AUDIO_ADC_DEF_SAMPLE_RATE);
    audio_adc_set_hpf2_bypass_bit(1);
    audio_adc_set_volume(AUDIO_ADC_DEF_VOLUME);

    audio_power_up();

    return RT_EOK;
}

static rt_err_t audio_adc_open(rt_device_t dev, rt_uint16_t oflag)
{
    struct audio_mic_device *audio_adc = RT_NULL;

    audio_adc = (struct audio_mic_device *)dev;

    if ((oflag & RT_DEVICE_OFLAG_RDONLY) && (!(audio_adc->stat & ADC_IS_OPENED)))
    {
        rt_device_open(&audio_adc->record_pipe.parent, RT_DEVICE_OFLAG_RDONLY);
        
    #ifdef BEKEN_AUDIO_ADC_DMA
        adc_dma_init(audio_adc);
        audio_adc_set_dma(1);
        audio_adc->stat |= ADC_DMA_IRQ_ENABLE;
    #else
        /* For adc, use isr */
        audio_adc_set_write_thred_bit(AUDIO_ADC_DEF_WR_THRED);
        audio_adc_set_int_enable_bit(1);
        audio_enable_interrupt();
        audio_adc->stat |= ADC_IRQ_ENABLE;
    #endif
        audio_adc_set_enable_bit(1);
        audio_adc_open_analog_regs();

        dbg_log(DBG_INFO, "open mic device");

        audio_adc->stat |= ADC_IS_OPENED;
    }

    return RT_EOK;
}

static rt_size_t audio_adc_read(rt_device_t dev, rt_off_t pos,
                                void *buffer, rt_size_t size)
{
    struct audio_mic_device *audio_adc = RT_NULL;

    audio_adc = (struct audio_mic_device *)dev;
    return rt_device_read(&audio_adc->record_pipe.parent, pos, buffer, size);
}

static rt_err_t audio_adc_control(rt_device_t dev, int cmd, void *args)
{
    rt_err_t result = RT_EOK, stat;
    struct audio_mic_device *audio = RT_NULL;

    audio = (struct audio_mic_device *)dev;
    stat = audio->stat;

    switch (cmd)
    {
    case CODEC_CMD_SET_CHANNEL:
        if(audio->stat & ADC_IS_OPENED) {
            #ifdef BEKEN_AUDIO_ADC_DMA
            audio_adc_set_dma(0);
            audio_adc_set_enable_bit(0);
            audio_adc_set_int_enable_bit(0);

            audio->n_channel = *(rt_uint32_t *)args;
            
            adc_dma_init(audio);

            audio_adc_set_dma(1);
            audio_adc_set_enable_bit(1);
            audio_adc_set_int_enable_bit(1);
            #else
            #error "please add isr set channel"
            #endif
        }
        else 
        {
            audio->n_channel = *(rt_uint32_t *)args;
        }
        rt_kprintf("set adc channel %d \n", audio->n_channel);
        break;
        
    case CODEC_CMD_SET_VOLUME:
    {
        rt_uint32_t volume = *(rt_uint32_t *)args;

        dbg_log(DBG_INFO, "set adc volume %d \n", volume);

        audio_adc_set_volume(volume);
        break;
    }

    case CODEC_CMD_SAMPLERATE:
    {
        rt_uint32_t freq = *(rt_uint32_t *)args;
        if ((audio->stat & ADC_IS_OPENED))
        {
            audio_adc_set_enable_bit(0);
            audio_adc_set_dma(0);
            
            audio_adc_set_sample_rate(freq);

            audio_adc_set_dma(1);
            audio_adc_set_enable_bit(1);
        }
        else 
        {
            audio_adc_set_sample_rate(freq);
        }

        rt_kprintf("set adc sample rate %d \n", freq);
        
        break;
    }

    default:
        result = RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t audio_adc_close(rt_device_t dev)
{
    struct audio_mic_device *audio_adc = RT_NULL;
    rt_uint32_t stat;

    audio_adc = (struct audio_mic_device *)dev;
    stat = audio_adc->stat;

    if (((stat & ADC_DMA_IRQ_ENABLE) || (stat & ADC_IRQ_ENABLE)) && 
        (stat & ADC_IS_OPENED))
    {
        audio_adc_set_enable_bit(0);
        audio_adc_set_int_enable_bit(0);
        // comment this for open again may have noise 
        //audio_adc_close_analog_regs();

        audio_adc->n_channel = 1;

    #ifdef BEKEN_AUDIO_ADC_DMA
        audio_adc_set_dma(0);
    #else

    #endif
        rt_device_close(&audio_adc->record_pipe.parent);
        stat &= ~(ADC_DMA_IRQ_ENABLE | ADC_IRQ_ENABLE | ADC_IS_OPENED);
    }

    audio_adc->stat = stat;
    dbg_log(DBG_INFO, "close mic device");
}

void audio_adc_irq_handler(UINT32 arg)
{
    rt_uint32_t status, cnt;
    struct audio_mic_device *audio_adc = RT_NULL;

    audio_adc = &_g_audio_mic;

    status = REG_READ(AUD_AD_FIFO_STATUS);
    if (status & ADC_INT_FLAG)
    {
        cnt = 0;
        while (!(status & (ADC_FIFO_EMPTY)))
        {
            if (audio_adc->n_channel == 1)
                audio_adc_get_l_sample(&audio_adc->recv_fifo[cnt++]);
            else
                audio_adc_get_l_and_r_samples(&audio_adc->recv_fifo[cnt++],
                                              &audio_adc->recv_fifo[cnt++]);

            status = REG_READ(AUD_AD_FIFO_STATUS);
        }
        if (cnt > 0)
        {
            rt_device_write(&audio_adc->record_pipe.parent, 0, audio_adc->recv_fifo, cnt << 1);
        }
    }
}

void adc_dma_half_handler(UINT32 flag)
{
    
}

void adc_dma_finish_handler(UINT32 flag)
{
    int result;
    struct audio_mic_device *audio_adc = RT_NULL;
    rt_uint8_t *adc_buf_ptr, *end_buf;

    audio_adc = &_g_audio_mic;
    adc_buf_ptr = (rt_uint8_t *)audio_adc->cur_ptr;
    end_buf = ((rt_uint8_t *)audio_adc->recv_fifo) + audio_adc->recv_fifo_len;

    if(adc_buf_ptr) 
	{	
        // copy(actually only need change write ptr of pipe)
        rt_device_write(&audio_adc->record_pipe.parent, 0, 
								adc_buf_ptr, 
								AUDIO_RECV_BUFFER_LEN);

        adc_buf_ptr += AUDIO_RECV_BUFFER_LEN;

        if(adc_buf_ptr >= end_buf)
            adc_buf_ptr = (rt_uint8_t *)audio_adc->recv_fifo;

        audio_adc->cur_ptr = (rt_uint16_t *)adc_buf_ptr;
    } 
}

void adc_dma_init(struct audio_mic_device *audio_adc)
{
    GDMACFG_TPYES_ST cfg;
    GDMA_CFG_ST en_cfg;

    rt_uint8_t *adc_buf_ptr = (rt_uint8_t *)audio_adc->recv_fifo;
    int adc_buf_len = audio_adc->recv_fifo_len;
    rt_uint32_t n_channel = audio_adc->n_channel;

    rt_kprintf("adc-buf:%p, adc-buf-len:%d, ch:%d\r\n", adc_buf_ptr, adc_buf_len, n_channel);
    if(!adc_buf_ptr)
        return; 
    
    memset(&cfg, 0, sizeof(GDMACFG_TPYES_ST));
    memset(&en_cfg, 0, sizeof(GDMA_CFG_ST));

    cfg.dstdat_width = 32;
    if(n_channel == 1)
        cfg.srcdat_width = 16;
    else
        cfg.srcdat_width = 32;
    
    cfg.dstptr_incr = 1;
    cfg.srcptr_incr = 0;
    
    cfg.src_start_addr = (void*)AUD_ADC_FIFO_PORT;
    cfg.dst_start_addr = adc_buf_ptr;

    cfg.channel = AUD_ADC_DEF_DMA_CHANNEL;
    cfg.prio = 0;
    cfg.u.type5.dst_loop_start_addr = adc_buf_ptr;
    cfg.u.type5.dst_loop_end_addr = adc_buf_ptr + adc_buf_len; 

    cfg.half_fin_handler = NULL;//adc_dma_half_handler;
    cfg.fin_handler = adc_dma_finish_handler;
    
    cfg.src_module = GDMA_X_SRC_AUDIO_TX_REQ;
    cfg.dst_module = GDMA_X_DST_DTCM_WR_REQ;

    sddev_control(GDMA_DEV_NAME, CMD_GDMA_CFG_TYPE5, &cfg);

    en_cfg.channel = AUD_ADC_DEF_DMA_CHANNEL;
    en_cfg.param = AUDIO_RECV_BUFFER_LEN;//adc_buf_len; // dma translen
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_TRANS_LENGTH, &en_cfg);
}

#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops audio_mic_ops =
{
    audio_adc_init,
    audio_adc_open,
    audio_adc_close,
    audio_adc_read,
    RT_NULL,
    audio_adc_control
};
#endif /* RT_USING_DEVICE_OPS */

int rt_audio_adc_hw_init(void)
{
    int result = RT_EOK;
    struct audio_mic_device *audio_adc = &_g_audio_mic;
    struct rt_device *device = &audio_adc->parent;

    if (audio_adc->recv_fifo)
    {
        return RT_EOK;
    }

    audio_adc->parent.type = RT_Device_Class_Sound;
    audio_adc->parent.rx_indicate = RT_NULL;
    audio_adc->parent.tx_complete = RT_NULL;
    audio_adc->parent.user_data   = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops = &audio_mic_ops;
#else
    audio_adc->parent.control = audio_adc_control;
    audio_adc->parent.init    = audio_adc_init;
    audio_adc->parent.open    = audio_adc_open;
    audio_adc->parent.close   = audio_adc_close;
    audio_adc->parent.read    = audio_adc_read;
    audio_adc->parent.write   = RT_NULL;
#endif /* RT_USING_DEVICE_OPS */

    /* register the device */
    rt_device_register(&audio_adc->parent, "mic", 
        RT_DEVICE_FLAG_STANDALONE | RT_DEVICE_FLAG_RDONLY | RT_DEVICE_FLAG_DMA_RX);

    rt_device_init(&audio_adc->parent);

    {
        rt_size_t size = AUDIO_RECV_BUFFER_LEN * AUDIO_RECV_BUFFER_CNT;
        rt_uint8_t *buf = sdram_malloc(size);
        if(buf == RT_NULL)
        {
            rt_device_unregister(&audio_adc->parent);
            return -RT_ENOMEM;
        }

        memset(buf, 0, size);
        
        rt_audio_pipe_init(&audio_adc->record_pipe,
                           "voice",
                           RT_PIPE_FLAG_FORCE_WR | RT_PIPE_FLAG_BLOCK_RD,
                           buf,
                           size);

        // set the rx fifo which used by dma same as record pipe, save memery
        audio_adc->recv_fifo_len = size;
        audio_adc->recv_fifo = (rt_uint16_t*)buf;
        audio_adc->cur_ptr = (rt_uint16_t*)buf;
    }

    return RT_EOK;
}

INIT_DEVICE_EXPORT(rt_audio_adc_hw_init);
#endif // #if (CFG_SOC_NAME == SOC_BK7221U)