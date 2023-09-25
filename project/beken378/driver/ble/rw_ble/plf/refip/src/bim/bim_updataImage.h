
#include <stdint.h>



#define BIM_FSIZE_16K

#ifdef BIM_FSIZE_16K

#define FLASH_SEC_SIZE										(0X1000)
#define FLASH_HALF_BLOCK_SIZE							(0X8000)
#define FLASH_ONE_BLOCK_SIZE							(0X10000)


#define SEC_MAX_FSIZE_4K									(240) //240KB 
#define SEC_MAX_FSIZE_BLOCK								(0X3C00) //240 * 1024 f000
#define SEC_MAX_FSIZE_BYTE								(0X3C000) //240 * 1024

#define SEC_IMAGE_RUN_CADDR			         (0x3C60) 
#define SEC_IMAGE_RUN_FADDR			         (0x4026) //( 0x3C60 * 0X22 / 0X20)

#define SEC_IMAGE_OAD_HEADER_FADDR			 (0x4016) //(0x4026 - 0X10)

#define SEC_IMAGE_ALLOC_START_FADDR			 (0x4000) //(16KB)
#define SEC_IMAGE_ALLOC_END_FADDR			 (0x40000) //(256KB)

#define SEC_BACKUP_OAD_HEADER_FADDR			 (0x44000) //272kb * 1024 = 270336
#define SEC_BACKUP_OAD_IMAGE_FADDR			 (0x44010) //272kb * 1024 + 0X10 = 270336
#define SEC_BACKUP_ALLOC_START_FADDR		 (0x44000) //(272KB)
#define SEC_BACKUP_ALLOC_END_FADDR			 (0x80000) //(512KB)

#else



#define FLASH_SEC_SIZE									(0X1000)
#define SEC_MAX_FSIZE_4K								(248) //248KB 
#define SEC_MAX_FSIZE_BLOCK								0xffff //(0X3E00) //248 * 1024
#define SEC_MAX_FSIZE_BYTE								(0X3E000) //248 * 1024

#define SEC_IMAGE_RUN_CADDR			         (0x1E40) 
#define SEC_IMAGE_RUN_FADDR			         (0x2024) //( 0x1E40 * 0X22 / 0X20)

#define SEC_IMAGE_OAD_HEADER_FADDR			 (0x2014) //(0x2024 - 0X10)

#define SEC_IMAGE_ALLOC_START_FADDR			 (0x2000) //(8KB)
#define SEC_IMAGE_ALLOC_END_FADDR			 (0x40000) //(256KB)


#define SEC_BACKUP_OAD_HEADER_FADDR			 (0x42000) //264kb * 1024 = 270336
#define SEC_BACKUP_ALLOC_START_FADDR		 (0x42000) //(264KB)
#define SEC_BACKUP_ALLOC_END_FADDR			 (0x80000) //(512KB)

#endif//


/*********************************************************************
 * TYPEDEFS
 */
// The Image Header will not be encrypted, but it will be included in a Signature.
typedef struct
{
    // Secure OAD uses the Signature for image validation instead of calculating a CRC, but the use
    // of CRC==CRC-Shadow for quick boot-up determination of a validated image is still used.
    uint32_t crc;       // CRC must not be 0x0000 or 0xFFFF.
    // User-defined Image Version Number - default logic uses simple a '!=' comparison to start an OAD.
  	uint16_t ver; 
	
	uint16_t len;        // Image length in 4-byte blocks (i.e. HAL_FLASH_WORD_SIZE blocks).
	
	uint32_t  uid;       // User-defined Image Identification bytes.
	uint8_t  crc_status;     // cur image crc status
	uint8_t  sec_status;     // cur image sec status
	uint8_t  res[2];     // Reserved space for future use.
} img_hdr_t;




#define  BLOCK_SIZE        	0X10

#define CRC_UNCHECK 		0xFF
#define CRC_CHECK_OK 		0xAA
#define CRC_CHECK_FAIL 		0x55

#define SECT_UNKNOW			0xFF
#define SECT_NORMAL			0xAA
#define SECT_ABNORMAL 		0x55		

enum
{
    SSTATUS_SECT_NORMAL = 0,           
    SSTATUS_SECT_ERASED,
    SSTATUS_SECT_ABNORMAL,
	SSTATUS_SECT_UNKOWN ,
};


void udi_erase_image_sec(void);

void udi_erase_backup_sec(void);

void udi_updata_backup_to_image_sec(void);

void udi_updata_image_to_backup_sec(void);

void test_crc(void);

void test_erase_time(void);

uint8_t udi_select_sec(void);

