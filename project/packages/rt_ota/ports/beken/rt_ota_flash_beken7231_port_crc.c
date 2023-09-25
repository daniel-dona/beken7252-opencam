/*
 * File      : rt_ota_flash_beken7231_port_crc.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 * 2018-02-03     Murphy       Adapted beken7231 board feature
 * 2018-02-06     Murphy       Adapted read for crc partition, add flash control
 */

#include <stdint.h>
#include <rt_ota.h>
#include <rt_ota_flash_dev.h>

#ifdef RT_OTA_FLASH_PORT_DRIVER_BEKEN_CRC
#include "flash.h"
#include <flash_pub.h>
#include <string.h>

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

static int read(uint32_t offset, uint8_t *buf, size_t size)
{
    /* Addr must be 32 bytes aligned */
    assert(offset % 32 == 0); 

    uint32_t addr, index = 0;
    uint32_t crc_offset = 0;

    uint32_t len = 32;

    addr = beken_onchip_flash_crc.addr + offset/32*34;

    while(size)
    {
        if (size < 32)
        {
            len = size;
            size = 0;
        }
        else
        {
            len = 32;
            size -= 32;
        }

        flash_read((unsigned char *)&buf[index], (unsigned long)len, (unsigned long)(addr + crc_offset));
        index += len;
        crc_offset += 34;
    }

    return index;
}

static int write(uint32_t offset, const uint8_t *buf, size_t size)
{
    char crc16_buf[34] = {0xff};

    uint32_t addr, index = 0;
    uint32_t crc_offset = 0;
    uint16_t crc_value = 0;
    uint32_t len = 32;

    /* Addr must be 32 bytes aligned */
    assert(offset % 32 == 0);

    addr = beken_onchip_flash_crc.addr + offset/32*34;

    int param = 1;
    flash_ctrl(CMD_FLASH_SET_WSR, &param);
    flash_ctrl(CMD_FLASH_WRITE_ENABLE, NULL);
    
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

    flash_ctrl(CMD_FLASH_WRITE_DISABLE, NULL);

    return index;
}

/* 
 * size param: The flash size of you want to erase in bytes.
 * return: Returns the size of the actual erase.
 */
static int erase(uint32_t offset, size_t size)
{
    unsigned int _size = size/32*34;
    uint32_t addr;

    offset = offset/32*34;

    /* start erase */
    int param = 1;
    flash_ctrl(CMD_FLASH_SET_WSR, &param);     

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

/* beken_onchip_crc dev */
const struct rt_ota_flash_dev beken_onchip_flash_crc = {RT_OTA_BL_FLASH_DEV_CRC_NAME, RT_OTA_FLASH_START_ADDR, RT_OTA_FLASH_SIZE, {read, write, erase}};
#endif /* RT_OTA_FLASH_PORT_DRIVER_BEKEN_CRC */
