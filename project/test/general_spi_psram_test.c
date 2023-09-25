#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>
#include "sys_config.h"

#ifdef BEKEN_USING_SPI_PSRAM

#if ((CFG_USE_SPI_MASTER == 0) || (CFG_USE_SPI_MST_PSRAM == 0))
#error "test gspi psram need 'CFG_USE_SPI_MASTER' and 'CFG_USE_SPI_MST_PSRAM'"
#endif

void spi_psram_test(int argc, char** argv)
{
    struct rt_device *psram;

    /*find device*/
    psram = rt_device_find("spi_psram");
    if (psram == NULL)
    {
        rt_kprintf("psram not found \n");
        return;
    }

    /*initial device*/
    if (rt_device_init(psram) != RT_EOK)
    {
        return;
    }

    /*open device*/
    if (rt_device_open(psram, 0) != RT_EOK)
    {
        return;
    }

	uint8_t buffer[4096];
	int i;
	rt_kprintf("[PSRAM]: SPRAM test begin\n");

    /*initial write data*/
	for(i = 0; i < sizeof(buffer); i++)
	{
		buffer[i] = (uint8_t)i;
	}

    /*write data to psram*/
    rt_device_write(psram, 0, buffer, sizeof(buffer));

    /*clear buffer*/
	rt_memset(buffer, 0, sizeof(buffer));

    /*read data from psram*/
    rt_device_read(psram, 0, buffer, sizeof(buffer));

    /*check data, print those different data*/
	for(i = 0; i < sizeof(buffer); i++)
	{
		if(buffer[i] != (uint8_t)i)
		{
			rt_kprintf("[%02d]: %02x - %02x\n", i, (uint8_t)i, buffer[i]);
		}	
	}
	rt_kprintf("[PSRAM]: SPRAM test end\n");

    rt_device_close(psram);
}
MSH_CMD_EXPORT(spi_psram_test, spi_psram_test);

#endif  // BEKEN_USING_SPI_PSRAM
