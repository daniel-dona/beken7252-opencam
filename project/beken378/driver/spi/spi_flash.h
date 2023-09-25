#ifndef _BK_SPI_FLASH_H_
#define _BK_SPI_FLASH_H_

int spi_flash_init(void);
void spi_flash_deinit(void);
UINT32 spi_flash_read_id(void);
int spi_flash_read(UINT32 addr, UINT32 size, UINT8 *dst);
int spi_flash_write(UINT32 addr, UINT32 size, UINT8 *src);
int spi_flash_erase(UINT32 addr, UINT32 size);
void spi_flash_protect(void);
void spi_flash_unprotect(void);

#endif //_BK_SPI_FLASH_H_