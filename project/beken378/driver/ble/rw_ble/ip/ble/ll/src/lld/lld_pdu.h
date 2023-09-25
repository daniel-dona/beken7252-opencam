/**
 ****************************************************************************************
 *
 * @file lld_pdu.h
 *
 * @brief Functions for adv\acl\llcp transmission/reception handling
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef LLD_PDU_H_
#define LLD_PDU_H_

/**
 ****************************************************************************************
 * @addtogroup LLDPDU LLDPDU
 * @ingroup LLD
 * @brief Functions for adv/acl/llcp transmission/reception handling
 *
 * This module implements the primitives allowing the LLC asking for adv/acl/llcp  transmission.
 * It configures the Tx lists and handles the reception.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "em_buf.h"
#include "lld.h"
#include "lld_evt.h"

/*
 * MESSAGE PARAMETER STRUCTURES
 ****************************************************************************************
 */
/// ACL data TX element structure
struct lld_pdu_data_tx_tag
{
    /// List element for chaining
    struct common_list_hdr hdr;
    /// index
    uint16_t    idx;
    /// connection handle
    uint16_t    conhdl;
    /// length of the data
    uint16_t    length;
    /// broadcast and packet boundary flag
    uint8_t     pb_bc_flag;
    #if (BLE_EMB_PRESENT)
    /// Pointer to the first descriptor containing RX Data
    struct em_buf_node *buf;
    #else // (BLE_HOST_PRESENT)
    /// Pointer to the data buffer
    uint8_t* buffer;
    #endif // (BLE_EMB_PRESENT) / (BLE_HOST_PRESENT)
};


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Ask for a adv/connection/scan transmission
 *
 * This function is called by the LLM to request for a transmission. It chains the
 * descriptor passed as parameter in the list of descriptors ready for transmission.
 *
 * @param[in] elt    Pointer to the element for which a transmission is requested
 * @param[in] txnode Pointer to the TX node
 *
 ****************************************************************************************
 */
void lld_pdu_tx_push(struct ea_elt_tag *elt, struct em_desc_node *txnode);

/**
 ****************************************************************************************
 * @brief Ask for a data transmission
 *
 * This function is called by the LLC to request for a transmission. It chains the
 * descriptor passed as parameter in the list of descriptors ready for transmission.
 *
 * @param[in] elt           Pointer to the element for which a transmission is requested
 * @param[in] txnode        Pointer to the TX node
 * @param[in] can_be_freed  Flag to indicate if the buffer can be freed
 *
 ****************************************************************************************
 */
void lld_pdu_data_tx_push(struct lld_evt_tag *evt , struct em_desc_node* txnode, bool can_be_freed, bool encrypted);

/**
 ****************************************************************************************
 * @brief Loop back the TX data in the exchange memory
 *
 * This function chains the next descriptor pointer of the last TX descriptor with the
 * first TX descriptor. It therefore creates a loop.
 *
 * @param[in] evt Event associated with the data to loop
 *
 ****************************************************************************************
 */
void lld_pdu_tx_loop(struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief Prepare buffer pointers in the control structure for TX
 *
 * This function is called by the event scheduler when an event for a connection is ready
 * to be programmed. It chains the descriptors ready for transmissions with the ones
 * already programmed, and update the control structure with the pointer to the first
 * descriptor.
 *
 * @param[in] evt Event for which the buffers have to be programmed
 *
 ****************************************************************************************
 */
void lld_pdu_tx_prog(struct lld_evt_tag *evt);


/**
 ****************************************************************************************
 * @brief Flush the data currently programmed for transmission
 *
 * @param[in] evt Pointer to the event for which TX data has to be flushed
 ****************************************************************************************
 */
void lld_pdu_tx_flush(struct lld_evt_tag *evt);

uint8_t lld_pdu_adv_pack(uint8_t code, uint8_t* buf, uint8_t* p_len);


/**
 ****************************************************************************************
 * @brief Called in interrupt context, it retrieves information about received data packet
 * and decide if it should process it within interrupt context or to defer it.
 *
 * @param[in] evt        Pointer on the on-going event
 * @param[in] nb_rx_desc Number of RX descriptor updated by HW
 ****************************************************************************************
 */
void lld_pdu_rx_handler(struct lld_evt_tag *evt, uint8_t nb_rx_desc);

/**
 ****************************************************************************************
 * @brief Called in event context, it retrieves information about received and acknowledged
 * data packet
 *
 * @param[in] evt           Pointer on the event defered
 * @param[out] elt_deleted  Return if the element has been deleted (scan evt during init)
 ****************************************************************************************
 */
bool lld_pdu_check(struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief Called in background context, it pushes the data to transmit in the ready to be
 * sent queue
 *
 * @param[in] param    Pointer on element to be sent
 *
 * Return true if pushed in teh ready queue
 ****************************************************************************************
 */
bool lld_pdu_data_send(void *param);

/// @} LLDPDU

#endif // LLD_PDU_H_
