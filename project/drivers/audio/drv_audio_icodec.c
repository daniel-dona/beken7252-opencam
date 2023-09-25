#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <drivers/audio.h>
#include <audio_pipe.h>

#include <string.h>

#include "include.h"

#if ((CFG_SOC_NAME == SOC_BK7221U) && (CFG_USE_AUD_DAC == 1))
#include "arm_arch.h"
#include "general_dma_pub.h"
#include "audio.h"
#include "audio_pub.h"
#include "board.h"

#define codec_printf    rt_kprintf

#define DATA_NODE_MAX           10

#define AUDIO_SEND_BUFFER_SIZE 2048

#define AUDIO_DAC_DEF_DMA_CHANNEL     GDMA_CHANNEL_1
#define AUDIO_DAC_DEF_RD_THRED        (8)
#define AUDIO_DAC_DEF_VOLUME          65
#define AUDIO_DAC_DEF_SAMPLE_RATE     (44100)

/* sample numbers */
#define AUDIO_ADC_FIFO_SIZE           (32)
#define PAUSE_EN                      1

struct rt_data_node
{
    char *data_ptr;
    rt_uint32_t data_size;
};
struct rt_data_node_list;

struct rt_data_node_list
{
    struct rt_data_node *node;
    rt_uint32_t size;
    rt_uint32_t read_index, write_index;
    rt_uint32_t data_offset;
    void (*read_complete)(struct rt_data_node *node, void *user_data);
    void *user_data;
};

/* data node for Tx Mode */
struct data_node
{
    char     *data_ptr;
    int32_t  data_size;
};

enum dac_status
{
    DAC_TRANSFER_STOP = 0,
    DAC_TRANSFER_ING = 1,
};

struct audio_codec_device
{
    /* inherit from rt_device */
    struct rt_device parent;
    struct rt_data_node_list *node_list;

    char *send_fifo;
    rt_uint32_t stat;
    rt_uint32_t dma_irq_cnt;
    rt_uint32_t volume;
	rt_uint32_t paused;
	rt_uint32_t fill_pos;
	rt_uint32_t fill_size;
};

static struct audio_codec_device _g_audio_codec;

/**
 * RT-Thread Audio utils
 */
int rt_data_node_init(struct rt_data_node_list **node_list, rt_uint32_t size)
{
    int result = RT_EOK;
    struct rt_data_node_list *list = RT_NULL;
    struct rt_data_node *node = RT_NULL;

    list = rt_malloc(sizeof(struct rt_data_node_list));
    if (list == RT_NULL)
    {
        result = -RT_ENOMEM;
        goto __exit;
    }
    memset(list, 0, sizeof(struct rt_data_node_list));

    node = rt_malloc(sizeof(struct rt_data_node) * size);
    if (node == RT_NULL)
    {
        result = -RT_ENOMEM;
        goto __exit;
    }
    memset(node, 0, sizeof(struct rt_data_node) * size);

    list->node = node;
    list->size = size;
    list->read_index = 0;
    list->write_index = 0;
    list->data_offset = 0;
    list->read_complete = RT_NULL;
    list->user_data = 0;

    *node_list = list;

    return result;

__exit:
    if (list)
        rt_free(list);

    if (node)
        rt_free(node);

    return result;
}

int rt_data_node_is_empty(struct rt_data_node_list *node_list)
{
    struct rt_data_node *node = RT_NULL;
    rt_uint32_t read_index, write_index, next_index;
    rt_base_t level;
    rt_uint32_t result;

    level = rt_hw_interrupt_disable();
    read_index = node_list->read_index;
    write_index = node_list->write_index;
    rt_hw_interrupt_enable(level);

    if (read_index == write_index)
    {
        return RT_TRUE;
    }
    else
    {
        return RT_FALSE;
    }
}

void wait_node_free(struct rt_data_node_list *node_list)
{
    while (node_list->read_index != node_list->write_index)
        rt_thread_delay(5);
}

int rt_data_node_write(struct rt_data_node_list *node_list, void *buffer, rt_uint32_t size)
{
    struct rt_data_node *node = RT_NULL;
    rt_uint32_t read_index, write_index, next_index;
    rt_base_t level;
    rt_uint32_t result;

    level = rt_hw_interrupt_disable();
    read_index = node_list->read_index;
    write_index = node_list->write_index;
    rt_hw_interrupt_enable(level);

    next_index = write_index + 1;
    if (next_index >= node_list->size)
        next_index = 0;

    if (next_index == read_index)
    {
        rt_kprintf("[node]:node list full, write index = %d, read index = %d \n", write_index, read_index);
        return -RT_ERROR;
    }

    level = rt_hw_interrupt_disable();
    /* set node attribute */
    node = &node_list->node[write_index];
    node->data_ptr = (char *) buffer;
    node->data_size = size;
    node_list->write_index = next_index;
    rt_hw_interrupt_enable(level);

    return size;
}

int rt_data_node_read(struct rt_data_node_list *node_list, void *buffer, rt_uint32_t size)
{
    struct rt_data_node *node = RT_NULL;
    rt_uint32_t read_index, write_index, next_index;
    rt_int32_t remain_len, copy_size, read_len;
    rt_uint32_t read_offset, data_offset;
    rt_base_t level;
    rt_uint32_t result = size;

    level = rt_hw_interrupt_disable();
    read_index = node_list->read_index;
    write_index = node_list->write_index;
    rt_hw_interrupt_enable(level);

    read_offset = 0;
    if (read_index == write_index)
    {
        result = 0;
    }
    else
    {
        do
        {
            node = &node_list->node[node_list->read_index];
            data_offset = node_list->data_offset;
            remain_len = node->data_size - data_offset;
            if (size - read_offset > remain_len)
            {
                /* Full*/
                copy_size = remain_len;
            }
            else
            {
                /* reamain buffer */
                copy_size = size - read_offset;
            }

            if(buffer != RT_NULL)
            {
                memcpy((char *)buffer + read_offset, node->data_ptr + data_offset, copy_size);
            }
            
            read_offset += copy_size;
            data_offset += copy_size;
            node_list->data_offset = data_offset;

            if (data_offset >= node->data_size)
            {
                /* notify transmitted complete. */
                if (node_list->read_complete != RT_NULL)
                {
                    node_list->read_complete(node, node_list->user_data);
                }

                level = rt_hw_interrupt_disable();
                read_index = node_list->read_index;
                write_index = node_list->write_index;
                rt_hw_interrupt_enable(level);

                next_index = read_index + 1;
                if (next_index >= node_list->size)
                    next_index = 0;

                level = rt_hw_interrupt_disable();
                node_list->read_index = next_index;
                node_list->data_offset = 0;
                rt_hw_interrupt_enable(level);

                if (next_index == write_index)
                {
                    result = read_offset;
                    break;
                }
            }
        }
        while (read_offset < size);
    }

    return result;
}

void rt_data_node_empty(struct rt_data_node_list *node_list)
{
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    
    while(rt_data_node_is_empty(node_list) == RT_FALSE)
    {
        rt_data_node_read(node_list, RT_NULL, 1024); 
    }

    rt_hw_interrupt_enable(level);
}

/**
 * RT-Thread Audio Driver Interface
 */
static void dac_speaker_enable(rt_uint32_t enable)
{
    UINT32 param;

    param =  GPIO_CFG_PARAM(GPIO9, GMODE_OUTPUT);
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);

    if (enable)
    {
        param = GPIO_CFG_PARAM(GPIO9, 1);
        sddev_control(GPIO_DEV_NAME, CMD_GPIO_OUTPUT, &param);
    }
    else
    {
        param = GPIO_CFG_PARAM(GPIO9, 0);
        sddev_control(GPIO_DEV_NAME, CMD_GPIO_OUTPUT, &param);
    }
}

static void dac_dma_addr_reset(void)
{
    GDMA_CFG_ST en_cfg;

    memset(&en_cfg, 0, sizeof(GDMA_CFG_ST));

    en_cfg.channel = AUDIO_DAC_DEF_DMA_CHANNEL;
    en_cfg.param = 0;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_SRC_PAUSE_ADDR, &en_cfg);
}

#ifdef PAUSE_EN
static void dac_dma_pause_addr_set(UINT32 addr)
{
    GDMA_CFG_ST en_cfg;

    memset(&en_cfg, 0, sizeof(GDMA_CFG_ST));

    en_cfg.channel = AUDIO_DAC_DEF_DMA_CHANNEL;
    en_cfg.param = addr;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_SRC_PAUSE_ADDR, &en_cfg);
}
#endif


static void dac_dma_enable(rt_uint32_t enable)
{
    GDMA_CFG_ST en_cfg;

    en_cfg.channel = AUDIO_DAC_DEF_DMA_CHANNEL;
    if (enable)
        en_cfg.param = 1;
    else
        en_cfg.param = 0;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);
}

static rt_err_t audio_codec_init(rt_device_t dev)
{
    struct audio_codec_device *audio = RT_NULL;

    audio = (struct audio_codec_device *)dev;
    
    /* DAC Init */
    audio_dac_set_enable_bit(0);
    audio_dac_set_sample_rate(AUDIO_DAC_DEF_SAMPLE_RATE);
    audio_dac_set_hpf1_bit(1);
    audio_dac_set_hpf2_bit(1);

    audio->volume = AUDIO_DAC_DEF_VOLUME;
    audio_dac_set_volume(audio->volume);
#if AUD_USE_EXT_PA
    audio_dac_init_mute_pin();
    audio_dac_eable_mute(1);
#endif

    audio_dac_set_int_enable_bit(0);
    audio_dac_open_analog_regs();
    audio_power_up();

    return RT_EOK;
}

static void net_transmit_init(void);
static rt_err_t audio_codec_open(rt_device_t dev, rt_uint16_t oflag)
{
    struct audio_codec_device *audio = RT_NULL;

    audio = (struct audio_codec_device *)dev;
    if ((oflag & RT_DEVICE_OFLAG_WRONLY) && (!(audio->stat & DAC_IS_OPENED)))
    {
#if AUD_USE_EXT_PA
        audio_dac_eable_mute(1);
#endif
        _g_audio_codec.dma_irq_cnt = 0;
        dac_dma_addr_reset();
        dac_dma_enable(1);
		
        audio_dac_open_analog_regs();
        audio_dac_set_enable_bit(1);
        audio->stat |= (DAC_DMA_IRQ_ENABLE | DAC_IS_OPENED);

        audio_dac_set_analog_mute(0);
#if AUD_USE_EXT_PA
        if(audio->volume != 0)
            audio_dac_eable_mute(0);
#endif
        codec_printf("[icodec]:open sound device \n");
    }

    return RT_EOK;
}

static rt_size_t audio_codec_write(rt_device_t dev, rt_off_t pos,
                                   const void *buffer, rt_size_t size)
{
    int ret;
    struct audio_codec_device *audio = RT_NULL;
	rt_uint32_t result;

    audio = (struct audio_codec_device *)dev;
    ret = rt_data_node_write(audio->node_list, (void *)buffer, size);

#ifdef PAUSE_EN
	if (audio->paused)
	{    
	    result = rt_data_node_read(audio->node_list, (void*)audio->fill_pos, audio->fill_size);

		if (result == audio->fill_size)
		{
		    audio->paused = 0;
			dac_dma_addr_reset();
		}
		else
		{
		    audio->fill_pos += result;
			audio->fill_size -= result;
		}
	}
#endif
    return ret;
}

static rt_err_t audio_codec_control(rt_device_t dev, int cmd, void *args)
{
    rt_err_t result = RT_EOK, stat;
    struct audio_codec_device *audio = RT_NULL;

    audio = (struct audio_codec_device *)dev;
    stat = audio->stat;

    switch (cmd)
    {
    case CODEC_CMD_SET_VOLUME:
    {
        rt_uint32_t volume = *(rt_uint32_t *)args;

        rt_kprintf("set_volume %d-%d\n", volume, audio->volume);
        if(audio->volume != volume)
        {
            audio->volume = volume;
            audio_dac_set_volume(volume);
        }
        break;
    }

    case CODEC_CMD_SAMPLERATE:
    {
        rt_uint32_t freq = *(rt_uint32_t *)args;

#if !CONFIG_SOUND_MIXER
        #if AUD_USE_EXT_PA
		rt_kprintf("\r\n\r\n===%s:%d===\r\n",__FUNCTION__,__LINE__);
        audio_dac_eable_mute(1);
        #endif
#endif
        if ((audio->stat & DAC_IS_OPENED))
        {
            audio_dac_set_enable_bit(0);
            dac_dma_enable(0);
            audio_dac_set_sample_rate(freq);
            audio_dac_set_enable_bit(1);
            dac_dma_enable(1);
        }
        else 
        {
            audio_dac_set_sample_rate(freq);
        }

        rt_kprintf("set_dac_sample_rate %d \n", freq);
        
        #if AUD_USE_EXT_PA
        if ((audio->stat & DAC_IS_OPENED) && (audio->volume != 0))
        {
        	rt_kprintf("\r\n\r\n===%s:%d===\r\n",__FUNCTION__,__LINE__);
#if !CONFIG_SOUND_MIXER
            audio_dac_eable_mute(0);
#endif
        }
        #endif

        break;
    }

	case CODEC_CMD_RESET:
	{
		rt_data_node_empty(audio->node_list); 
		break;       
	}

    default:
        result = RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t audio_codec_close(rt_device_t dev)
{
    struct audio_codec_device *audio = RT_NULL;
    rt_uint32_t stat;

    audio = (struct audio_codec_device *)dev;
    stat = audio->stat;

    /* DAC */
    if (((stat & DAC_DMA_IRQ_ENABLE) || (stat & DAC_IRQ_ENABLE)) && 
        (stat & DAC_IS_OPENED))
    {
#if AUD_USE_EXT_PA
        audio_dac_eable_mute(1);
#endif

        // wait_node_free(audio->node_list);
		rt_data_node_empty(audio->node_list); 

        //dac_dma_enable(0);
        audio_dac_set_analog_mute(1);
       //audio_dac_set_enable_bit(0);
       //audio_dac_set_int_enable_bit(0);
       //audio_dac_close_analog_regs();
        stat &= ~(DAC_DMA_IRQ_ENABLE | DAC_IRQ_ENABLE | DAC_IS_OPENED);
    }

    audio->stat = stat;
    codec_printf("\n[icodec]:close sound device \n");

    return RT_EOK;
}

static void data_node_read_complete(struct rt_data_node *node, void *user_data)
{
    struct rt_device *dev = RT_NULL;

    // codec_printf("<func:%s> <line:%d>, read complete \n", __FUNCTION__, __LINE__);
    dev = (struct rt_device *)user_data;
    if (dev->tx_complete != RT_NULL)
    {
        dev->tx_complete(dev, node->data_ptr);
    }
}

static int print_cnt = 0;
void dac_dma_half_handler(UINT32 flag)
{
    int result;
    struct audio_codec_device *audio = RT_NULL;

    audio = &_g_audio_codec;
    result = rt_data_node_is_empty(audio->node_list);
    if (result)
    {
    #ifdef PAUSE_EN
        dac_dma_pause_addr_set((UINT32)audio->send_fifo + (AUDIO_SEND_BUFFER_SIZE -4));
		audio->paused = 1;
		audio->fill_pos = (UINT32)audio->send_fifo;
		audio->fill_size = AUDIO_SEND_BUFFER_SIZE / 2;
	#endif
		
        //rt_kprintf("#1 ");
        if (print_cnt == 0)
        {
            // msh_exec("audio_dump", strlen("audio_dump"));
            // msh_exec("stream_pipe_dump", strlen("stream_pipe_dump"));
        }
        print_cnt ++;
        memset(audio->send_fifo, 0, AUDIO_SEND_BUFFER_SIZE / 2);
    }
    else
    {
        print_cnt = 0;
        memset(audio->send_fifo, 0, AUDIO_SEND_BUFFER_SIZE / 2);
        result = rt_data_node_read(audio->node_list, audio->send_fifo, AUDIO_SEND_BUFFER_SIZE / 2);

     #ifdef PAUSE_EN
		if (result < (AUDIO_SEND_BUFFER_SIZE / 2))
		{
			dac_dma_pause_addr_set((UINT32)audio->send_fifo + (AUDIO_SEND_BUFFER_SIZE -4));
			audio->paused = 1;
		    audio->fill_pos = (UINT32)audio->send_fifo + result;
		    audio->fill_size = AUDIO_SEND_BUFFER_SIZE / 2 - result;
		}
	#endif
    }
}

void dac_dma_finish_handler(UINT32 flag)
{
    int result;
    struct audio_codec_device *audio = RT_NULL;

    audio = &_g_audio_codec;
    audio->dma_irq_cnt ++;
    result = rt_data_node_is_empty(audio->node_list);
    if (result)
    {
    #ifdef PAUSE_EN
        dac_dma_pause_addr_set((UINT32)audio->send_fifo + (AUDIO_SEND_BUFFER_SIZE / 2 -4));
		audio->paused = 1;
		audio->fill_pos = (UINT32)audio->send_fifo + AUDIO_SEND_BUFFER_SIZE / 2;
		audio->fill_size = AUDIO_SEND_BUFFER_SIZE / 2;
	#endif
        //rt_kprintf("* ");
        memset(audio->send_fifo + (AUDIO_SEND_BUFFER_SIZE / 2), 0, AUDIO_SEND_BUFFER_SIZE / 2);
    }
    else
    {
        memset(audio->send_fifo + (AUDIO_SEND_BUFFER_SIZE / 2), 0, AUDIO_SEND_BUFFER_SIZE / 2);
        result = rt_data_node_read(audio->node_list, audio->send_fifo + (AUDIO_SEND_BUFFER_SIZE / 2), AUDIO_SEND_BUFFER_SIZE / 2);
	#ifdef PAUSE_EN
		if (result < (AUDIO_SEND_BUFFER_SIZE / 2))
		{
			dac_dma_pause_addr_set((UINT32)audio->send_fifo + (AUDIO_SEND_BUFFER_SIZE / 2 -4));
			audio->paused = 1;
		    audio->fill_pos = (UINT32)audio->send_fifo + AUDIO_SEND_BUFFER_SIZE / 2 + result;
		    audio->fill_size = AUDIO_SEND_BUFFER_SIZE / 2 - result;
		}
	#endif
    }
}

int dac_dma_init(struct audio_codec_device *audio)
{
    GDMACFG_TPYES_ST init_cfg;
    GDMA_CFG_ST en_cfg;

    memset(&init_cfg, 0, sizeof(GDMACFG_TPYES_ST));
    memset(&en_cfg, 0, sizeof(GDMA_CFG_ST));

    init_cfg.dstdat_width = 32;
    init_cfg.srcdat_width = 32;
    init_cfg.dstptr_incr = 0;
    init_cfg.srcptr_incr = 1;

    init_cfg.src_start_addr = audio->send_fifo;
    init_cfg.dst_start_addr = (void *)AUD_DAC_FIFO_PORT;

    init_cfg.channel = AUDIO_DAC_DEF_DMA_CHANNEL;
    init_cfg.prio = 0;
    init_cfg.u.type4.src_loop_start_addr = audio->send_fifo;
    init_cfg.u.type4.src_loop_end_addr = audio->send_fifo + AUDIO_SEND_BUFFER_SIZE;

    init_cfg.half_fin_handler = dac_dma_half_handler;
    init_cfg.fin_handler = dac_dma_finish_handler;

    init_cfg.src_module = GDMA_X_SRC_DTCM_RD_REQ;
    init_cfg.dst_module = GDMA_X_DST_AUDIO_RX_REQ;

    sddev_control(GDMA_DEV_NAME, CMD_GDMA_CFG_TYPE4, &init_cfg);

    en_cfg.channel = AUDIO_DAC_DEF_DMA_CHANNEL;
    en_cfg.param = AUDIO_SEND_BUFFER_SIZE; // dma translen
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_TRANS_LENGTH, &en_cfg);
}

#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops audio_icodec_ops =
{
    audio_codec_init,
    audio_codec_open,
    audio_codec_close,
    RT_NULL,
    audio_codec_write,
    audio_codec_control
};
#endif /* RT_USING_DEVICE_OPS */

int rt_audio_codec_hw_init(void)
{
    int result = RT_EOK;
    struct audio_codec_device *audio = &_g_audio_codec;
    struct rt_device *device = &audio->parent;

    if (audio->send_fifo)
    {
        return RT_EOK;
    }

    audio_init();
	
#if (USING_DEMO_BOARD == 1)
    audio_dac_volume_use_single_port();
#else
    audio_dac_volume_diff_port();
#endif
    //audio_dac_volume_use_single_port();
    
    audio->send_fifo = sdram_malloc(AUDIO_SEND_BUFFER_SIZE);
    if ((UINT32)audio->send_fifo == RT_NULL)
    {
        result = -RT_ENOMEM;
        goto __exit;
    }
    memset(audio->send_fifo, 0, AUDIO_SEND_BUFFER_SIZE);

    rt_data_node_init(&audio->node_list, 12 /* AUDIO_DEVICE_MP_CNT + MP_BLOCK_COUNT + 1 */);
    audio->node_list->read_complete = data_node_read_complete;
    audio->node_list->user_data = audio;

    dac_dma_init(audio);

    audio->parent.type = RT_Device_Class_Sound;
    audio->parent.rx_indicate = RT_NULL;
    audio->parent.tx_complete = RT_NULL;
    audio->parent.user_data   = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops = &audio_icodec_ops;
#else
    device->control = audio_codec_control;
    device->init    = audio_codec_init;
    device->open    = audio_codec_open;
    device->close   = audio_codec_close;
    device->read    = RT_NULL;
    device->write   = audio_codec_write;
#endif /* RT_USING_DEVICE_OPS */

    /* register the device */
    rt_device_register(&audio->parent, "sound", 
        RT_DEVICE_FLAG_STANDALONE | RT_DEVICE_FLAG_WRONLY | RT_DEVICE_FLAG_DMA_TX);

    rt_device_init(&audio->parent);

    return RT_EOK;

__exit:
    if (audio->send_fifo)
    {
        rt_free(audio->send_fifo);
        audio->send_fifo = RT_NULL;
    }

    return result;
}

INIT_DEVICE_EXPORT(rt_audio_codec_hw_init);

#include "general_dma.h"
int audio_dump(void)
{
    int val;
    int i = 0;
    struct rt_data_node_list *node_list = _g_audio_codec.node_list;

    rt_kprintf("write index = %d \n", node_list->write_index);
    rt_kprintf("read index = %d \n", node_list->read_index);
    rt_kprintf("read offset = %d \n", node_list->data_offset);
    rt_kprintf("size = %d \n", node_list->size);
    rt_kprintf("audio->dma_irq_cnt = %d \n", _g_audio_codec.dma_irq_cnt);
}

MSH_CMD_EXPORT(audio_dump, audio_dump);

#endif // #if (CFG_SOC_NAME == SOC_BK7221U)