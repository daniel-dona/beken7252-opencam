#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>
#include "sys_config.h"

#ifdef BEKEN_USING_SPI_FLASH

#if ((CFG_USE_SPI_MASTER == 0) || (CFG_USE_SPI_MST_FLASH == 0))
#error "test gspi psram need 'CFG_USE_SPI_MASTER' and 'CFG_USE_SPI_MST_FLASH'"
#endif

#include "drv_spi_flash.h"

#define FTEST_BUF_SIZE      1024
#define FTEST_BASE          0x40
#define FTEST_ADDR          0x100000

void gspi_flash_test(int argc, char** argv)
{
    struct rt_device *flash;

    /*find device*/
    flash = rt_device_find("spi_flash");
    if (flash == NULL)
    {
        rt_kprintf("psram not found \n");
        return;
    }
    /*initial device*/
    if (rt_device_init(flash) != RT_EOK)
    {
        return;
    }
    /*open device*/
    if (rt_device_open(flash, 0) != RT_EOK)
    {
        return;
    }

    uint8_t buffer[FTEST_BUF_SIZE], *ptr;
    int i;

    rt_kprintf("[SPIFLASH]: SPIFLASH test begin\n");
    rt_memset(buffer, 0, FTEST_BUF_SIZE);

    /*read device*/
    rt_device_read(flash, FTEST_ADDR, buffer, FTEST_BUF_SIZE);

    /*show read data*/
    ptr = buffer;
    rt_kprintf("flash data:%x\r\n", FTEST_ADDR);
    for(i=0; i<FTEST_BUF_SIZE; i++)
    {
        rt_kprintf("0x%02x,", ptr[i]);
        if((i+1)%16 == 0)
            rt_kprintf("\r\n");
    }
    rt_kprintf("\r\n");

    /*set write data*/
    ptr = (uint8_t *)FTEST_BASE;
    rt_kprintf("base data:%08x\r\n", ptr);
    for(i=0; i<FTEST_BUF_SIZE; i++)
    {
        rt_kprintf("0x%02x,", ptr[i]);
        buffer[i] = ptr[i];
        if((i+1)%16 == 0)
            rt_kprintf("\r\n");
    }
    rt_kprintf("\r\n");      

    /*disable flash's protect bit */
    rt_device_control(flash, BK_SPI_FLASH_UNPROTECT_CMD, NULL);

    /*write data */
    rt_device_write(flash, FTEST_ADDR, buffer, FTEST_BUF_SIZE);
    rt_kprintf("write fin\r\n");

    /*clear buffer */
    rt_memset(buffer, 0, FTEST_BUF_SIZE);

    /*read data */
    rt_device_read(flash, FTEST_ADDR, buffer, FTEST_BUF_SIZE);
    rt_kprintf("read fin\r\n");

    /*show read data */
    ptr = buffer;
    rt_kprintf("flash data:%x\r\n", FTEST_ADDR);
    for(i=0; i<FTEST_BUF_SIZE; i++)
    {
        rt_kprintf("0x%02x,", ptr[i]);
        if((i+1)%16 == 0)
            rt_kprintf("\r\n");
    }
    rt_kprintf("\r\n");

    /*earse flash */
    rt_kprintf("earase\r\n");      
    BK_SPIFLASH_ERASE_ST erase_st;
    erase_st.addr = FTEST_ADDR;
    erase_st.size = 4 * 1024;
    rt_device_control(flash, BK_SPI_FLASH_ERASE_CMD, &erase_st);
    rt_kprintf("[SPIFLASH]: SPIFLASH test end\n");

    /*enable flash's protect bit */
    rt_device_control(flash, BK_SPI_FLASH_PROTECT_CMD, NULL);

    /*close device */
    rt_device_close(flash);
}

MSH_CMD_EXPORT(gspi_flash_test, gspi_flash_test);

#endif // BEKEN_USING_SPI_FLASH
