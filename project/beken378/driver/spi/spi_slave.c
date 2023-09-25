#include "include.h"
#include "arm_arch.h"

#include "typedef.h"
#include "arm_arch.h"
#include "icu_pub.h"
#include "spi.h"
#include "spi_pub.h"
#include "gpio_pub.h"
#include "sys_ctrl_pub.h"

#include "drv_model_pub.h"
#include "mem_pub.h"
#if CFG_SUPPORT_ALIOS
#include "ll.h"
#endif
#include "sys_config.h"
#include "error.h"
#include "bk_rtos_pub.h"

#if CFG_USE_SPI_SLAVE

#define SPI_SLAVE_RX_FIFO_LEN      (512)

struct spi_rx_fifo
{
    UINT8 *buffer;

    UINT16 put_index, get_index;

    UINT32 is_full;
};

struct bk_spi_slave_dev
{
    UINT32 flag;

    beken_semaphore_t tx_sem;
    UINT8 *tx_ptr;
    UINT32 tx_len;
    
    beken_semaphore_t rx_sem;
    struct spi_rx_fifo *rx_fifo;

    beken_mutex_t mutex;
};

static struct bk_spi_slave_dev *spi_slave_dev;

static UINT32 bk_spi_slave_get_rx_fifo(void)
{
    UINT32 rx_length;
    struct spi_rx_fifo* rx_fifo = spi_slave_dev->rx_fifo;
    GLOBAL_INT_DECLARATION();
    
    /* get rx length */
    GLOBAL_INT_DISABLE();
    
    rx_length = (rx_fifo->put_index >= rx_fifo->get_index)? 
        (rx_fifo->put_index - rx_fifo->get_index) :
        (SPI_SLAVE_RX_FIFO_LEN - (rx_fifo->get_index - rx_fifo->put_index));
        
    GLOBAL_INT_RESTORE();

    return rx_length;
}

static void bk_spi_slave_spi_rx_callback(int is_rx_end, void *param)
{
    UINT8 ch;
    struct spi_rx_fifo *rx_fifo;
    //GLOBAL_INT_DECLARATION();
    
    rx_fifo = (struct spi_rx_fifo*)spi_slave_dev->rx_fifo;
    ASSERT(rx_fifo != RT_NULL);
    
    //REG_WRITE((0x00802800+(18*4)), 0x02);
    while (1)
    {
        if(spi_read_rxfifo(&ch) == 0)
            break;

        //GLOBAL_INT_DISABLE();

        rx_fifo->buffer[rx_fifo->put_index] = ch;
        rx_fifo->put_index += 1;
        if (rx_fifo->put_index >= SPI_SLAVE_RX_FIFO_LEN) 
            rx_fifo->put_index = 0;

        if (rx_fifo->put_index == rx_fifo->get_index)
        {
            rx_fifo->get_index += 1;
            rx_fifo->is_full = RT_TRUE;
            if (rx_fifo->get_index >= SPI_SLAVE_RX_FIFO_LEN) 
                rx_fifo->get_index = 0;
        }
        //GLOBAL_INT_RESTORE();

        if(spi_slave_dev->tx_ptr == NULL)
            spi_write_txfifo(0xFF);
    }
   
    if(is_rx_end)
    {
        // only rx end happened, wake up rx_semp
        //os_printf("----> rx end\r\n");
        bk_rtos_set_semaphore(&spi_slave_dev->rx_sem);
    }
    //REG_WRITE((0x00802800+(18*4)), 0x00);
}

static int bk_spi_slave_get_rx_data(UINT8 *rx_buf, int len)
{ 
    struct spi_rx_fifo *rx_fifo;
    rx_fifo = (struct spi_rx_fifo *)spi_slave_dev->rx_fifo;
    int size = len;

    ASSERT(rx_fifo != RT_NULL);

    if(rx_buf == NULL)
        return 0;

    //os_printf("%d %d %d\r\n", _spi_get_rx_fifo(), 
    //    rx_fifo->get_index, rx_fifo->put_index);
    
    while (size) 
    {    
        rt_uint8_t ch;
        GLOBAL_INT_DECLARATION();

        GLOBAL_INT_DISABLE();

        if ((rx_fifo->get_index == rx_fifo->put_index) 
            && (rx_fifo->is_full == RT_FALSE))
        {
            GLOBAL_INT_RESTORE();
            break;
        }

        ch = rx_fifo->buffer[rx_fifo->get_index];
        rx_fifo->get_index += 1;
        if (rx_fifo->get_index >= SPI_SLAVE_RX_FIFO_LEN) 
            rx_fifo->get_index = 0;

        if (rx_fifo->is_full == RT_TRUE)
        {
            rx_fifo->is_full = RT_FALSE;
        }

        GLOBAL_INT_RESTORE();

        *rx_buf = ch & 0xff;
        rx_buf ++; 
        size --;
    }
    
    return (len - size);
}

static void bk_spi_slave_tx_needwrite_callback(int port, void *param)
{
    UINT8 *tx_ptr = spi_slave_dev->tx_ptr;
    UINT32 tx_len = spi_slave_dev->tx_len;
    GLOBAL_INT_DECLARATION();
    
    if(tx_ptr && tx_len) 
    {
        UINT8 data = *tx_ptr;
        
        while(spi_write_txfifo(data) == 1)
        {
            spi_read_rxfifo(&data);
            tx_len --;
            tx_ptr ++;
            if(tx_len == 0) 
            {
                UINT32 enable = 0;
                spi_ctrl(CMD_SPI_TXINT_EN, (void *)&enable);
                break;
            }
            data = *tx_ptr;
        }
    } 
    else
    {
        //rt_kprintf("nw:%p,%d\r\n", tx_ptr, tx_len);
        while(spi_write_txfifo(0xff))
        {
            if(tx_len)
            {
                tx_len--;
            }
            
            if(tx_len == 0) 
            {
                UINT32 enable = 0;
                spi_ctrl(CMD_SPI_TXINT_EN, (void *)&enable);
                break;
            }
        }
    }

    GLOBAL_INT_DISABLE();
    spi_slave_dev->tx_ptr = tx_ptr;
    spi_slave_dev->tx_len = tx_len;
    GLOBAL_INT_RESTORE();
    
}

static void bk_spi_slave_tx_finish_callback(int port, void *param)
{
    if((spi_slave_dev->tx_len == 0) && (spi_slave_dev->tx_ptr))
    {
        if((spi_slave_dev->flag & TX_FINISH_FLAG) == 0)
        {
            spi_slave_dev->flag |= TX_FINISH_FLAG;
            bk_rtos_set_semaphore(&spi_slave_dev->tx_sem);
        }
    }
}

static void bk_spi_slave_configure(UINT32 rate, UINT32 mode)
{
    UINT32 param;
    struct spi_callback_des spi_dev_cb;

    /* data bit width */
    param = 0;
    spi_ctrl(CMD_SPI_SET_BITWIDTH, (void *)&param);

    /* baudrate */
    BK_SPI_PRT("max_hz = %d \n", rate);
    spi_ctrl(CMD_SPI_SET_CKR, (void *)&rate);

    /* mode */
    if (mode & BK_SPI_CPOL)
    {
        param = 1;
        spi_ctrl(CMD_SPI_SET_CKPOL, (void *)&param);
    }
    else
    {
        param = 0;
        spi_ctrl(CMD_SPI_SET_CKPOL, (void *)&param);
    }

    /* CPHA */
    if (mode & BK_SPI_CPHA)
    {
        param = 1;
        spi_ctrl(CMD_SPI_SET_CKPHA, (void *)&param);
    }
    else
    {
        param = 0;
        spi_ctrl(CMD_SPI_SET_CKPHA, (void *)&param);
    }

    /* slave */
    param = 0;
    spi_ctrl(CMD_SPI_SET_MSTEN, (void *)&param);
    param = 1;
    spi_ctrl(CMD_SPI_SET_NSSID, (void *)&param);
    param = 0;
    spi_ctrl(CMD_SPI_INIT_MSTEN, (void *)&param);

    /* set call back func */
    spi_dev_cb.callback = bk_spi_slave_spi_rx_callback;
    spi_dev_cb.param = NULL;
    spi_ctrl(CMD_SPI_SET_RX_CALLBACK, (void *)&spi_dev_cb);

    spi_dev_cb.callback = bk_spi_slave_tx_needwrite_callback;
    spi_dev_cb.param = NULL;
    spi_ctrl(CMD_SPI_SET_TX_NEED_WRITE_CALLBACK, (void *)&spi_dev_cb);

    spi_dev_cb.callback = bk_spi_slave_tx_finish_callback;
    spi_dev_cb.param = NULL;
    spi_ctrl(CMD_SPI_SET_TX_FINISH_CALLBACK, (void *)&spi_dev_cb);

    /* enable rx int */
    param = 1;
    spi_ctrl(CMD_SPI_RXINT_EN, (void *)&param);

    /* enable spi */
    param = 1;
    spi_ctrl(CMD_SPI_UNIT_ENABLE, (void *)&param);

    BK_SPI_PRT("spi_slave [CTRL]:0x%08x \n", REG_READ(SPI_CTRL));
}

static void bk_spi_slave_unconfigure(void)
{
    spi_ctrl(CMD_SPI_DEINIT, NULL);
}

int bk_spi_slave_xfer(struct spi_message *msg)
{
    UINT8 *recv_ptr = RT_NULL;
    const UINT8 *send_ptr = RT_NULL;
    UINT32 param, send_len, recv_len;
    GLOBAL_INT_DECLARATION();

    ASSERT(spi_slave_dev != NULL);
    ASSERT(msg != NULL);

    bk_rtos_lock_mutex(&spi_slave_dev->mutex);

    recv_ptr = msg->recv_buf;
    recv_len = msg->recv_len;
    send_ptr = msg->send_buf;
    send_len = msg->send_len;

    if((send_ptr) && send_len) 
    {
        GLOBAL_INT_DISABLE();
        spi_slave_dev->tx_ptr = (UINT8 *)send_ptr;
        spi_slave_dev->tx_len = send_len;
        spi_slave_dev->flag &= ~(TX_FINISH_FLAG);
        GLOBAL_INT_RESTORE();

        param = 1;
        spi_ctrl(CMD_SPI_TXINT_EN, (void *)&param);

        os_printf("0 %p-%d\r\n", send_ptr, send_len);
        bk_rtos_get_semaphore(&spi_slave_dev->tx_sem, BEKEN_NEVER_TIMEOUT);

        param = 0;
        spi_ctrl(CMD_SPI_TXINT_EN, (void *)&param);

        GLOBAL_INT_DISABLE();
        spi_slave_dev->tx_ptr = NULL;
        spi_slave_dev->tx_len = 0;
        spi_slave_dev->flag |= TX_FINISH_FLAG;
        GLOBAL_INT_RESTORE();
        
        os_printf("1 %p-%d\r\n", send_ptr, send_len);
        param = send_len;
    } 
    else if((recv_ptr) && recv_len) 
    {
        OSStatus err;
        int len;

        GLOBAL_INT_DISABLE();
        spi_slave_dev->tx_ptr = NULL;
        spi_slave_dev->tx_len = recv_len;
        GLOBAL_INT_RESTORE();

        param = 1;
        spi_ctrl(CMD_SPI_TXINT_EN, (void *)&param);

        do {
            len = bk_spi_slave_get_rx_data(recv_ptr, recv_len);
            if(len == 0)
            {
                err = bk_rtos_get_semaphore(&spi_slave_dev->rx_sem, RT_WAITING_FOREVER);
                if(err != kNoErr)
                    break;
            }
        } while(len == 0);

        param = 0;
        spi_ctrl(CMD_SPI_TXINT_EN, (void *)&param);
        
        // clear all rx semp for this time
        do {
            err = bk_rtos_get_semaphore(&spi_slave_dev->rx_sem, 0);
        }
        while(err == kNoErr);

        param = len;
    }

    bk_rtos_unlock_mutex(&spi_slave_dev->mutex);

    return param;
}

int bk_spi_slave_deinit(void);
int bk_spi_slave_init(UINT32 rate, UINT32 mode)
{
    OSStatus result = 0;

    if (spi_slave_dev)
    {
        bk_spi_slave_deinit();
    }

    spi_slave_dev = os_malloc(sizeof(struct bk_spi_slave_dev));
    if (!spi_slave_dev)
    {
        BK_SPI_PRT("[spi]:malloc memory for spi_dev failed\n");
        result = -1;
        goto _exit;
    }
    os_memset(spi_slave_dev, 0, sizeof(struct bk_spi_slave_dev));


#if CFG_SUPPORT_ALIOS
    result = bk_rtos_init_semaphore(&spi_slave_dev->tx_sem, 0);
#else
    result = bk_rtos_init_semaphore(&spi_slave_dev->tx_sem, 1);
#endif
    if (result != kNoErr)
    {
        BK_SPI_PRT("[spi]: spi tx semp init failed\n");
        goto _exit;
    }

#if CFG_SUPPORT_ALIOS
    result = bk_rtos_init_semaphore(&spi_slave_dev->rx_sem, 0);
#else
    result = bk_rtos_init_semaphore(&spi_slave_dev->rx_sem, 1);
#endif
    if (result != kNoErr)
    {
        BK_SPI_PRT("[spi]: spi rx semp init failed\n");
        goto _exit;
    }

    result = bk_rtos_init_mutex(&spi_slave_dev->mutex);
    if (result != kNoErr)
    {
        BK_SPI_PRT("[spi]: spi mutex init failed\n");
        goto _exit;
    }
    
    struct spi_rx_fifo* rx_fifo;

    rx_fifo = (struct spi_rx_fifo*)os_malloc(sizeof(struct spi_rx_fifo) +
        SPI_SLAVE_RX_FIFO_LEN);
    if(!rx_fifo)
    {
        BK_SPI_PRT("[spi]: spi rx fifo malloc failed\n");
        goto _exit;
    }
    
    rx_fifo->buffer = (rt_uint8_t*) (rx_fifo + 1);
    os_memset(rx_fifo->buffer, 0, SPI_SLAVE_RX_FIFO_LEN);
    rx_fifo->put_index = 0;
    rx_fifo->get_index = 0;
    rx_fifo->is_full = 0;

    spi_slave_dev->rx_fifo = rx_fifo;

    spi_slave_dev->tx_ptr = NULL;
    spi_slave_dev->tx_len = 0;
    spi_slave_dev->flag |= TX_FINISH_FLAG;
    
    bk_spi_slave_configure(rate, mode);
    
    return 0;

_exit:
    if(spi_slave_dev->mutex)
        bk_rtos_deinit_mutex(&spi_slave_dev->mutex);
    
    if(spi_slave_dev->tx_sem)
        bk_rtos_deinit_semaphore(&spi_slave_dev->tx_sem);

    if(spi_slave_dev->rx_sem)
        bk_rtos_deinit_semaphore(&spi_slave_dev->rx_sem);

    if(spi_slave_dev->rx_fifo)
        os_free(spi_slave_dev->rx_fifo);
    
    if (spi_slave_dev)
    {
        os_free(spi_slave_dev);
        spi_slave_dev = NULL;
    }

    return 1;
}


int bk_spi_slave_deinit(void)
{
    if (spi_slave_dev == NULL)
    {
        return 0;
    }

    bk_spi_slave_unconfigure();

    if(spi_slave_dev->mutex)
        bk_rtos_lock_mutex(&spi_slave_dev->mutex);
    
    if(spi_slave_dev->tx_sem)
        bk_rtos_deinit_semaphore(&spi_slave_dev->tx_sem);

    if(spi_slave_dev->rx_sem)
        bk_rtos_deinit_semaphore(&spi_slave_dev->rx_sem);

    if(spi_slave_dev->rx_fifo)
        os_free(spi_slave_dev->rx_fifo);

    if(spi_slave_dev->mutex) 
    {
        bk_rtos_unlock_mutex(&spi_slave_dev->mutex);
        bk_rtos_deinit_mutex(&spi_slave_dev->mutex);
    }
    
    os_free(spi_slave_dev);
    spi_slave_dev = NULL;

    return 0;
}

////////////////////////////////////////////////////////
static rt_err_t rt_spi_slave_init(rt_device_t dev)
{   
    bk_spi_slave_init(SPI_DEF_CLK_HZ, SPI_DEF_MODE);

    return RT_EOK;
}

static rt_err_t rt_spi_slave_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_spi_slave_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t rt_spi_slave_read(rt_device_t dev, rt_off_t pos,
                                   void *buffer, rt_size_t size)
{
    int ret;
    struct spi_message msg;

    if(buffer == NULL)
        return 1;

    if(size == 0)
        return 0;

    msg.send_buf = NULL;
    msg.send_len = 0;
    msg.recv_buf = buffer;
    msg.recv_len = size;

    ret = bk_spi_slave_xfer(&msg);

    return ret;
}
static rt_size_t rt_spi_slave_write(rt_device_t dev, rt_off_t pos,
                                   const void *buffer, rt_size_t size)
{
    int ret;
    struct spi_message msg;

    if(buffer == NULL)
        return 1;

    if(size == 0)
        return 0;

    msg.send_buf = (void *)buffer;
    msg.send_len = size;
    msg.recv_buf = NULL;
    msg.recv_len = 0;

    ret = bk_spi_slave_xfer(&msg);

    return ret;
}


#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops spi_slave_ops =
{
    rt_spi_slave_init,
    rt_spi_slave_open,
    rt_spi_slave_close,
    rt_spi_slave_read,
    rt_spi_slave_write,
    NULL
};
#endif /* RT_USING_DEVICE_OPS */

static struct rt_device _g_spi_slave;

int rt_spi_slave_hw_init(void)
{
    struct rt_device *device = &_g_spi_slave;

    device->type = RT_Device_Class_SPIBUS;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    device->user_data   = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops = &spi_slave_ops;
#else
    device->control = NULL;
    device->init    = rt_spi_flash_init;
    device->open    = rt_spi_flash_open;
    device->close   = rt_spi_flash_read;
    device->read    = rt_spi_flash_read;
    device->write   = rt_spi_flash_write;
#endif /* RT_USING_DEVICE_OPS */

    /* register the device */
    rt_device_register(device, "spislave", 
        RT_DEVICE_FLAG_STANDALONE | RT_DEVICE_FLAG_RDWR);

    //rt_device_init(device);

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_spi_slave_hw_init);

#if 1
void spi_slave_test(void *parameter)
{
    struct rt_device *slave;
    
    slave = rt_device_find("spislave");
    if (slave== NULL)
    {
        rt_kprintf("spi slave not found \n");
        return;
    }

    if (rt_device_init(slave) != RT_EOK)
    {
        return;
    }

    if (rt_device_open(slave, 0) != RT_EOK)
    {
        return;
    }

	while(1)
	{
		uint8_t buffer[32];
		int i;

        bk_rtos_delay_milliseconds(10000);
        
		os_printf("[spislave]: spislave test begin\n");
		for(i = 0; i < sizeof(buffer); i++)
		{
			buffer[i] = (uint8_t)i;
		}

        //rt_device_write(slave, 0, buffer, sizeof(buffer));

        #if 1
		os_memset(buffer, 0, sizeof(buffer));
        rt_device_read(slave, 0, buffer, sizeof(buffer));

		for(i = 0; i < 32; i++)
		{
		    os_printf("[%02d]: %02x - %02x\n", i, (uint8_t)i, buffer[i]);
		}
        #endif
        
		os_printf("[spislave]: spislave test end\n");
		// while(1);
	}
}

int spi_slave_samples(int argc, char *argv)
{

    rt_thread_t tid;

    tid = rt_thread_create("spi_slave",
                           spi_slave_test,
                           RT_NULL,
                           1024 * 8,
                           22,
                           10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

FINSH_FUNCTION_EXPORT_ALIAS(spi_slave_samples, __cmd_spi_slave_samples, spi slave samples);
#endif


#endif  // CFG_USE_SPI_MASTER

