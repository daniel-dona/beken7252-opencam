#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>
#include "sys_config.h"

#define SPI_TX_BUF_LEN     (512)
#define SPI_RX_BUF_LEN     (512)

#ifdef BEKEN_USING_SPI_HSLAVE

#if (CFG_USE_HSLAVE_SPI == 0)
#error "spi_hs_test need 'CFG_USE_HSLAVE_SPI' and 'CFG_USE_SPI_MST_PSRAM'"
#endif

int spi_hs_test(int argc, char** argv)
{
	struct rt_device *spi_hs;
    
    spi_hs = (struct rt_device *)rt_device_find("spi_hs");
    if (spi_hs == RT_NULL)
    {
        rt_kprintf("spi device %s not found!\r\n", "spi_hs");
        return -RT_ENOSYS;
    }

    /*open device*/
    if (rt_device_open(spi_hs, 0) != RT_EOK)
    {
        return 0;
    }

    if(strcmp(argv[1], "tx") == 0)
    {
        rt_uint8_t *buf;
        int tx_len;

        if(argc < 3)
            tx_len = SPI_TX_BUF_LEN;
        else
            tx_len = atoi(argv[2]);

        rt_kprintf("spi hs tx_len:%d\n", tx_len);

        buf = rt_malloc(tx_len * sizeof(rt_uint8_t));

        if(buf)
        {
            rt_memset(buf, 0, tx_len);
            for(int i=0; i<tx_len; i++) 
            {
                buf[i] = i & 0xff;
            }
            
            rt_device_write(spi_hs, 0, (const void *)buf, tx_len);

            for(int i=0; i<tx_len; i++) 
            {
                rt_kprintf("%02x,", buf[i]);
                if((i+1)%32 == 0)
                    rt_kprintf("\r\n");
            }
            rt_kprintf("\r\n"); 

            rt_free(buf);
        }
    }
    else if(strcmp(argv[1], "rx") == 0)
    {
        rt_uint8_t *buf;
        int rx_len;

        if(argc < 3)
            rx_len = SPI_RX_BUF_LEN;
        else
            rx_len = atoi(argv[2]);

        rt_kprintf("spi hs rx_len:%d\n", rx_len);

        buf = rt_malloc(rx_len * sizeof(rt_uint8_t));

        if(buf)
        {
            rt_memset(buf, 0, rx_len);
            
            rx_len = rt_device_read(spi_hs, 0, buf, rx_len);
            rt_kprintf("rx ret:%d\r\n", rx_len);
            
            for(int i=0; i<rx_len; i++) 
            {
                rt_kprintf("%02x,", buf[i]);
                if((i+1)%32 == 0)
                    rt_kprintf("\r\n");
            }
            rt_kprintf("\r\n"); 

            rt_free(buf);
        }
    }
    else 
    {
        rt_kprintf("spi_hs_test tx/rx len\r\n"); 
    }

    rt_device_close(spi_hs);
}

MSH_CMD_EXPORT(spi_hs_test, spi_hs_test);

#endif // BEKEN_USING_SPI_HSLAVE
