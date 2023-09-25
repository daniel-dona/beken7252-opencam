#ifndef _BK_SPI_PSRAM_H_
#define _BK_SPI_PSRAM_H_

int32_t spi_psram_init(void);
int32_t spi_psram_burst_set(uint32_t burst_size);
int32_t spi_psram_read_id(uint8_t id[12]);
uint32_t spi_psram_read(uint32_t addr, uint8_t* buffer, uint32_t size);
uint32_t spi_psram_write(uint32_t addr, uint8_t* buffer, uint32_t size);

#endif //_BK_SPI_PSRAM_H_