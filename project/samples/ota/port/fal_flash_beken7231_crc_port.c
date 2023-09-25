/*
 * File      : fal_flash_stm32f2_port.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include <fal.h>

#ifdef RT_FLASH_PORT_DRIVER_BEKEN_CRC
#include "flash.h"
#include <flash_pub.h>
#include <string.h>
#include <stdint.h>

/*
 * CRC16 checksum
 * MSB first; Polynomial: 8005; Initial value: 0xFFFF
 * */
static unsigned short CRC16_CCITT_FALSE(unsigned char *puchMsg, unsigned int usDataLen)  
{  
  unsigned short wCRCin = 0xFFFF;  
  unsigned short wCPoly = 0x8005;  
  unsigned char wChar = 0;  
    
  while (usDataLen--)     
  {  
        wChar = *(puchMsg++);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
  }  
  return (wCRCin) ;  
}

static int init(void)
{
    /* do nothing now */
}

static int read(long offset, uint8_t *buf, size_t size)
{
    uint32_t pos = offset % 32;
    uint32_t stage0_offset = 0;
    uint32_t stage0_len = 0;
    uint32_t stage1_offset = 1;
    uint32_t stage1_len = 0;
    uint32_t index = 0;
    uint32_t len = 0;
    
    if(pos > 0)//have stage0
    {
        stage0_offset = beken_onchip_flash_crc.addr + offset / 32 * 34 + pos;
        stage0_len = size > 32 ? 32 - pos : size;
        stage1_offset = stage0_offset + stage0_len + 2;
        stage1_len = size - stage0_len;
    }
    else
    {
        stage1_offset = beken_onchip_flash_crc.addr + offset / 32 * 34;
        stage1_len = size;
    }
    
    if(stage0_len > 0)
    {
        flash_read(&buf[index], (unsigned long)stage0_len, (unsigned long)stage0_offset);
        index += stage0_len;
    }
    
    if(stage1_len > 0)
    {
        while(stage1_len > 0)
        {
            len = stage1_len > 32 ? 32: stage1_len;
            flash_read(&buf[index], (unsigned long)len, (unsigned long)stage1_offset);
            stage1_offset += 34;
            index += len;
            stage1_len -= len;
        }
    }
    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    char crc16_buf[34] = {0xff};

    uint32_t addr, index = 0;
    uint32_t crc_offset = 0;
    uint16_t crc_value = 0;
    uint32_t len = 32;

    /* Addr must be 32 bytes aligned */
    assert(offset % 32 == 0);

    addr = beken_onchip_flash_crc.addr + offset/32*34;
    
    while (size)
    {
        memset(crc16_buf, 0xff, sizeof(crc16_buf));
        
        if (size < 32)
        {
            len = size;
            size = 0;
        }
        else{
            len = 32;
            size -= 32;        
        }

        memcpy(crc16_buf, &buf[index], len);
        index += len;

        crc_value = CRC16_CCITT_FALSE(crc16_buf, 32);
        crc16_buf[32] = (unsigned char)(crc_value >> 8);
        crc16_buf[33] = (unsigned char)(crc_value & 0x00FF);     
        
        flash_write((unsigned char *)(&crc16_buf[0]), (unsigned long)34, addr + crc_offset);

        crc_offset += 34;
    }

    return index;
}

static int erase(long offset, size_t size)
{
    unsigned int _size = size/32*34;
    uint32_t addr;

    offset = offset/32*34;

    /* Calculate the start address of the flash sector(4kbytes) */
    offset = offset & 0x00FFF000;
    addr = beken_onchip_flash_crc.addr + offset;

    do
    {
        flash_ctrl(CMD_FLASH_ERASE_SECTOR, &addr);

        addr += 4096;

        if (_size < 4096)
        {
            _size = 0;
        }        
        else
        {
            _size -= 4096;
        }        
    } while (_size);

    return addr - (beken_onchip_flash_crc.addr + offset); // return true erase size
}

/* beken_onchip dev */
const struct fal_flash_dev beken_onchip_flash_crc = 
{
    RT_BK_FLASH_DEV_CRC_NAME, 
    RT_FLASH_BEKEN_START_ADDR, 
    RT_FLASH_BEKEN_SIZE, 
    RT_FLASH_BEKEN_SECTOR_SIZE,
    {init, read, write, erase} 
};
#endif /* RT_FLASH_PORT_DRIVER_BEKEN_CRC */