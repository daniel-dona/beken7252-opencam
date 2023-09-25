


typedef unsigned char  BYTE;
typedef signed   long  int32;      //  有符号32位整型变量
typedef signed   short int16;      //  有符号16位整型变量
typedef signed   char  int8;       //  有符号8位整型变量 
typedef unsigned long  uint32;     //  无符号32位整型变量
typedef unsigned short uint16;     //  无符号16位整型变量
typedef unsigned char  uint8;      //  无符号8位整型变量 


/**
 ****************************************************************************************
 *
 * @file flash.h
 *
 * @brief Flash driver interface
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>               // standard integer functions
#include "BK3435_reg.h"
//#include "icu.h"
/**
 ****************************************************************************************
 * @addtogroup FLASH
 * @ingroup DRIVERS
 *
 * @brief Flash memory driver
 *
 * @{
 ****************************************************************************************
 */
 
/*
 * DEFINES
 ****************************************************************************************
 */
#if  (MCU_DEFAULT_CLK  == MCU_CLK_16M)
#define FLASH_CLK_16M
#else
#define FLASH_CLK_64M
#endif

///Flash type code used to select the correct erasing and programming algorithm
#define FLASH_TYPE_UNKNOWN             0
#define FLASH_SPACE_TYPE_MAIN		   0x3435
#define FLASH_TYPE_EMBEN_BK3435        FLASH_SPACE_TYPE_MAIN


///Base address of Flash on system bus
#define FLASH_MAIN_BASE_ADDR         0x00000000



#define FLASH_MAIN_SIZE              0x00040000

/// Common for EMBED BEKEN FLASH Family

#define FLASH_SECT_SIZE2         0x00001000  //  4 KByte
#define FLASH_BOUNDARY256B       0x00000100  // 256 Byte

#define FLASH_PAGE_SIZE                                    (256)
#define FLASH_PAGE_MASK                                   (FLASH_PAGE_SIZE - 1)
#define FLASH_ERASE_SECTOR_SIZE_RSL_BIT_CNT               (12)
#define FLASH_ERASE_SECTOR_SIZE                           (4096)
#define FLASH_ERASE_SECTOR_SIZE_MASK                      (FLASH_ERASE_SECTOR_SIZE - 1)
#define UPDATE_CHUNK_SIZE                                 (32)

#define MAX(x, y)                  (((x) > (y)) ? (x) : (y))
#define MIN(x, y)                  (((x) < (y)) ? (x) : (y))
#define max(x, y)                  (((x) > (y)) ? (x) : (y))
#define min(x, y)                  (((x) < (y)) ? (x) : (y))

/// flash operation command type(decimal)
typedef enum {
	FLASH_OPCODE_WREN    = 1,
	FLASH_OPCODE_WRDI    = 2,
	FLASH_OPCODE_RDSR    = 3,
	FLASH_OPCODE_WRSR    = 4,
	FLASH_OPCODE_READ    = 5,
	FLASH_OPCODE_RDSR2   = 6,
	FLASH_OPCODE_WRSR2   = 7,
	FLASH_OPCODE_PP      = 12,
	FLASH_OPCODE_SE      = 13,
	FLASH_OPCODE_BE1     = 14,
	FLASH_OPCODE_BE2     = 15,
	FLASH_OPCODE_CE      = 16,
	FLASH_OPCODE_DP      = 17,
	FLASH_OPCODE_RFDP    = 18,
	FLASH_OPCODE_RDID    = 20,
	FLASH_OPCODE_HPM     = 21,
	FLASH_OPCODE_CRMR    = 22,
	FLASH_OPCODE_CRMR2   = 23,
} FLASH_OPCODE;



enum
{
	ICU_CLK_16M,
	ICU_CLK_64M,

};
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize flash driver.
 ****************************************************************************************
 */

uint32 fflash_get_id(void);
void fflash_rd_data(uint8_t *buffer, uint32_t address, uint32_t len);
void fflash_wr_data (uint8_t *buffer, uint32_t address, uint32_t len);
uint32_t udi_erase_section(uint32_t address, uint32_t len);
void fflash_erase_one_sector(uint32_t address);
void fflash_erase_half_block(uint32_t address);
void fflash_erase_one_block(uint32_t address);

void flash_wr_protect_none( void);

void fflash_wr_protect_16k( void );
void fflash_wp_ALL( void );
void flash_clk_conf(uint8_t clk_sel,uint8_t clk_src,uint8_t div);


void udi_wdt_enable(uint16_t wdt_cnt);

void udi_init_bim_env(uint8_t clk);
void flash_rw_test(void);

/// @} FLASH

#endif // FLASH_H_
