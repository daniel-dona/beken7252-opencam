#include "include.h"
#include "arm_arch.h"

#include "typedef.h"
#include "arm_arch.h"
#include "icu_pub.h"
#include "spi.h"
#include "spi_pub.h"

#include "sys_ctrl_pub.h"

#include "drv_model_pub.h"
#include "mem_pub.h"
#if CFG_SUPPORT_ALIOS
#include "ll.h"
#endif
#include "sys_config.h"
#include "error.h"
#include "bk_rtos_pub.h"

#if CFG_USE_SPI_MASTER
struct bk_spi_dev
{
    UINT8 *tx_ptr;
    UINT32 tx_len;
    beken_semaphore_t tx_sem;

    UINT8 *rx_ptr;
    UINT32 rx_len;
    UINT32 rx_offset;
    UINT32 rx_drop;

    UINT32 total_len;
    UINT32 flag;
    
    beken_mutex_t mutex;
};

static struct bk_spi_dev *spi_dev;

static void bk_spi_rx_callback(int is_rx_end, void *param)
{
    UINT8 ch, *rxbuf;
    UINT32 offset, drop;

    GLOBAL_INT_DECLARATION();
    
    rxbuf = spi_dev->rx_ptr;
    drop = spi_dev->rx_drop;
    offset = spi_dev->rx_offset;

    while (1)
    {
        if(spi_read_rxfifo(&ch) == 0)
            break;

        if(rxbuf)
        {
            if(drop != 0)
            {
                drop--;
            }
            else
            {
                if(offset < spi_dev->rx_len)
                {
                    rxbuf[offset] = ch;
                    offset++;
                }
                else
                {
                    BK_SPI_WPRT("rx over flow:%02x, %d\r\n", ch, spi_dev->rx_len);
                }
            }
        }
    }

    GLOBAL_INT_DISABLE();
    spi_dev->rx_drop = drop;
    spi_dev->rx_offset = offset;
    GLOBAL_INT_RESTORE();
}

static void bk_spi_tx_needwrite_callback(int port, void *param)
{
    UINT8 *tx_ptr = spi_dev->tx_ptr, data;
    UINT32 tx_len = spi_dev->tx_len, total_len = spi_dev->total_len, tx_ok = 0;

    UINT8 *rxbuf;
    UINT32 offset, drop;
    
    rxbuf = spi_dev->rx_ptr;
    drop = spi_dev->rx_drop;
    offset = spi_dev->rx_offset;

    GLOBAL_INT_DECLARATION();

    while(total_len) 
    {
        tx_ok = 0;
        
        if(tx_len)
        {
            data = *tx_ptr;
            if(spi_write_txfifo(data) == 1)
            {
                tx_ok = 1;
                
                tx_len --;
                tx_ptr ++;
            }
        }
        else
        {
            data = 0xff;
            if(spi_write_txfifo(data) == 1)
            {
                tx_ok = 1;
            }
        }

        /* check rx data to prevent rx over flow */
        if(spi_read_rxfifo(&data) == 1)
        {
            if(rxbuf)
            {
                if(drop != 0)
                {
                    drop--;
                }
                else
                {
                    if(offset < spi_dev->rx_len)
                    {
                        rxbuf[offset] = data;
                        offset++;
                    }
                    else
                    {
                        BK_SPI_WPRT("0 rx over flow:%02x, %d\r\n", data, spi_dev->rx_len);
                    }
                }
            }
        }

        if(tx_ok == 1)
        {
            total_len --;
            if(total_len == 0) 
            {
                UINT32 enable = 0;
                spi_ctrl(CMD_SPI_TXINT_EN, (void *)&enable);
                
                //BK_SPI_PRT("tx fin\r\n");
                break;
            }
        }
        else
        {
            break;
        }
    }

    GLOBAL_INT_DISABLE();
    spi_dev->tx_ptr = tx_ptr;
    spi_dev->tx_len = tx_len;
    spi_dev->total_len = total_len;

    spi_dev->rx_drop = drop;
    spi_dev->rx_offset = offset;
    GLOBAL_INT_RESTORE();
    
}

static void bk_spi_tx_finish_callback(int port, void *param)
{
    if((spi_dev->total_len == 0) && ((spi_dev->flag & TX_FINISH_FLAG) == 0))
    {
        spi_dev->flag |= TX_FINISH_FLAG;
        bk_rtos_set_semaphore(&spi_dev->tx_sem);
        //BK_SPI_PRT("tx end\r\n");
    }
}

static void bk_spi_configure(UINT32 rate, UINT32 mode)
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

    /* Master */
    param = 1;
    spi_ctrl(CMD_SPI_SET_MSTEN, (void *)&param);
    param = 3;
    spi_ctrl(CMD_SPI_SET_NSSID, (void *)&param);
    param = 1;
    spi_ctrl(CMD_SPI_INIT_MSTEN, (void *)&param);

    /* set call back func */
    spi_dev_cb.callback = bk_spi_rx_callback;
    spi_dev_cb.param = NULL;
    spi_ctrl(CMD_SPI_SET_RX_CALLBACK, (void *)&spi_dev_cb);

    spi_dev_cb.callback = bk_spi_tx_needwrite_callback;
    spi_dev_cb.param = NULL;
    spi_ctrl(CMD_SPI_SET_TX_NEED_WRITE_CALLBACK, (void *)&spi_dev_cb);

    spi_dev_cb.callback = bk_spi_tx_finish_callback;
    spi_dev_cb.param = NULL;
    spi_ctrl(CMD_SPI_SET_TX_FINISH_CALLBACK, (void *)&spi_dev_cb);

    /* enable spi */
    param = 1;
    spi_ctrl(CMD_SPI_UNIT_ENABLE, (void *)&param);

    BK_SPI_PRT("[CTRL]:0x%08x \n", REG_READ(SPI_CTRL));
}

static void bk_spi_unconfigure(void)
{
    spi_ctrl(CMD_SPI_DEINIT, NULL);
}

int bk_spi_master_xfer(struct spi_message *msg)
{
    UINT32 param, total_size;

    ASSERT(spi_dev != NULL);
    ASSERT(msg != NULL);

    bk_rtos_lock_mutex(&spi_dev->mutex);
    
    total_size = msg->recv_len + msg->send_len;
    if(total_size) 
    {
        GLOBAL_INT_DECLARATION();

        /* initial spi_dev */
        GLOBAL_INT_DISABLE();
        spi_dev->tx_ptr = msg->send_buf;
        spi_dev->tx_len = msg->send_len;
        
        spi_dev->rx_ptr = msg->recv_buf;
        spi_dev->rx_len = msg->recv_len;
        spi_dev->rx_offset = 0;
        spi_dev->rx_drop = msg->send_len;

        spi_dev->total_len = total_size;
        spi_dev->flag &= ~(TX_FINISH_FLAG);
        GLOBAL_INT_RESTORE();

        /* take CS */
        param = 0x2;
        spi_ctrl(CMD_SPI_SET_NSSID, (void *)&param);

        /* enabel tx & rx interrupt */
        param = 1;
        spi_ctrl(CMD_SPI_RXINT_EN, (void *)&param);
        spi_ctrl(CMD_SPI_TXINT_EN, (void *)&param);

        //os_printf("0 %d\r\n", total_size);

        /* wait tx finish */
        bk_rtos_get_semaphore(&spi_dev->tx_sem, BEKEN_NEVER_TIMEOUT);

        //os_printf("1 %d\r\n", total_size);

        /* disable tx & rx interrupt again */
        param = 0;
        spi_ctrl(CMD_SPI_RXINT_EN, (void *)&param);
        spi_ctrl(CMD_SPI_TXINT_EN, (void *)&param);

        /* release CS */
        param = 0x3;
        spi_ctrl(CMD_SPI_SET_NSSID, (void *)&param);

        /* initial spi_dev with zero*/
        GLOBAL_INT_DISABLE();
        spi_dev->tx_ptr = NULL;
        spi_dev->tx_len = 0;
        
        spi_dev->rx_ptr = NULL;
        spi_dev->rx_len = 0;

        spi_dev->total_len = 0;
        spi_dev->flag |= TX_FINISH_FLAG;
        GLOBAL_INT_RESTORE();
    } 

    bk_rtos_unlock_mutex(&spi_dev->mutex);

    return msg->recv_len;
}

int bk_spi_master_init(UINT32 rate, UINT32 mode)
{
    OSStatus result = 0;

    if (spi_dev)
    {
        bk_spi_master_deinit();
    }

    spi_dev = os_malloc(sizeof(struct bk_spi_dev));
    if (!spi_dev)
    {
        BK_SPI_PRT("[spi]:malloc memory for spi_dev failed\n");
        result = -1;
        goto _exit;
    }
    os_memset(spi_dev, 0, sizeof(struct bk_spi_dev));


#if CFG_SUPPORT_ALIOS
    result = bk_rtos_init_semaphore(&spi_dev->tx_sem, 0);
#else
    result = bk_rtos_init_semaphore(&spi_dev->tx_sem, 1);
#endif
    if (result != kNoErr)
    {
        BK_SPI_PRT("[spi]: spi tx semp init failed\n");
        goto _exit;
    }

    result = bk_rtos_init_mutex(&spi_dev->mutex);
    if (result != kNoErr)
    {
        BK_SPI_PRT("[spi]: spi mutex init failed\n");
        goto _exit;
    }
    
    spi_dev->tx_ptr = NULL;
    spi_dev->tx_len = 0;
    spi_dev->flag |= TX_FINISH_FLAG;

    bk_spi_configure(rate, mode);
    
    return 0;

_exit:
    if(spi_dev->mutex)
        bk_rtos_deinit_mutex(&spi_dev->mutex);
    
    if(spi_dev->tx_sem)
        bk_rtos_deinit_semaphore(&spi_dev->tx_sem);
    
    if (spi_dev)
    {
        os_free(spi_dev);
        spi_dev = NULL;
    }

    return 1;
}

int bk_spi_master_deinit(void)
{
    if (spi_dev == NULL)
    {
        return 0;
    }

    if(spi_dev->mutex)
        bk_rtos_lock_mutex(&spi_dev->mutex);
    
    if(spi_dev->tx_sem)
        bk_rtos_deinit_semaphore(&spi_dev->tx_sem);

    if(spi_dev->mutex) 
    {
        bk_rtos_unlock_mutex(&spi_dev->mutex);
        bk_rtos_deinit_mutex(&spi_dev->mutex);
    }
    
    if (spi_dev)
    {
        os_free(spi_dev);
        spi_dev = NULL;
    }

    bk_spi_unconfigure();

    return 0;
}

#endif  // CFG_USE_SPI_MASTER

