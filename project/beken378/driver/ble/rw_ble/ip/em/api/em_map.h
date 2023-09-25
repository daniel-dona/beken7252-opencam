/**
 ****************************************************************************************
 *
 * @file em_map.h
 *
 * @brief Mapping of the exchange memory
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */

#ifndef EM_MAP_H_
#define EM_MAP_H_

/**
 ****************************************************************************************
 * @addtogroup EM EM
 * @ingroup IP
 * @brief Mapping of the different common area in the exchange memory
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if BT_EMB_PRESENT
#include "_reg_em_et.h"
/// Exchange memory base address
#define EM_BASE_ADDR         REG_EM_ET_BASE_ADDR
#elif BLE_EMB_PRESENT
#include "_reg_common_em_et.h"
//#include "_reg_common_em_et.h"
/// Exchange memory base address
#define EM_BASE_ADDR         REG_COMMON_EM_ET_BASE_ADDR
#endif // BT_EMB_PRESENT/BLE_EMB_PRESENT


/*
 ****************************************************************************************
 **********************              Common EM part                **********************
 ****************************************************************************************
 */
/// Start of the common EM part
#define EM_COMMON_OFFSET     (0)

/*
 * EXCHANGE TABLE
 ****************************************************************************************
 */

/// Exchange table area definition
#define EM_EXCH_TABLE_LEN    16
#define EM_ET_OFFSET         (EM_COMMON_OFFSET)
#if BT_EMB_PRESENT
#define EM_ET_END            (EM_ET_OFFSET + EM_EXCH_TABLE_LEN * REG_EM_ET_SIZE)
#elif BLE_EMB_PRESENT
#define EM_ET_END            (EM_ET_OFFSET + EM_EXCH_TABLE_LEN * REG_COMMON_EM_ET_SIZE)
#endif // BT_EMB_PRESENT/BLE_EMB_PRESENT

/**
 * MODEn[3:0]
 *
 *  0x0: No mode selected, nothing to be performed
 *  0x1: BR/EDR Mode
 *  0x2: BLE Mode
 *  0x3-0xF: Reserved for future use           -
 */
#define EM_ET_MODE_NONE    0x00
#define EM_ET_MODE_BREDR   0x01
#define EM_ET_MODE_BLE     0x02

/**
 * STATUSn[1:0]
 *
 *  00: Control Structure Pointer is ready for processing.
 *  01: Control Structure Pointer is currently under process
 *  10: Reserved
 *  11: Reserved
 */
#define EM_ET_STATUS_READY           0x00
#define EM_ET_STATUS_UNDER_PROCESS   0x01

/**
 * ISOBUFSELn
 * Used to select Isochronous channel Buffer Pointer
 * 0x0: Uses ISO<0/1/2><TX/RX>PTR0
 * 0x1: Uses ISO<0/1/2><TX/RX>PTR1
 */
#define EM_ET_ISO_PTR0          0x00
#define EM_ET_ISO_PTR1          0x01

/**
 * ISOn
 * Indicates a Isochronous connection event is programmed
 * 0x0: Not an Isochronous Channel event
 * 0x1: Isochronous Channel event
 */
#define EM_ET_ISO_NOTPROGRAMMED     0x00
#define EM_ET_ISO_PROGRAMMED        0x01

/**
 * ISOCHANn[1:0]
 * Meaningful if ISOn = 1
 * 00: Select Isochronous channel 0.
 * 01: Select Isochronous channel 1.
 * 10: Select Isochronous channel 2.
 * 11: Trash received isochronous packet / Sent null length packet if ET-ISO = 1 / No Tx if ET-ISO = 0
 */
#define EM_ET_ISO_CHANNEL_0     0x00
#define EM_ET_ISO_CHANNEL_1     0x01
#define EM_ET_ISO_CHANNEL_2     0x02
#define EM_ET_ISO_NOCHANNEL     0x03


/**
 * RSVDn
 * Indicates a reserved event
 * 0x0: Not a reserved event (ISO connection re-Tx)
 * 0x1: Reserved event (ISO connection primary event)
 */
#define EM_ET_ISO_NOT_RSVD      0x00
#define EM_ET_ISO_RSVD          0x01
/*
 * FREQUENCY TABLE
 ****************************************************************************************
 */

/// Frequency table area definition
#define EM_FT_OFFSET         (EM_ET_END)

/// number of frequencies / Depends on RF target
#if defined(CFG_RF_ATLAS)
  /// IcyTRx requires 40 x 32-bit words for Frequency table + 40 byte for VCO sub-band table
  #define EM_RF_FREQ_TABLE_LEN  160
  #define EM_RF_VCOMMON_TABLE_LEN 40
  
#elif defined(CFG_RF_BLIM)
  /// IcyTRx requires 40 x 32-bit words for Frequency table + 40 byte for VCO sub-band table
  #define EM_RF_FREQ_TABLE_LEN  160
  #define EM_RF_VCOMMON_TABLE_LEN 40
#else
    #if BT_EMB_PRESENT
        /// Ripple/ExtRC requires 80 x 8-bit words for Frequency table / No VCO sub-band table
        #define EM_RF_FREQ_TABLE_LEN  80
        #define EM_RF_VCOMMON_TABLE_LEN 0
    #elif BLE_EMB_PRESENT
        /// Ripple/ExtRC requires 40 x 8-bit words for Frequency table / No VCO sub-band table
        #define EM_RF_FREQ_TABLE_LEN  40
        #define EM_RF_VCOMMON_TABLE_LEN 0
    #endif // BT_EMB_PRESENT/BLE_EMB_PRESENT
#endif

#define EM_FT_END            (EM_FT_OFFSET + (EM_RF_VCOMMON_TABLE_LEN + EM_RF_FREQ_TABLE_LEN) * sizeof(uint8_t))

/*
 * RF SW SPI
 ****************************************************************************************
 */
/// RF SW-Driven SPI transfers area definition 
#if defined(CFG_RF_ATLAS)
  /// IcyTYRx (Former Atlas) SW Driven SPI space
  #define EM_RF_SW_SPI_OFFSET     (EM_FT_END)
  #define EM_RF_SW_SPI_SIZE_MAX   100
  #define EM_RF_SW_SPI_END        (EM_RF_SW_SPI_OFFSET + EM_RF_SW_SPI_SIZE_MAX)

#elif defined(CFG_RF_BLIM)
  /// IcyTYRx (Former Atlas) SW Driven SPI space
  #define EM_RF_SW_SPI_OFFSET     (EM_FT_END)
  #define EM_RF_SW_SPI_SIZE_MAX   100
  #define EM_RF_SW_SPI_END        (EM_RF_SW_SPI_OFFSET + EM_RF_SW_SPI_SIZE_MAX)

#else
  /// Ripple SW Driven SPI space
  /// RF-SW-SPI transfers area definition 
  #define EM_RF_SW_SPI_OFFSET     (EM_FT_END)
  #define EM_RF_SW_SPI_SIZE_MAX   6
  #define EM_RF_SW_SPI_END        (EM_RF_SW_SPI_OFFSET + EM_RF_SW_SPI_SIZE_MAX)
#endif



/*
 * RF HW SPI
 ****************************************************************************************
 */
/// RF HW-Driven SPI transfers area definition 
#if defined(CFG_RF_ATLAS)
  /// IcyTYRx (Former Atlas) SW Driven SPI space
  #define EM_RF_HW_SPI_OFFSET     (EM_RF_SW_SPI_END)
  #define EM_RF_HW_SPI_SIZE_MAX   100
  #define EM_RF_HW_SPI_END        (EM_RF_HW_SPI_OFFSET + EM_RF_HW_SPI_SIZE_MAX)

#elif defined(CFG_RF_BLIM)
  /// IcyTYRx (Former Atlas) SW Driven SPI space
  #define EM_RF_HW_SPI_OFFSET     (EM_RF_SW_SPI_END)
  #define EM_RF_HW_SPI_SIZE_MAX   100
  #define EM_RF_HW_SPI_END        (EM_RF_HW_SPI_OFFSET + EM_RF_HW_SPI_SIZE_MAX)

#else
  /// Ripple SW Driven SPI space
  /// RF-SW-SPI transfers area definition 
  #define EM_RF_HW_SPI_OFFSET     (EM_RF_SW_SPI_END)
  #define EM_RF_HW_SPI_SIZE_MAX   0
  #define EM_RF_HW_SPI_END        (EM_RF_HW_SPI_OFFSET + EM_RF_HW_SPI_SIZE_MAX)
#endif


/// End of the common EM part
#define EM_COMMON_END        (EM_RF_HW_SPI_END)


/*
 ****************************************************************************************
 **********************                BLE EM part                 **********************
 ****************************************************************************************
 */

#if BLE_EMB_PRESENT
#define EM_BLE_OFFSET        (EM_COMMON_END)
#include "em_map_ble.h"
#endif //BLE_EMB_PRESENT



/*
 ****************************************************************************************
 **********************                BT EM part                  **********************
 ****************************************************************************************
 */

#if BT_EMB_PRESENT
#if BLE_EMB_PRESENT
#define EM_BT_OFFSET         (EM_BLE_END)
#else //BLE_EMB_PRESENT
#define EM_BT_OFFSET         (EM_COMMON_END)
#endif //BLE_EMB_PRESENT
#include "em_map_bt.h"
#endif //BT_EMB_PRESENT


/*
 * RF SPI part
 ****************************************************************************************
 */


/// @} IPDEXMEM

#endif // EM_MAP_H_
