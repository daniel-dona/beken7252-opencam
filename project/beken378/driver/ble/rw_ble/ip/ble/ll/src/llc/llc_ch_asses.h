/**
 ****************************************************************************************
 *
 * @file llc_ch_asses.h
 *
 * @brief Declaration of functions used for channel assessment
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef LLC_CH_ASSES_H_
#define LLC_CH_ASSES_H_

/**
 ****************************************************************************************
 * @addtogroup LLCCHASSES
 * @ingroup LLC
 * @brief Channel assessment functions
 *
 * This module implements the primitives used for channel assessment
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if (BLE_CHNL_ASSESS)

#include <stdint.h>
#include <stdbool.h>
#include "rwip_config.h"
#include "common_bt.h"



/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */



/*
 * ENUMERATIONS
 ****************************************************************************************
 */

enum lld_ch_asses_ponderation
{
    // Noiser
    LLD_CH_ASSES_SYNC_ERR_HIGH_RSSI             = -3,
    // Packet error
    LLD_CH_ASSES_CRC_ERR                        = -3,
    // Sync missed
    LLD_CH_ASSES_SYNC_ERR_LOW_RSSI_NO_LATENCY   = -1,
    // Latency
    LLD_CH_ASSES_SYNC_ERR_LOW_RSSI_LATENCY      = 0,
    // Packet ok
    LLD_CH_ASSES_SYNC_FOUND_NO_CRC_ERR          = 3
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Structure describing the channel assessment environment
struct llc_ch_asses_tag
{
    /// Standard deviation
    int8_t  rcvd_quality[LE_DATA_FREQ_LEN];
    /// Latency enable
    bool latency_en;
    /// Re-assessment counter (nb of assessment timer expiration)
    uint8_t reassess_count;
    /// cursor to last channel re-assess
    uint8_t reassess_cursor;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Get the informations on the received packet (error, RSSI) to take the decision
 * to remove or not the channel
 *
 * @param[in] handle      Connection handle of the current link to be analyzed
 * @param[in] status      rx status information from rx descriptor
 * @param[in] rssi        rssi value of received pdu
 * @param[in] channel     channel number of RX packet
 *
 ****************************************************************************************
 */
void llc_ch_assess_local(uint16_t conhdl, uint16_t status, int8_t rssi, uint8_t channel);

/**
 ****************************************************************************************
 * @brief Get the local channel map
 *
 * @param[in] conhdl      Connection handle of the current link
 * @param[in] map         pointer to local channel map
 * @param[in] hostmap     pointer to channel map configured by host
 *
 * @return number of good channels
 ****************************************************************************************
 */
uint8_t llc_ch_assess_get_local_ch_map(uint16_t conhdl, struct le_chnl_map *map, struct le_chnl_map *hostmap);

/**
 ****************************************************************************************
 * @brief Get the current channel map
 *
 * @param[in] conhdl      Connection handle of the current link
 *
 * @param[out] le_chnl_map   Current channel map
 *
 ****************************************************************************************
 */
struct le_chnl_map * llc_ch_assess_get_current_ch_map(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Re-enable 25% of new channel (if possible) and at least ensure that 2 channels are available
 *
 * @param[in] conhdl      Connection handle of the current link
 * @param[in] map         pointer to local channel map
 * @param[in] hostmap     pointer to channel map configured by host
 * @param[in] nb_chgood   Current number of good channels
 *
 ****************************************************************************************
 */
void llc_ch_assess_reass_ch(uint16_t conhdl, struct le_chnl_map *map, struct le_chnl_map *hostmap, uint8_t nb_chgood);
#endif//#if (BLE_CHNL_ASSESS)

/// @} LLCCHASSES

#endif // LLC_CH_ASSES_H_
