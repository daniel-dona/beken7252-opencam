/**
 ****************************************************************************************
 *
 * @file em_map_ble.h
 *
 * @brief Mapping of the different descriptors in the exchange memory
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */

#ifndef EM_MAP_BLE_H_
#define EM_MAP_BLE_H_

/**
 ****************************************************************************************
 * @addtogroup EM EM
 * @ingroup CONTROLLER
 * @brief Mapping of the different descriptors in the exchange memory
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"                // stack configuration

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "common_bt.h"
#include "_reg_ble_em_tx_desc.h"
#include "_reg_ble_em_tx_buffer_data.h"
#include "_reg_ble_em_tx_buffer_cntl.h"
#include "_reg_ble_em_rx_desc.h"
#include "_reg_ble_em_rx_buffer.h"
#include "_reg_ble_em_wpb.h"
#include "_reg_ble_em_wpv.h"
#include "_reg_ble_em_ral.h"
#include "_reg_ble_em_cs.h"

#if (BLE_AUDIO)
#include "_reg_ble_em_tx_audio_buffer.h"
#include "_reg_ble_em_rx_audio_buffer.h"
#endif // (BLE_AUDIO)


/*
 * DEFINES
 ****************************************************************************************
 */



/// Size of the encryption area
#define EM_BLE_ENC_LEN         16

/// number of control structure entries for the exchange memory
#define EM_BLE_CS_COUNT        (BLE_CONNECTION_MAX + 1)

/// number of tx buffer entries for the exchange memory
#define EM_BLE_TX_BUFF_CNTL_COUNT   (BLE_TX_BUFF_CNTL)

/// number of tx buffer entries for the exchange memory
#define EM_BLE_TX_BUFF_ADV_COUNT   (BLE_TX_BUFF_ADV)

/// number of tx buffer entries for the exchange memory
#define EM_BLE_TX_BUFF_DATA_COUNT   (BLE_TX_BUFF_DATA)


/// number of tx descriptor entries for the exchange table
#define EM_BLE_TX_DESC_COUNT   (BLE_TX_DESC_CNT)

/// number of tx buffer entries for the exchange memory
#define EM_BLE_TX_BUFFER_COUNT (BLE_TX_BUFFER_CNT)

/// number of rx descriptor entries for the exchange memory
#define EM_BLE_RX_DESC_COUNT   (BLE_RX_DESC_CNT)

/// number of rx buffer entries for the exchange memory
#define EM_BLE_RX_BUFFER_COUNT (BLE_RX_BUFFER_CNT)

#if (BLE_AUDIO)
/// number of tx audio buffer entries for the exchange memory
#define EM_BLE_TX_AUDIO_BUFF_DATA_COUNT (BLE_TX_AUDIO_BUFFER_CNT)

/// number of rx audio buffer entries for the exchange memory
#define EM_BLE_RX_AUDIO_BUFF_DATA_COUNT (BLE_RX_AUDIO_BUFFER_CNT)
#endif // (BLE_AUDIO)

/*
 * Mapping of the different elements in EM
 ****************************************************************************************
 */

#define EM_BLE_ET_OFFSET                (EM_ET_OFFSET)

/// Offset of the plain data area (used for SW initiated encryption)
#define EM_BLE_ENC_PLAIN_OFFSET         (EM_BLE_OFFSET)
/// Offset of the ciphered data area (used for SW initiated encryption)
#define EM_BLE_ENC_CIPHER_OFFSET        (EM_BLE_ENC_PLAIN_OFFSET + EM_BLE_ENC_LEN * sizeof(uint8_t))
/// Offset of the control structure area
#define EM_BLE_CS_OFFSET                (EM_BLE_ENC_CIPHER_OFFSET + EM_BLE_ENC_LEN * sizeof(uint8_t))
/// Offset of the public white list area
#define EM_BLE_WPB_OFFSET               (EM_BLE_CS_OFFSET + EM_BLE_CS_COUNT * REG_BLE_EM_CS_SIZE)
/// Offset of the private white list area
#define EM_BLE_WPV_OFFSET               (EM_BLE_WPB_OFFSET + BLE_WHITELIST_MAX * REG_BLE_EM_WPB_SIZE)
/// Offset of the private white list area
#define EM_BLE_RAL_OFFSET               (EM_BLE_WPV_OFFSET + BLE_WHITELIST_MAX * REG_BLE_EM_WPV_SIZE)
/// Offset of the TX descriptor area
#define EM_BLE_TX_DESC_OFFSET           (EM_BLE_RAL_OFFSET + BLE_RESOL_ADDR_LIST_MAX * REG_BLE_EM_RAL_SIZE)
/// Offset of the RX descriptor area
#define EM_BLE_RX_DESC_OFFSET           (EM_BLE_TX_DESC_OFFSET + EM_BLE_TX_DESC_COUNT * REG_BLE_EM_TX_DESC_SIZE)
/// Offset of the TX buffer area
#define EM_BLE_TX_BUFFER_CNTL_OFFSET    (EM_BLE_RX_DESC_OFFSET + EM_BLE_RX_DESC_COUNT * REG_BLE_EM_RX_DESC_SIZE)
/// Offset of the TX buffer area
#define EM_BLE_TX_BUFFER_DATA_OFFSET    (EM_BLE_TX_BUFFER_CNTL_OFFSET + (EM_BLE_TX_BUFF_CNTL_COUNT + EM_BLE_TX_BUFF_ADV_COUNT) * REG_BLE_EM_TX_BUFFER_CNTL_SIZE)
/// Offset of the RX buffer area
#define EM_BLE_RX_BUFFER_OFFSET         (EM_BLE_TX_BUFFER_DATA_OFFSET + EM_BLE_TX_BUFF_DATA_COUNT * REG_BLE_EM_TX_BUFFER_DATA_SIZE)
#if (BLE_AUDIO)
/// Offset of the TX Audio buffer area
#define EM_BLE_TX_AUDIO_BUFFER_OFFSET   (EM_BLE_RX_BUFFER_OFFSET + EM_BLE_RX_BUFFER_COUNT * REG_BLE_EM_RX_BUFFER_SIZE)
/// Offset of the RX Audio buffer area
#define EM_BLE_RX_AUDIO_BUFFER_OFFSET   (EM_BLE_TX_AUDIO_BUFFER_OFFSET + EM_BLE_TX_AUDIO_BUFF_DATA_COUNT * REG_BLE_EM_TX_AUDIO_BUFFER_SIZE)
/// End of BLE EM
#define EM_BLE_END                      (EM_BLE_RX_AUDIO_BUFFER_OFFSET + EM_BLE_RX_AUDIO_BUFF_DATA_COUNT * REG_BLE_EM_RX_AUDIO_BUFFER_SIZE)
#else // !(BLE_AUDIO)
/// End of BLE EM
#define EM_BLE_END                      (EM_BLE_RX_BUFFER_OFFSET + EM_BLE_RX_BUFFER_COUNT * REG_BLE_EM_RX_BUFFER_SIZE)
#endif // (BLE_AUDIO)

/// @} LLDEXMEM

#endif // EM_MAP_BLE_H_
