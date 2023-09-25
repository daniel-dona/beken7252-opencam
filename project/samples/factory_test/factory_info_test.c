#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <rtthread.h>
#include <finsh.h>
#include "drv_flash.h"
#include "BkDriverFlash.h"

#if defined RT_USING_FACTORYTEST
#define DBG_SECTION_NAME  "[FACTORYINFO]"
#define DBG_LEVEL         DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#define FACTORY_INFO_LEN     (4096)
#define DATA_OFFSET          (64 * 1024)
#define OFFSET               (1024)

static rt_uint8_t *buf = RT_NULL;
static uint8_t *dst_ptr = RT_NULL;
static void dump_hex(const void *ptr1, rt_size_t buflen);

static uint8_t HexToChar(uint8_t bChar)
{
	if((bChar>=0x30)&&(bChar<=0x39))
	{
		bChar -= 0x30;
	}
	else if((bChar>=0x41)&&(bChar<=0x46)) // Capital
	{
		bChar -= 0x37;
	}
	else if((bChar>=0x61)&&(bChar<=0x66)) //littlecase
	{
		bChar -= 0x57;
	}
	else 
	{
		bChar = 0xff;
	}
	return bChar;
}

static uint8_t *datatrans(uint8_t *data,size_t *data_len)
{
    
    uint8_t temp;
    int str_len = 0;
    uint8_t i = 0;
    uint8_t count = 0;
    uint8_t j = 0;
    str_len = strlen(data);
    dst_ptr = rt_malloc(str_len);
    if( dst_ptr == RT_NULL)
    {
        rt_kprintf("malloc dst_ptr failed\r\n");
        return 0;
    }

    while (*(data+i) != '\0')
    {
        if((*(data + i) == '\\') && (*(data + i+1) == 'x'))
        {
            temp = 0xf0 | HexToChar(*(data+i+2));
            temp = temp << 4;
            temp = temp | HexToChar(*(data+i+3));
            //rt_kprintf("0x%02x",temp);
            *(dst_ptr+count) = temp;
            i = i+4;
            count++;
        }
        else
        {
            *(dst_ptr+count) = *(data + i) ;
             count++;
             i = i+1;
        }
        
    }
    *data_len = count;
    return dst_ptr;
}

/*
function: factoryinfo_load
to read data from flash
 */

static void factoryinfo_load(void)
{
    if(buf == RT_NULL)
    {
        buf = rt_malloc(FACTORY_INFO_LEN);

        if (buf == RT_NULL)
        {
            rt_kprintf("No memory for buf...\r\n");
            return;
        }
    }
    
    beken_flash_read(DATA_OFFSET, buf, FACTORY_INFO_LEN);
}

static void factoryinfo_read(rt_uint32_t offset,rt_uint32_t size)
{
   rt_uint8_t *read_buf = RT_NULL;
   rt_kprintf("read out ...\r\n");
   if (read_buf == RT_NULL)
   {
       read_buf = rt_malloc(size);

       if (read_buf == RT_NULL)
       {
           rt_kprintf("No memory for read_buf \r\n");
           return;
       }
   }
   beken_flash_read(DATA_OFFSET+OFFSET+offset, read_buf, size);
   dump_hex(read_buf, size);
   if( read_buf != RT_NULL)
   {
       free(read_buf);
   }
}
/*
function :factoryinfo_write
To copy data to buf
 */
static void factoryinfo_write(rt_uint32_t offset,rt_uint8_t *data, rt_uint32_t size)
{
    if(buf == RT_NULL)
    {
        return;
    }

    if(size > (FACTORY_INFO_LEN - OFFSET))
    {
        rt_kprintf("size > 3072 can't write in flash....\r\n");
        return;
    }
    memcpy(buf+OFFSET+offset,data,size);
}

/*
function: factoryinfo_prog
To erase flash data and write data to flash.
 */
static void factoryinfo_prog(void)
{
     if(buf == RT_NULL)
    {
        rt_kprintf("Buf is empty make sure have load buf before...\r\n");
        return;
    }
    bk_flash_enable_security(FLASH_PROTECT_NONE); // half or custom
    beken_flash_erase(DATA_OFFSET);
    beken_flash_write(DATA_OFFSET, buf,FACTORY_INFO_LEN);
    bk_flash_enable_security(FLASH_UNPROTECT_LAST_BLOCK);  // last or custom
    
}

#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void dump_hex(const void *ptr1, rt_size_t buflen)
{
    const rt_uint8_t *ptr = (const rt_uint8_t *)ptr1;
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;

    for (i = 0; i < buflen; i += 16)
    {
        rt_kprintf("%08X: ", i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%02X ", buf[i + j]);
            else
                rt_kprintf("   ");
        rt_kprintf(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
        rt_kprintf("\n");
    }
}


static int factoryinfo(int argc, char* argv[])
{
    rt_err_t result = RT_EOK;
    rt_uint8_t *data = "1223456";
    rt_uint8_t offset = 0;
    size_t data_len = 0;

    if (argc < 2)
    {
        rt_kprintf("usage: %s [option] [target] ...\n\n", argv[0]); 
        rt_kprintf("%s options:\n", argv[0]); 
        rt_kprintf("  load,                 \tfactoryinfo load must before write start.\n"); 
        rt_kprintf("  read offset size,     \tLike:factoryinfo read 0 100.\n"); 
        rt_kprintf("  write offset data,    \tLike:factoryinfo write 0 Hello,world.\n");
        rt_kprintf("  prog,                 \tfactoryinfo prog must after write end.\n"); 
        return 0;
    }

    if (strcmp(argv[1],"load") == 0)
    {
        rt_kprintf("load data to buf...\r\n");
        factoryinfo_load();
    }

    if (strcmp(argv[1],"read") == 0)
    {
        offset = atoi(argv[2]);
        int size = atoi(argv[3]);
        factoryinfo_read(offset, size);
        result = 1;
    }

    if(strcmp(argv[1],"write") == 0)
    {
        offset = atoi(argv[2]);
        data = datatrans(argv[3],&data_len);
        factoryinfo_write(offset,data, data_len);
        result = 2;
        if (dst_ptr != RT_NULL)
        {
            rt_free(dst_ptr);
        }
        
    }

    if (strcmp(argv[1],"prog") == 0)
    {
        rt_kprintf("write in...\r\n");
        factoryinfo_prog();
        if (buf != RT_NULL)
        {
            rt_free(buf);
            buf = RT_NULL;
            rt_kprintf("free buf \r\n");
        }
    }

    return 0;
}

MSH_CMD_EXPORT(factoryinfo, factory info write test);

#endif



