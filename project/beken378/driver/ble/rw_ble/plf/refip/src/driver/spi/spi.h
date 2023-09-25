/**
 ****************************************************************************************
 *
 * @file spi.h
 *
 * @brief SPI Driver for SPI operation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _SPI_H_
#define _SPI_H_

/**
 ****************************************************************************************
 * @defgroup SPI SPI
 * @ingroup DRIVERS
 * @brief SPI driver
 *
 * @{
 *
 ****************************************************************************************
 */

/* 
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>          // standard boolean definitions
#include <stdint.h>           // standard integer functions

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

#define SPI_ENABLE					0
 
#define NUMBER_ROUND_UP(a,b)        ((a) / (b) + (((a) % (b)) ? 1 : 0))

#define SPI_DEFAULT_CLK             16000000

/// 1MHZ baudrate supported by this SPI (in bps)
#define SPI_BAUD_1MHZ         	1000000

/// Default baudrate supported by this SPI (in bps)
#define SPI_BAUD_4MHZ         	4000000

/// Default baudrate supported by this SPI (in bps)
#define SPI_DEFAULT_BAUD      SPI_BAUD_1MHZ


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void spi_init(uint32_t baud);

uint8_t spi_is_busy(void);

void spi_write_read(uint8_t *wbuf, uint32_t w_size, uint8_t *rbuf, uint32_t r_size,uint32_t r_off);



/// @} UART
#endif /* _UART_H_ */
