#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>
#include "sys_config.h"

#define SPI_BAUDRATE       (10 * 1000 * 1000)
#define SPI_TX_BUF_LEN     (32)
#define SPI_RX_BUF_LEN     (32)

#if ((CFG_USE_SPI_MASTER) &&(CFG_USE_SPI_SLAVE))

int gspi_test(int argc, char** argv)
{
	struct rt_spi_device *spi_device;
    struct rt_spi_configuration cfg;
    
    spi_device = (struct rt_spi_device *)rt_device_find("gspi");
    if (spi_device == RT_NULL)
    {
        rt_kprintf("spi device %s not found!\r\n", "gspi");
        return -RT_ENOSYS;
    }
    
    cfg.data_width = 8;
    
    if(strcmp(argv[1], "master") == 0)
    {
        cfg.mode = RT_SPI_MODE_0 | RT_SPI_MSB | RT_SPI_MASTER; 
    }
    else if(strcmp(argv[1], "slave") == 0)
    {
        cfg.mode = RT_SPI_MODE_0 | RT_SPI_MSB | RT_SPI_SLAVE;
    }
    else
    {
        rt_kprintf("gspi_test master/slave   tx/rx  rate  len\r\n"); 
        return -RT_ENOSYS;
    }
    
    /* SPI Interface with Clock Speeds Up to 30 MHz */
    if(argc == 5)
        cfg.max_hz = atoi(argv[3]);
    else
        cfg.max_hz = SPI_BAUDRATE;
    
    rt_kprintf("cfg:%d, 0x%02x, %d\r\n", cfg.data_width, cfg.mode, cfg.max_hz);
    
    rt_spi_configure(spi_device, &cfg);

    if(strcmp(argv[2], "tx") == 0)
    {
        rt_uint8_t *buf;
        int tx_len;

        if(argc < 4)
            tx_len = SPI_TX_BUF_LEN;
        else
            tx_len = atoi(argv[4]);

        rt_kprintf("spi init tx_len:%d\n", tx_len);

        buf = rt_malloc(tx_len * sizeof(rt_uint8_t));

        if(buf)
        {
            rt_memset(buf, 0, tx_len);
            for(int i=0; i<tx_len; i++) 
            {
                buf[i] = i & 0xff;
            }
            
            rt_spi_send(spi_device, buf, tx_len);

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
    else if(strcmp(argv[2], "rx") == 0)
    {
        rt_uint8_t *buf;
        int rx_len;

        if(argc < 4)
            rx_len = SPI_RX_BUF_LEN;
        else
            rx_len = atoi(argv[4]);

        rt_kprintf("spi init rx_len:%d\n", rx_len);

        buf = rt_malloc(rx_len * sizeof(rt_uint8_t));

        if(buf)
        {
            rt_memset(buf, 0, rx_len);
            
            rx_len = rt_spi_recv(spi_device, buf, rx_len);
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
        rt_kprintf("gspi_test master/slave   tx/rx  rate  len\r\n"); 
    }
}

MSH_CMD_EXPORT(gspi_test, gspi_test);
#endif
