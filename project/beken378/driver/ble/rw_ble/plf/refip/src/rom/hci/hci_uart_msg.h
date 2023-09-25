/**
 ****************************************************************************************
 *
 * @file hci_msg.h
 *
 * @brief Header file containing the definitions and structures corresponding to the
 * standard HCI messages: commands from Host, events from Controller.
 *
 * This file is to be included where the defines or structures are needed, but also parsed
 * in order to generate a source file with needed HCI API functions (packing and unpacking
 * command and event  packet components)
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef HCI_MSG_H_
#define HCI_MSG_H_

/**
 ****************************************************************************************
 * @addtogroup HCI_MSG HCI Pack/Unpack Functions
 * @ingroup HCI
 * @brief Host-Controller Interface functionalities.
 *
 * This module contains the @ref HOST "HOST" - @ref Controller "Controller" interface
 * functionalities: message IDs, message structures and communication handlers. \n
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>


/*
 * DEFINES
 ****************************************************************************************
 */

/**************************************************************************************
 **************                       HCI COMMANDS                     ****************
 **************************************************************************************/

/// Legacy Commands Opcodes: OGF(6b) | OCF(10b)
#define HCI_RESET_CMD_OPCODE                        0x0C03
#define HCI_RD_LOCAL_VER_INFO_CMD_OPCODE            0x1001
///Debug commands - OGF = 0x3F (6b) , OCF(10b)
#define HCI_DBG_RD_MEM_CMD_OPCODE                   0xFC01
#define HCI_DBG_WR_MEM_CMD_OPCODE                   0xFC02
#define HCI_DBG_DEL_PAR_CMD_OPCODE                  0xFC03
#define HCI_DBG_FLASH_ID_CMD_OPCODE                 0xFC05
#define HCI_DBG_FLASH_ER_CMD_OPCODE                 0xFC06
#define HCI_DBG_FLASH_WR_CMD_OPCODE                 0xFC07
#define HCI_DBG_FLASH_RD_CMD_OPCODE                 0xFC08
#define HCI_DBG_RD_PAR_CMD_OPCODE                   0xFC09
#define HCI_DBG_WR_PAR_CMD_OPCODE                   0xFC0A
#define HCI_DBG_PLF_RESET_CMD_OPCODE                0xFC11


/**************************************************************************************
 **************                        HCI EVENTS                      ****************
 **************************************************************************************/

/// Legacy Events Codes - 1 byte
#define HCI_CMD_CMPL_EVT_CODE                       0x0E

///HCI Debug events parameter lengths -
#define HCI_CCEVT_UNNKNOWN_RETPAR_LEN              1
#define HCI_CCEVT_RESET_RETPAR_LEN                 1
#define HCI_CCEVT_RD_LOCAL_VER_INFO_RETPAR_LEN     9
#define HCI_CCEVT_DBG_RD_MEM_RETPAR_LEN            0 //variable
#define HCI_CCEVT_DBG_WR_MEM_RETPAR_LEN            1
#define HCI_CCEVT_DBG_DEL_PAR_RETPAR_LEN           1
#define HCI_CCEVT_DBG_FLASH_ID_RETPAR_LEN          2
#define HCI_CCEVT_DBG_FLASH_ER_RETPAR_LEN          1
#define HCI_CCEVT_DBG_FLASH_WR_RETPAR_LEN          1
#define HCI_CCEVT_DBG_FLASH_RD_RETPAR_LEN          0 //variable
#define HCI_CCEVT_DBG_RD_PAR_RETPAR_LEN            0 //variable
#define HCI_CCEVT_DBG_WR_PAR_RETPAR_LEN            1
#define HCI_CCEVT_DBG_PLF_RESET_RETPAR_LEN         1


/*
 * FUNCTION DECLARATIONS - Lower Layers Command unpacking
 ****************************************************************************************
 */
/**
****************************************************************************************
* @brief GENERAL COMMENT FOR hci_..._cmd_unpk : HCI function for LL Command unpacking
*        after transport.
*
* @param bufptr Pointer to the buffer containing the received packet. Space is part of
*               allocated kernel message prior to reception. This kernel_msg will be updated
*               with extracted data.
*****************************************************************************************
*/
//debug commands unpacking functions
void hci_dbg_rd_mem_cmd_unpk(uint8_t * bufptr);
void hci_dbg_wr_mem_cmd_unpk(uint8_t * bufptr);
void hci_dbg_del_param_cmd_unpk(uint8_t *bufptr);
void hci_dbg_flash_identify_cmd_unpk(uint8_t * bufptr);
void hci_dbg_flash_erase_cmd_unpk( uint8_t * bufptr);
void hci_dbg_flash_write_cmd_unpk( uint8_t * bufptr);
void hci_dbg_flash_read_cmd_unpk(uint8_t * bufptr);
void hci_dbg_rd_param_cmd_unpk(uint8_t *bufptr);
void hci_dbg_wr_param_cmd_unpk(uint8_t *bufptr);
void hci_dbg_plf_reset_cmd_unpk(uint8_t * bufptr);

void hci_rd_local_ver_info_ccevt_pk(void);
void hci_ccevt_pk(uint16_t opcode, uint8_t status, uint8_t parlen);

/// @} HCI_MSG

#endif // HCI_MSG_H_
