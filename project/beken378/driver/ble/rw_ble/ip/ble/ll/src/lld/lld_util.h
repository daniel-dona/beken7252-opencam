/**
 ****************************************************************************************
 *
 * @file lld_util.h
 *
 * @brief Link layer driver utilities definitions
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */
#ifndef LLD_UTIL_H_
#define LLD_UTIL_H_
/**
 ****************************************************************************************
 * @addtogroup LLDUTIL
 * @ingroup LLD
 * @brief Link layer driver utilities definitions
 *
 * full description
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>

#include "common_bt.h"
#include "ea.h"
#include "lld_evt.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Minimum number of connection event for instant calculation (6 according to the SIG)
#define LLD_UTIL_MIN_INSTANT_CON_EVT         (6)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
enum lld_util_instant_action
{
    LLD_UTIL_NO_ACTION           = 0,
    LLD_UTIL_PARAM_UPDATE,
    LLD_UTIL_CHMAP_UPDATE,
    #if (BLE_2MBPS)
    LLD_UTIL_PHY_UPDATE,
    #endif // (BLE_2MBPS)
};


enum lld_cs_rate
{
    LLD_CS_RATE_1MBPS,
    LLD_CS_RATE_2MBPS,
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief  Get the BD address
 *
 * @param[out]    Pointer to BD address buffer
 ****************************************************************************************
 */
void lld_util_get_bd_address(struct bd_addr *bd_addr);

/**
 ****************************************************************************************
 * @brief  Set the BD address
 *
 * @param[in]    bd_addr    Pointer to BD address buffer
 * @param[in]    type       Address type (0:public, 1:random)
 ****************************************************************************************
 */
void lld_util_set_bd_address(struct bd_addr *bd_addr, uint8_t type);


/**
 ****************************************************************************************
 * @brief Force renewal of resolvable private addresses
 ****************************************************************************************
 */
void lld_util_ral_force_rpa_renew(void);
/**
 ****************************************************************************************
 * @brief  Convert frequecny in channel
 *
 * @param[in]    freq        Frequency to convert
 *
 * @return return the channel used
 *
 ****************************************************************************************
 */
uint8_t lld_util_freq2chnl(uint8_t freq);

/**
 ****************************************************************************************
 * @brief  Check if the element is in the programmed queue
 *
 * @param[in]    elt        element to be checked
 *
 * @return return true if the element has been found
 *
 ****************************************************************************************
 */
bool lld_util_elt_is_prog(struct ea_elt_tag *elt);


/**
 ****************************************************************************************
 * @brief This function returns the local offset (with respect to CLKN).
 *
 * @param[in] PeerOffset     Peer offset
 * @param[in] Interval       Interval
 * @param[in] AnchorPoint    Anchor point (with respect to CLKN)
 *
 * @return Offset value
 *
 ****************************************************************************************
 */
uint16_t lld_util_get_local_offset(uint16_t PeerOffset, uint16_t Interval, uint32_t AnchorPoint);

/**
 ****************************************************************************************
 * @brief This function returns the peer offset (with respect to CLKE).
 *
 * @param[in] LocalOffset      Local offset
 * @param[in] Interval         Interval
 * @param[in] AnchorPoint      Anchor point (with respect to CLKN)
 *
 * @return Offset value
 *
 ****************************************************************************************
 */
uint16_t lld_util_get_peer_offset(uint16_t LocalOffset, uint16_t Interval, uint32_t AnchorPoint);

/**
 ****************************************************************************************
 * @brief Function to set the duration and the start of the event
 *
 * @param[in] elt       element to update
 *
 * @param[in] param     parameter to compute the start of the event
 *
 ****************************************************************************************
 */
void lld_util_connection_param_set(struct ea_elt_tag *elt, struct ea_param_output* param);


/**
 ****************************************************************************************
 * @brief Free all the elements of the list
 *
 * This function is called to flush and free the elements contain in a list
 *
 * @param[in] list pointer on the list to be flushed
 *
 ****************************************************************************************
 */
void lld_util_flush_list(struct common_list *list);

/**
 ****************************************************************************************
 * @brief Set the Rxmax size and time in the CS for a dedicated link
 *
 * This function is called by the event scheduler when an event for a connection is ready
 * to be programmed. It updates the CS-RXMAXBUFF and CS_RXMAXTIME
 *
 * @param[in] conhdl handle for which the fields should be updated
 *
 ****************************************************************************************
 */
void lld_util_dle_set_cs_fields(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Function to launch automatically an anchor point move
 *
 * @param[in] elt       element to move
 *
 ****************************************************************************************
 */
void lld_util_anchor_point_move(struct ea_elt_tag *elt_connect);
#if (BLE_PERIPHERAL || BLE_CENTRAL)
/**
 ****************************************************************************************
 * @brief Check if an instant is already pending
 *
 * @param[in] elt       element to check
 *
 * @return if an instant is on going
 ****************************************************************************************
 */
bool  lld_util_instant_ongoing(struct ea_elt_tag *elt);
/**
 ****************************************************************************************
 * @brief Compute the new connection event max accordingly to the rx and tx time (DLE)
 *
 * @param[in] elt       element to update
 * @param[in] tx_time   rx time negotiated during DLE
 * @param[in] rx_time   tc time negotiated during DLE
 *
 ****************************************************************************************
 */

void lld_util_compute_ce_max(struct ea_elt_tag *elt, uint16_t tx_time, uint16_t rx_time);

#endif //(BLE_PERIPHERAL || BLE_CENTRAL)

/**
 ****************************************************************************************
 * @brief Check if an the element is already programmed or will be programmed
 *
 * @param[in] elt    element to check
 *
 * @return true if the element is programmed
 ****************************************************************************************
 */
bool lld_util_elt_programmed(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Set the priority used by the element
 *
 * @param[in] elt             element to set the priority
 * @param[in] priority_index  index of the priority
 *
 ****************************************************************************************
 */
void lld_util_priority_set(struct ea_elt_tag *elt, uint8_t priority_index);
/**
 ****************************************************************************************
 * @brief Update the default priority used by the element
 *
 * @param[in] elt       element to set the default priority
 * @param[in] value     value of the priority
 *
 ****************************************************************************************
 */
void lld_util_priority_update(struct ea_elt_tag *elt , uint8_t value);


#if (BLE_CENTRAL || BLE_PERIPHERAL)
/**
 ****************************************************************************************
 * Retrieve number of Packet to transmit in programmed queue
 *
 * @param elt Connection element
 *
 * @return Number of Packet to transmit in programmed queue
 ****************************************************************************************
 */
uint8_t lld_util_get_tx_pkt_cnt(struct ea_elt_tag *elt);


/**
 ****************************************************************************************
 * @brief Request to program a channel map update
 *
 * @param[in] evt     Pointer to the event for which the update has to be programmed
 * @param[in] action  Type of action which request the instant (channel map,...)
 *
 * @return The instant computed for the update
 *
 ****************************************************************************************
 */
uint16_t lld_util_instant_get(void *evt, uint8_t action);

#if (BLE_2MBPS)
/**
 ****************************************************************************************
 * @brief Gets the values of the RX and TX PHYS used by the element
 *
 * @param[in] elt       element to set
 *
 * @return    value of the rx and tx phys used by the link
 *
 ****************************************************************************************
 */
void lld_util_get_phys(struct ea_elt_tag *elt, uint8_t *tx_phy, uint8_t *rx_phy);


/**
 ****************************************************************************************
 * @brief Update phys parameters (in Slave mode)
 *
 * This function give to the driver the instant and the parameters of the future phys
 *
 * @param[in] elt        The element used
 * @param[in] instant    Instant (event counter) of the switch
 * @param[in] tx_phy     transmitter phy for the slave
 * @param[in] rx_phy     receiver phy for the slave
 *
 ****************************************************************************************
 */
void lld_util_phy_update_req(struct ea_elt_tag *elt, uint16_t instant, uint8_t tx_phy, uint8_t rx_phy);

/**
 ****************************************************************************************
 * @brief Update phys parameters (in Slave mode)
 *
 * This function request to change the current phys parameters.
 *
 * @param[in] elt        The element used
 ****************************************************************************************
 */
void lld_util_phy_update_ind(struct ea_elt_tag *elt);
#endif // (BLE2MBPS)
#endif // (BLE_CENTRAL || BLE_PERIPHERAL)

/**
 ****************************************************************************************
 * @brief Set maximum transmission time
 *
 * This function set the maximum time for a pdu transmission
 *
 * @param[in] elt           The element used
 * @param[in] max_tx_time   Maximum transmission time
 * @param[in] max_tx_time   Maximum transmission size
 ****************************************************************************************
 */
void lld_util_eff_tx_time_set(struct ea_elt_tag *elt, uint16_t max_tx_time, uint16_t max_tx_size);

/// @} LLDUTIL

#endif // LLD_UTIL_H_
