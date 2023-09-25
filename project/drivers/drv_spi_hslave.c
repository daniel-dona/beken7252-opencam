#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>

#include "include.h"
#include "arm_arch.h"

#include "icu_pub.h"
#include "drv_spi.h"

#ifdef BEKEN_USING_SPI_HSLAVE

#if (!CFG_USE_HSLAVE_SPI)
#error "spi psram need 'CFG_USE_HSLAVE_SPI ENABLE' !"
#endif

#include "spidma_pub.h"
#include "drv_model_pub.h"
#include "mem_pub.h"
#if CFG_GENERAL_DMA
#include "general_dma_pub.h"
#endif

#include "audio_pipe.h"

#define SPI_HSLAVE_DEBUG           1
#include "uart_pub.h"
#if SPI_HSLAVE_DEBUG
#define SPI_HSLAVE_PRT             os_printf
#define SPI_HSLAVE_WPRT            warning_prf
#define SPI_HSLAVE_FATAL           fatal_prf
#else
#define SPI_HSLAVE_PRT             null_prf
#define SPI_HSLAVE_WPRT            null_prf
#define SPI_HSLAVE_FATAL           null_prf
#endif

#define SPI_HSLAVE_RX_TIMEOUT      (120 * 500)
#ifndef SPI_HSLAVE_RX_TIMEOUT
#define SPI_HSLAVE_RX_TIMEOUT      SPIDMA_DEF_RXDATA_TIMEOUT_VAL
#endif

#define SPI_HSLAVE_RX_NODE_LEN     (1024)
#define SPI_HSLAVE_RX_LEN          (SPI_HSLAVE_RX_NODE_LEN * 4)
#define SPI_HSLAVE_RX_DMA_CHNAL    GDMA_CHANNEL_5 

#define SPI_HSLAVE_TX_NODE_LEN     (1024)
#define SPI_HSLAVE_TX_LEN          (SPI_HSLAVE_TX_NODE_LEN * 2)
#define SPI_HSLAVE_TX_DMA_CHNAL    GDMA_CHANNEL_4 

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

typedef struct spi_high_slave_device
{
    /* inherit from rt_device */
    struct rt_device parent;
    
    DD_HANDLE dd_hdl;
    SPIDMA_DESC_ST desc;

    rt_uint8_t *rx_fifo;
    struct rt_audio_pipe rx_pipe;

    struct rt_semaphore tx_sem;
    struct rt_mutex tx_lock;
} SPI_HS_DEV;

SPI_HS_DEV _spi_hs;

static void spi_hslave_rx_handler(void *curptr, UINT32 newlen, 
    UINT32 is_eof, UINT32 frame_len)
{
    SPI_HS_DEV *spi_hs = &_spi_hs;

    // copy(actually only need change write ptr of pipe)
    rt_device_write(&spi_hs->rx_pipe.parent, 
                        0, 
    					curptr, 
    					newlen);

    //SPI_HSLAVE_PRT("spi_hs rx:%d\r\n", newlen);
}

static void spi_hslave_end_frame_handler(void)
{
    //tvideo_intfer_send_msg(TV_INT_POLL);
    //SPI_HSLAVE_PRT("spi_hs rx end\r\n");
}

#if CFG_GENERAL_DMA
static void spi_hslave_node_rx_handler(UINT32 dma)
{   
    SPI_HS_DEV *spi_hs = &_spi_hs;
    SPIDMA_DESC_PTR desc = &(spi_hs->desc);
    
    UINT16 already_len = desc->rx_read_len;
    UINT16 copy_len = desc->node_len - (already_len % desc->node_len);
    
    GLOBAL_INT_DECLARATION();
    
    if(desc->node_full_handler != NULL) {
        desc->node_full_handler(desc->rxbuf + already_len, copy_len, 0, 0);
    }

    already_len += copy_len;
    already_len = already_len % desc->rxbuf_len;

    GLOBAL_INT_DISABLE();
    desc->rx_read_len = already_len;
    GLOBAL_INT_RESTORE();
}
#endif

static void spi_hslave_tx_end_handler(void)
{
    SPI_HS_DEV *spi_hs = &_spi_hs;
    rt_sem_release(&(spi_hs->tx_sem));

    //os_printf("tx_handler\r\n");
}

static void spi_hslave_data_end_handler(void)
{   
    #if CFG_GENERAL_DMA
    GDMA_CFG_ST en_cfg;
    SPI_HS_DEV *spi_hs = &_spi_hs;
    SPIDMA_DESC_PTR desc = &(spi_hs->desc);
    
    UINT16 already_len = desc->rx_read_len;
    UINT32 channel = desc->dma_rx_channel;
    int left_len = sddev_control(GDMA_DEV_NAME, CMD_GDMA_GET_LEFT_LEN, (void*)channel);
    int rec_len = desc->node_len - left_len - (already_len % desc->node_len);
    
    GLOBAL_INT_DECLARATION();
    
    if((desc->node_full_handler != NULL) && (rec_len > 0)) {
        desc->node_full_handler(desc->rxbuf + already_len, rec_len, 0, 0);
    }

    already_len += rec_len;
    already_len = already_len % desc->rxbuf_len;

    GLOBAL_INT_DISABLE();
    desc->rx_read_len = already_len;
    GLOBAL_INT_RESTORE();
    #endif

    if((desc->data_end_handler)) {
        desc->data_end_handler();
    }
}

static void spi_hslave_config_desc(void)
{
    SPI_HS_DEV *spi_hs = &_spi_hs;
    SPIDMA_DESC_PTR desc = &(spi_hs->desc);
    rt_uint8_t *rx_buf = spi_hs->rx_fifo;
    
    os_memset(desc, 0, sizeof(SPIDMA_DESC_ST));

    desc->rxbuf = rx_buf;
    desc->rxbuf_len = SPI_HSLAVE_RX_LEN;
    desc->node_len = SPI_HSLAVE_RX_NODE_LEN;
    desc->rx_read_len = 0;

    desc->node_full_handler = spi_hslave_rx_handler;
    desc->data_end_handler = NULL;//spi_hslave_end_frame_handler;

    desc->txbuf = NULL;
    desc->txbuf_len = 0;
    desc->tx_handler = spi_hslave_tx_end_handler;
    
    desc->mode = 0;
    desc->timeout_val = SPI_HSLAVE_RX_TIMEOUT;
    desc->end_frame_handler = spi_hslave_data_end_handler;

 #if CFG_GENERAL_DMA
    desc->dma_rx_handler = spi_hslave_node_rx_handler;
    desc->dma_rx_channel = SPI_HSLAVE_RX_DMA_CHNAL;
    
    desc->dma_tx_handler = NULL; //spi_hslave_node_tx_handler;
    desc->dma_tx_channel = SPI_HSLAVE_TX_DMA_CHNAL;
#endif   
}

/*---------------------------------------------------------------------------*/
static rt_err_t spi_hsalve_control(rt_device_t dev, int cmd, void *args)
{

    return RT_EOK;
}

static rt_err_t spi_hsalve_init(rt_device_t dev)
{

    return RT_EOK;
}

static rt_err_t spi_hsalve_open(rt_device_t dev, rt_uint16_t oflag)
{
    UINT32 status;
    SPI_HS_DEV *spi_hs = &_spi_hs;
    
    spi_hslave_config_desc();

    rt_device_open(&(spi_hs->rx_pipe.parent), RT_DEVICE_OFLAG_RDONLY);
    
    spi_hs->dd_hdl = ddev_open(SPIDMA_DEV_NAME, &status, (UINT32)&spi_hs->desc);
    SPI_HSLAVE_FATAL("spi_hslave_init, %p\r\n", spi_hs->dd_hdl);

	// clear all rx semp for this time
    while(rt_sem_trytake(&(spi_hs->tx_sem)) != -RT_ETIMEOUT);

    return RT_EOK;
}

static rt_err_t spi_hsalve_close(rt_device_t dev)
{
    SPI_HS_DEV *spi_hs = &_spi_hs;
    DD_HANDLE dd_hdl = spi_hs->dd_hdl;
    
    SPI_HSLAVE_FATAL("spi_hslave_deinit, %p\r\n", dd_hdl);

    rt_sem_release(&(spi_hs->tx_sem));

    ddev_close(dd_hdl);
    os_memset(&(spi_hs->desc), 0, sizeof(SPIDMA_DESC_ST));

    return RT_EOK;
}

static rt_size_t spi_hsalve_write(rt_device_t dev, rt_off_t pos,
                                   const void *buffer, rt_size_t size)
{
    int ret = 0;
    SPI_HS_DEV *spi_hs = &_spi_hs;
    DD_HANDLE dd_hdl = spi_hs->dd_hdl;
    SPIDMA_TXDMA_ST cfg;

    rt_mutex_take(&(spi_hs->tx_lock), RT_WAITING_FOREVER);

	// clear all rx semp for this time
    while(rt_sem_trytake(&(spi_hs->tx_sem)) != -RT_ETIMEOUT);

    cfg.txbuf = (UINT8*)buffer;
    cfg.tx_len = size;
    ddev_control(dd_hdl, SPIDMA_CMD_START_TX_DMA, &cfg);

    rt_kprintf("0 %p-%d\r\n", buffer, size);
    rt_sem_take(&(spi_hs->tx_sem), RT_WAITING_FOREVER);
    rt_kprintf("1 %p-%d\r\n", buffer, size);

    ddev_control(dd_hdl, SPIDMA_CMD_STOP_TX_DMA, &cfg);

    // clear all rx semp for this time
    while(rt_sem_trytake(&(spi_hs->tx_sem)) != -RT_ETIMEOUT);

    rt_mutex_release(&(spi_hs->tx_lock));

    return size;
}

static rt_size_t spi_hsalve_read(rt_device_t dev, rt_off_t pos, 
                                   void *buffer, rt_size_t size)
{
    SPI_HS_DEV *spi_hs = RT_NULL;

    spi_hs = (SPI_HS_DEV *)dev;
    return rt_device_read(&spi_hs->rx_pipe.parent, pos, buffer, size);
}

#ifdef RT_USING_DEVICE_OPS
const struct rt_device_ops spi_hsalve_ops =
{
    spi_hsalve_init,
    spi_hsalve_open,
    spi_hsalve_close,
    spi_hsalve_read,
    spi_hsalve_write,
    spi_hsalve_control
};
#endif

int rt_spi_hslave_hw_init(void)
{
    int result = RT_EOK;
    SPI_HS_DEV *spi_hs = &_spi_hs;

    if (spi_hs->rx_fifo)
    {
        return RT_EOK;
    }

    spi_hs->parent.type = RT_Device_Class_Unknown;
    spi_hs->parent.rx_indicate = RT_NULL;
    spi_hs->parent.tx_complete = RT_NULL;
    spi_hs->parent.user_data   = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    spi_hs->parent.ops = &spi_hsalve_ops;
#else
    spi_hs->parent.control = spi_hsalve_control;
    spi_hs->parent.init    = spi_hsalve_init;
    spi_hs->parent.open    = spi_hsalve_open;
    spi_hs->parent.close   = spi_hsalve_close;
    spi_hs->parent.read    = spi_hsalve_read;
    spi_hs->parent.write   = spi_hsalve_write;
#endif

    /* register the device */
    rt_device_register(&spi_hs->parent, "spi_hs", 
        RT_DEVICE_FLAG_STANDALONE | RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX);

    rt_device_init(&spi_hs->parent);

    spi_hs->rx_fifo = rt_malloc(SPI_HSLAVE_RX_LEN);
    if (spi_hs->rx_fifo == RT_NULL)
    {
        result = -RT_ENOMEM;
        goto __exit;
    }
    memset(spi_hs->rx_fifo, 0, SPI_HSLAVE_RX_LEN);

    rt_audio_pipe_init(&spi_hs->rx_pipe,
                           "spi_hs_rx",
                           RT_PIPE_FLAG_FORCE_WR | RT_PIPE_FLAG_BLOCK_RD,
                           spi_hs->rx_fifo,
                           SPI_HSLAVE_RX_LEN);

    result = rt_sem_init(&(spi_hs->tx_sem), "hspitx", 0, 0);
    if (result != RT_EOK)
    {
        rt_kprintf("[hspi]:semp failed\n");
        goto __exit;
    }
    
    result = rt_mutex_init(&(spi_hs->tx_lock), "hspitx", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("[hspi]:mutex failed\n");
        goto __exit;
    }

    return RT_EOK;

__exit:
    if (spi_hs->rx_fifo)
    {
        rt_free(spi_hs->rx_fifo);
        spi_hs->rx_fifo = RT_NULL;
    }

    return result;
}

INIT_DEVICE_EXPORT(rt_spi_hslave_hw_init);

#endif  // BEKEN_USING_SPI_HSLAVE

