#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <data_node.h>
#include <string.h>

#include "typedef.h"
#include "sys_config.h"
#include "bk_rtos_pub.h"
#include "i2s_pub.h"
#include "i2s.h"
#include "drv_i2s.h"
#include "general_dma_pub.h"
#include "board.h"

#define I2S_RX_BUFFER_SIZE     2048
#define I2S_TX_BUFFER_SIZE     2048
#define I2S_RX_DMA_CHANNEL     GDMA_CHANNEL_1
#define I2S_TX_DMA_CHANNEL     GDMA_CHANNEL_3
#define I2S_TX_NODE_COUNT      12
#define I2S_SAMPLE_RATE       (44100)
#define I2S_BIT_LENGTH        (32)
#define PAUSE_EN              1
//#define I2S_RX_CALLBACK

#define RT_I2S_TRANS_DEBUG
#ifdef  RT_I2S_TRANS_DEBUG
#define i2s_trans_dbg(fmt, ...)   rt_kprintf(fmt, ##__VA_ARGS__)
#else
#define i2s_trans_dbg(fmt, ...)
#endif
#define REG_READ(addr)          (*((volatile UINT32 *)(addr)))
#define REG_WRITE(addr, _data) 	(*((volatile UINT32 *)(addr)) = (_data))

static struct rt_i2s_bus_device g_i2s_bus;

#if 0
static void i2s_dma_mode_isr(void)
{
	uint32_t tx_done;
	uint32_t i2s_status;

	i2s_status= REG_READ(PCM_STAT);
	tx_done  = i2s_status & TX_FIFO0_EMPTY;
	if (i2s_status & TX_FIFO0_EMPTY)
	{
	    i2s_status = i2s_status ^ TX_FIFO0_EMPTY;
        if (g_i2s_bus.tx_paused)
        {
            bk_printf("#\r\n");
            i2s_dma_master_enable(0);
        }
	}

	REG_WRITE(PCM_STAT,i2s_status);
}
#endif

static void i2s_tx_node_read_complete(struct rt_data_node *node, void *user_data)
{
    struct rt_device *dev = (struct rt_device *)user_data;

    // i2s_trans_dbg("<func:%s> <line:%d>, read complete \n", __FUNCTION__, __LINE__);
    if (dev->tx_complete != RT_NULL)
    {
        dev->tx_complete(dev, node->data_ptr);
    }
}

static void i2s_dma_tx_pause_addr_set(UINT32 addr)
{
    GDMA_CFG_ST en_cfg;

    memset(&en_cfg, 0, sizeof(GDMA_CFG_ST));

    en_cfg.channel = I2S_TX_DMA_CHANNEL;
    en_cfg.param = addr;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_SRC_PAUSE_ADDR, &en_cfg);
}

static void i2s_dma_rx_pause_addr_set(UINT32 addr)
{
    GDMA_CFG_ST en_cfg;

    memset(&en_cfg, 0, sizeof(GDMA_CFG_ST));

    en_cfg.channel = I2S_RX_DMA_CHANNEL;
    en_cfg.param = addr;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DST_PAUSE_ADDR, &en_cfg);
}

static void i2s_dma_master_enable(rt_uint32_t enable)
{
    sddev_control(I2S_DEV_NAME, I2S_CMD_DMA_MASTER_ENABLE, (void *)&enable);
}

static void i2s_dma_tx_enable(rt_uint32_t enable)
{
    struct rt_i2s_bus_device *i2s = &g_i2s_bus;
    GDMA_CFG_ST en_cfg;

    i2s_trans_dbg("%s:%d tx_enabled=%d,enable=%d\r\n", __FUNCTION__, __LINE__, i2s->tx_enabled, enable);
    if ((i2s->tx_enabled && enable) || (!i2s->tx_enabled && !enable))
    {
        i2s_trans_dbg("%s:%d already enable tx dma\r\n", __FUNCTION__, __LINE__);
        return;
    }

    en_cfg.channel = I2S_TX_DMA_CHANNEL;
    if (enable)
        en_cfg.param = 1;
    else
        en_cfg.param = 0;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);

    i2s->tx_enabled = enable ? 1 : 0;
}

static void i2s_dma_rx_enable(rt_uint32_t enable)
{
    struct rt_i2s_bus_device *i2s = &g_i2s_bus;
    GDMA_CFG_ST en_cfg;

    i2s_trans_dbg("%s:%d rx_enabled=%d,enable=%d\r\n", __FUNCTION__, __LINE__, i2s->rx_enabled, enable);
    if ((i2s->rx_enabled && enable) || (!i2s->rx_enabled && !enable))
    {
        i2s_trans_dbg("%s:%d already enable rx dma\r\n", __FUNCTION__, __LINE__);
        return;
    }

    en_cfg.channel = I2S_RX_DMA_CHANNEL;
    if (enable)
        en_cfg.param = 1;
    else
        en_cfg.param = 0;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);

    i2s->rx_enabled = enable ? 1 : 0;
}

void i2s_dma_tx_half_handler(UINT32 flag)
{
    int result;
    struct rt_i2s_bus_device *i2s = &g_i2s_bus;

    rt_kprintf("%s:%d\r\n", __FUNCTION__, __LINE__);
    //rt_kprintf("%s:%d PCM_CTRL=0x%x,PCM_CN=0x%x,PCM_STAT=0x%x\r\n", __FUNCTION__, __LINE__, REG_READ(PCM_CTRL), REG_READ(PCM_CN), REG_READ(PCM_STAT));
    result = rt_data_node_is_empty(i2s->tx_list);
    if (result)
    {
#ifdef PAUSE_EN
        i2s_dma_tx_pause_addr_set((UINT32)i2s->tx_fifo + (I2S_TX_BUFFER_SIZE -4));
		i2s->tx_paused = 1;
        //i2s_dma_master_enable(!i2s->tx_paused);
		i2s->tx_fill_pos = (UINT32)i2s->tx_fifo;
		i2s->tx_fill_size = I2S_TX_BUFFER_SIZE / 2;
#endif

        memset(i2s->tx_fifo, 0, I2S_TX_BUFFER_SIZE / 2);
    }
    else
    {
        memset(i2s->tx_fifo, 0, I2S_TX_BUFFER_SIZE / 2);
        result = rt_data_node_read(i2s->tx_list, i2s->tx_fifo, I2S_TX_BUFFER_SIZE / 2);

#ifdef PAUSE_EN
		if (result < (I2S_TX_BUFFER_SIZE / 2))
		{
			i2s_dma_tx_pause_addr_set((UINT32)i2s->tx_fifo + (I2S_TX_BUFFER_SIZE -4));
			i2s->tx_paused = 1;
            //i2s_dma_master_enable(!i2s->tx_paused);
		    i2s->tx_fill_pos = (UINT32)i2s->tx_fifo + result;
		    i2s->tx_fill_size = I2S_TX_BUFFER_SIZE / 2 - result;
		}
#endif
    }
}

void i2s_dma_tx_finish_handler(UINT32 flag)
{
    int result;
    struct rt_i2s_bus_device *i2s = &g_i2s_bus;

    rt_kprintf("%s:%d\r\n", __FUNCTION__, __LINE__);
    //rt_kprintf("%s:%d PCM_CTRL=0x%x,PCM_CN=0x%x,PCM_STAT=0x%x\r\n", __FUNCTION__, __LINE__, REG_READ(PCM_CTRL), REG_READ(PCM_CN), REG_READ(PCM_STAT));
    i2s->tx_dma_irq_cnt ++;
    result = rt_data_node_is_empty(i2s->tx_list);
    if (result)
    {
    #ifdef PAUSE_EN
        i2s_dma_tx_pause_addr_set((UINT32)i2s->tx_fifo + (I2S_TX_BUFFER_SIZE / 2 -4));
		i2s->tx_paused = 1;
        //i2s_dma_master_enable(!i2s->tx_paused);
		i2s->tx_fill_pos = (UINT32)i2s->tx_fifo + I2S_TX_BUFFER_SIZE / 2;
		i2s->tx_fill_size = I2S_TX_BUFFER_SIZE / 2;
	#endif
        //rt_kprintf("* ");
        memset(i2s->tx_fifo + (I2S_TX_BUFFER_SIZE / 2), 0, I2S_TX_BUFFER_SIZE / 2);
    }
    else
    {
        memset(i2s->tx_fifo + (I2S_TX_BUFFER_SIZE / 2), 0, I2S_TX_BUFFER_SIZE / 2);
        result = rt_data_node_read(i2s->tx_list, i2s->tx_fifo + (I2S_TX_BUFFER_SIZE / 2), I2S_TX_BUFFER_SIZE / 2);
	#ifdef PAUSE_EN
		if (result < (I2S_TX_BUFFER_SIZE / 2))
		{
			i2s_dma_tx_pause_addr_set((UINT32)i2s->tx_fifo + (I2S_TX_BUFFER_SIZE / 2 -4));
			i2s->tx_paused = 1;
            //i2s_dma_master_enable(!i2s->tx_paused);
		    i2s->tx_fill_pos = (UINT32)i2s->tx_fifo + I2S_TX_BUFFER_SIZE / 2 + result;
		    i2s->tx_fill_size = I2S_TX_BUFFER_SIZE / 2 - result;
		}
	#endif
    }
}

int i2s_dma_tx_init(struct rt_i2s_bus_device *i2s)
{
    GDMACFG_TPYES_ST init_cfg;
    GDMA_CFG_ST en_cfg;

    memset(&init_cfg, 0, sizeof(GDMACFG_TPYES_ST));
    memset(&en_cfg, 0, sizeof(GDMA_CFG_ST));

    init_cfg.dstdat_width = I2S_BIT_LENGTH;
    init_cfg.srcdat_width = I2S_BIT_LENGTH;
    init_cfg.dstptr_incr = 0;
    init_cfg.srcptr_incr = 1;

    init_cfg.src_start_addr = i2s->tx_fifo;
    init_cfg.dst_start_addr = (void *)PCM_DAT0;

    init_cfg.channel = I2S_TX_DMA_CHANNEL;
    init_cfg.prio = 0;
    init_cfg.u.type4.src_loop_start_addr = i2s->tx_fifo;
    init_cfg.u.type4.src_loop_end_addr = i2s->tx_fifo + I2S_TX_BUFFER_SIZE;

    init_cfg.half_fin_handler = i2s_dma_tx_half_handler;
    init_cfg.fin_handler = i2s_dma_tx_finish_handler;

    init_cfg.src_module = GDMA_X_SRC_DTCM_RD_REQ;
    init_cfg.dst_module = GDMA_X_DST_I2S_TX_REQ;

    sddev_control(GDMA_DEV_NAME, CMD_GDMA_CFG_TYPE4, (void *)&init_cfg);

    en_cfg.channel = I2S_TX_DMA_CHANNEL;
    en_cfg.param = I2S_TX_BUFFER_SIZE; // dma translen
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_TRANS_LENGTH, (void *)&en_cfg);
}

#if defined(I2S_RX_CALLBACK)
void i2s_dma_rx_half_handler(UINT32 flag)
{
    int result;
    struct rt_i2s_bus_device *i2s = &g_i2s_bus;

    rt_kprintf("%s:%d\r\n", __FUNCTION__, __LINE__);
    //rt_kprintf("%s:%d PCM_CTRL=0x%x,PCM_CN=0x%x,PCM_STAT=0x%x\r\n", __FUNCTION__, __LINE__, REG_READ(PCM_CTRL), REG_READ(PCM_CN), REG_READ(PCM_STAT));
#ifdef PAUSE_EN
    //i2s_dma_rx_pause_addr_set((UINT32)i2s->rx_fifo + (I2S_RX_BUFFER_SIZE -4));
#endif

    int index;
    int *ptr = (int *)i2s->rx_fifo;
    for (index = 0; index < I2S_RX_BUFFER_SIZE / 8; index++, ptr++)
    {
        if (*ptr != 0)
        {
            rt_kprintf("rx[%d]=0x%x\r\n", index, *ptr);
        }
    }
}

void i2s_dma_rx_finish_handler(UINT32 flag)
{
    int result;
    struct rt_i2s_bus_device *i2s = &g_i2s_bus;

    rt_kprintf("%s:%d\r\n", __FUNCTION__, __LINE__);
    //rt_kprintf("%s:%d PCM_CTRL=0x%x,PCM_CN=0x%x,PCM_STAT=0x%x\r\n", __FUNCTION__, __LINE__, REG_READ(PCM_CTRL), REG_READ(PCM_CN), REG_READ(PCM_STAT));
#ifdef PAUSE_EN
    //i2s_dma_rx_pause_addr_set((UINT32)i2s->rx_fifo + (I2S_RX_BUFFER_SIZE / 2 -4));
#endif

    int index;
    int *ptr = (int *)i2s->rx_fifo + I2S_RX_BUFFER_SIZE / 8;
    for (index = I2S_RX_BUFFER_SIZE / 8; index < I2S_RX_BUFFER_SIZE / 4; index++, ptr++)
    {
        if (*ptr != 0)
        {
            rt_kprintf("rx[%d]=0x%x\r\n", index, *ptr);
        }
    }
}
#endif

int i2s_dma_rx_init(struct rt_i2s_bus_device *i2s)
{
    GDMACFG_TPYES_ST init_cfg;
    GDMA_CFG_ST en_cfg;

    memset(&init_cfg, 0, sizeof(GDMACFG_TPYES_ST));
    memset(&en_cfg, 0, sizeof(GDMA_CFG_ST));

    init_cfg.dstdat_width = I2S_BIT_LENGTH;
    init_cfg.srcdat_width = I2S_BIT_LENGTH;
    init_cfg.dstptr_incr = 1;
    init_cfg.srcptr_incr = 0;
    
    init_cfg.src_start_addr = (void *)PCM_DAT0;
    init_cfg.dst_start_addr = i2s->rx_fifo;
    
    init_cfg.channel = I2S_RX_DMA_CHANNEL;
    init_cfg.prio = 0;
    init_cfg.u.type5.dst_loop_start_addr = i2s->rx_fifo;
    init_cfg.u.type5.dst_loop_end_addr = i2s->rx_fifo + I2S_RX_BUFFER_SIZE;

#if defined(I2S_RX_CALLBACK)
    init_cfg.half_fin_handler = i2s_dma_rx_half_handler;
    init_cfg.fin_handler = i2s_dma_rx_finish_handler;
#endif

    init_cfg.src_module = GDMA_X_SRC_I2S_RX_REQ;
    init_cfg.dst_module = GDMA_X_DST_DTCM_WR_REQ;

    sddev_control(GDMA_DEV_NAME, CMD_GDMA_CFG_TYPE5, (void *)&init_cfg);
    
    en_cfg.channel = I2S_RX_DMA_CHANNEL;
    en_cfg.param = I2S_RX_BUFFER_SIZE; // dma translen
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_TRANS_LENGTH, (void *)&en_cfg);

#if !defined(I2S_RX_CALLBACK)
    rb_init_dma_write(&i2s->rb_dma_wr, i2s->rx_fifo, I2S_RX_BUFFER_SIZE, I2S_RX_DMA_CHANNEL);
#endif
}

static rt_err_t rt_i2s_init(rt_device_t dev)
{
    rt_kprintf("%s:%d\r\n", __FUNCTION__, __LINE__);
    i2s_init(0);
	return RT_EOK;
}

static rt_err_t rt_i2s_open(rt_device_t dev, rt_uint16_t oflag)
{
    struct rt_i2s_bus_device *i2s = (struct rt_i2s_bus_device *)dev;
    
	/* open audio , set fifo level set sample rate/datawidth */
	uint32 i2s_mode = I2S_MODE| I2S_LRCK_NO_TURN| I2S_SCK_NO_TURN| I2S_MSB_FIRST| (0<<I2S_SYNC_LENGTH_BIT)| (0<<I2S_PCM_DATA_LENGTH_BIT);

    if (!i2s->open_flag)
    {
        i2s->open_flag = 1;
        i2s->tx_dma_irq_cnt = 0;
        #if 1
        i2s->tx_paused = 1;
		i2s->tx_fill_pos = (UINT32)i2s->tx_fifo;
		i2s->tx_fill_size = I2S_TX_BUFFER_SIZE / 2;
		#endif

        //sddev_control(I2S_DEV_NAME, I2S_CMD_DMA_ISR, (void *)i2s_dma_mode_isr);
        /* rate=8/16/44.4/48 * 1000  bitlength=8/16/24/32 */
        i2s_configure(FIFO_LEVEL_32, I2S_SAMPLE_RATE, I2S_BIT_LENGTH, i2s_mode);

        if (oflag & RT_DEVICE_OFLAG_RDONLY)
        {
            rt_kprintf("%s:%d read with DMA\r\n", __FUNCTION__, __LINE__);
            //i2s_dma_master_enable(0);
            i2s_dma_rx_init(i2s);
            i2s_dma_rx_pause_addr_set(0);
            //i2s_dma_rx_enable(1);
        }
        if (oflag & RT_DEVICE_OFLAG_WRONLY)
        {
            rt_kprintf("%s:%d write with DMA\r\n", __FUNCTION__, __LINE__);
            //i2s_dma_master_enable(1);
            i2s_dma_tx_init(i2s);

            i2s_dma_tx_pause_addr_set((UINT32)i2s->tx_fifo);
            //i2s_dma_tx_enable(1);
        }

        i2s_trans_dbg("[i2s]:open device\r\n");
    }

    return RT_EOK;
}


static rt_err_t rt_i2s_close(rt_device_t dev)
{
    struct rt_i2s_bus_device *i2s = (struct rt_i2s_bus_device *)dev;

    if (i2s->open_flag)
    {
        // wait_node_free(i2s->tx_list);
        rt_data_node_empty(i2s->tx_list); 

        //i2s_dma_enable(0);
        i2s->open_flag = 0;
        i2s->tx_paused = 0;
        i2s_dma_master_enable(0);
    }

    i2s_trans_dbg("[i2s]:close device\r\n");

    return RT_EOK;
}

static rt_size_t rt_i2s_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    struct rt_i2s_bus_device *i2s = (struct rt_i2s_bus_device *)dev;
    int fill_size;
    UINT8 *read, *write;

#if !defined(I2S_RX_CALLBACK)
#if CFG_GENERAL_DMA
    fill_size = rb_get_fill_size_dma_write(&i2s->rb_dma_wr);
    if(fill_size > size)
        fill_size = size;
    rb_read_dma_write(&i2s->rb_dma_wr, (UINT8 *)buffer + pos, fill_size, 1);
#endif
#endif

    if (fill_size > 0)
    {
        rt_kprintf("%s:%d size=%d,res=%d\r\n", __FUNCTION__, __LINE__, size, fill_size);
    }

    return fill_size;
}

static rt_size_t rt_i2s_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size )
{
    int ret;
    struct rt_i2s_bus_device *i2s = (struct rt_i2s_bus_device *)dev;
    rt_uint32_t result;

    rt_kprintf("%s:%d size=%d\r\n", __FUNCTION__, __LINE__, size);
    ret = rt_data_node_write(i2s->tx_list, (void *)((UINT8 *)buffer + pos), size);

#ifdef PAUSE_EN
    if (i2s->tx_paused)
    {    
        result = rt_data_node_read(i2s->tx_list, (void*)i2s->tx_fill_pos, i2s->tx_fill_size);
        rt_kprintf("%s:%d result=%d,fill_size=%d\r\n", __FUNCTION__, __LINE__, result, i2s->tx_fill_size);

        if (result == i2s->tx_fill_size)
        {
            i2s->tx_paused = 0;
            //i2s_dma_master_enable(1);
            i2s_dma_tx_pause_addr_set(0);
        }
        else
        {
            i2s->tx_fill_pos += result;
            i2s->tx_fill_size -= result;
        }
    }
#endif
    return ret;
}



static rt_err_t rt_i2s_cotrol(rt_device_t dev, int cmd, void *args)
{
    switch (cmd)
    {
        case RT_DEVICE_CTRL_I2S_DMA_RX_ENABLE:
            i2s_dma_rx_enable(*(rt_int32_t *)args);
            break;
        case RT_DEVICE_CTRL_I2S_DMA_TX_ENABLE:
            i2s_dma_tx_enable(*(rt_int32_t *)args);
            break;
        case RT_DEVICE_CTRL_I2S_DMA_MASTER_ENABLE:
            i2s_dma_master_enable(*(rt_int32_t *)args);
            break;
    }

	return RT_ERROR;
}

#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops i2s_ops =
{
    rt_i2s_init,
    rt_i2s_open,
    rt_i2s_close,
    rt_i2s_read,
    rt_i2s_write,
    rt_i2s_cotrol
};
#endif /* RT_USING_DEVICE_OPS */

int  rt_i2s_hw_init(void)
{
    struct rt_i2s_bus_device *i2s = &g_i2s_bus;
	struct rt_device *device = &i2s->parent;

    /* set device type */
    device->type = RT_Device_Class_I2SBUS;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    device->user_data   = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops = &i2s_ops;
#else
    device->control = rt_i2s_cotrol;
    device->init    = rt_i2s_init;
    device->open    = rt_i2s_open;
    device->close   = rt_i2s_close;
    device->read    = rt_i2s_read;
    device->write   = rt_i2s_write;
#endif /* RT_USING_DEVICE_OPS */	

    i2s->tx_fifo = (char *)sdram_malloc(I2S_TX_BUFFER_SIZE);
    if (i2s->tx_fifo == RT_NULL)
    {
        rt_kprintf("%s:%d malloc tx_fifo failed\r\n", __FUNCTION__, __LINE__);
        return -RT_ENOMEM;
    }
    memset(i2s->tx_fifo, 0, I2S_TX_BUFFER_SIZE);

#if 1
    i2s->rx_fifo = (char *)sdram_malloc(I2S_RX_BUFFER_SIZE);
    if (i2s->rx_fifo == RT_NULL)
    {
        rt_kprintf("%s:%d malloc rx_fifo failed\r\n", __FUNCTION__, __LINE__);
        free(i2s->tx_fifo);
        return -RT_ENOMEM;
    }
    memset(i2s->rx_fifo, 0, I2S_RX_BUFFER_SIZE);
#endif

    rt_data_node_init(&i2s->tx_list, I2S_TX_NODE_COUNT);
    i2s->tx_list->read_complete = i2s_tx_node_read_complete;
    i2s->tx_list->user_data = i2s;

    /* register to device manager */

	rt_device_register(device, I2S_DEV_NAME, RT_DEVICE_FLAG_RDWR);

	rt_kprintf("---i2s register over---\r\n");

	return RT_EOK;

}

INIT_DEVICE_EXPORT(rt_i2s_hw_init);
