#ifndef __DRV_SPI_FLASH_H__
#define __DRV_SPI_FLASH_H__

typedef struct bk_spi_flash_erase_st {
    rt_uint32_t addr;
    rt_uint32_t size;
}BK_SPIFLASH_ERASE_ST, *BK_SPIFLASH_ERASE_PTR;

#define BK_SPI_FLASH_ERASE_CMD          (0x01)
#define BK_SPI_FLASH_PROTECT_CMD        (0x02)
#define BK_SPI_FLASH_UNPROTECT_CMD      (0x03)


#endif
