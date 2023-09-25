/**
 ****************************************************************************************
 *
 * @file llc_llcp.h
 *
 * @brief Functions for control pdu transmission/reception handling
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef LLC_CNTL_H_
#define LLC_CNTL_H_

/**
 ****************************************************************************************
 * @addtogroup LLCLLCP LLCLLCP
 * @ingroup LLC
 * @brief Functions for control pdu transmission/reception handling
 *
 * This module implements the functions allowing the handling of the transmission and
 * reception of the control pdu.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "common_llcp.h"
#include "kernel_msg.h"
#include "llc_task.h"

#if (BLE_PERIPHERAL || BLE_CENTRAL)
/// Maximum size of LMP unpacked parameters
#define LLCP_MAX_UNPACKED_SIZE     (LLCP_PDU_LENGTH_MAX + MIC_LEN)

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// LLCP pdu element structure
struct llcp_pdu_tag
{
    /// List element for chaining
    struct common_list_hdr hdr;
    /// Node index
    uint16_t idx;
    /// Pointer on the pdu to send
    void *ptr;
    #if (BLE_TESTER)
    uint16_t pdu_length;
    #endif // BLE_TESTER

    /// opcode
    uint8_t opcode;
};


/// Status to check if a llcp can be sent or not during start en pause encryption
enum llc_llcp_authorize
{
    LLC_LLCP_NO_AUTHZED         = 0x00,
    LLC_LLCP_START_ENC_AUTHZED  = 0x01,
    LLC_LLCP_PAUSE_ENC_AUTHZED  = 0x02,
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Sends the (extended) reject indication pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the PDU will be sent.
 * @param[in] reason        The reason to be put in the Reject Indication PDU
 ****************************************************************************************
 */
void llc_llcp_reject_ind_pdu_send(uint16_t conhdl, uint8_t rej_opcode, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Sends the read remote information version pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the PDU will be sent.
 ****************************************************************************************
 */
void llc_llcp_version_ind_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the set host channel classification pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_ch_map_update_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the pause encryption request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_pause_enc_req_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the pause encryption response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_pause_enc_rsp_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the encryption request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         The parameters to be put in the encryption request
 ****************************************************************************************
 */
void llc_llcp_enc_req_pdu_send(uint16_t conhdl, struct hci_le_start_enc_cmd *param);

/**
 ****************************************************************************************
 * @brief Sends the encryption response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_enc_rsp_pdu_send(uint16_t conhdl, struct llcp_enc_req *param);

/**
 ****************************************************************************************
 * @brief Sends the start encryption response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_start_enc_rsp_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the connection update request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         Pointer on the structure which contains all the updated
 *                          parameters useful for the link.
 ****************************************************************************************
 */
void llc_llcp_con_update_pdu_send(uint16_t conhdl, struct llcp_con_upd_ind *param);

/**
 ****************************************************************************************
 * @brief Sends the connection parameters request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         Pointer on the structure which contains all the updated
 *                          parameters useful for the link.
 ****************************************************************************************
 */
void llc_llcp_con_param_req_pdu_send(uint16_t conhdl, struct llc_con_upd_req_ind *param);

/**
 ****************************************************************************************
 * @brief Sends the connection parameters response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         Pointer on the structure which contains all the updated
 *                          parameters useful for the link.
 ****************************************************************************************
 */
void llc_llcp_con_param_rsp_pdu_send(uint16_t conhdl, struct llc_con_upd_req_ind *param);

/**
 ****************************************************************************************
 * @brief Sends the features request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_feats_req_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the features response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_feats_rsp_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the start encryption request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_start_enc_req_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the terminate indication pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] err_code      Reason of the termination.
 ****************************************************************************************
 */
void llc_llcp_terminate_ind_pdu_send(uint16_t conhdl, uint8_t err_code);
/**
 ****************************************************************************************
 * @brief Sends the unknown response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] unk_type      Opcode of the unknown pdu type .
 ****************************************************************************************
 */
void llc_llcp_unknown_rsp_send_pdu(uint16_t conhdl, uint8_t unk_type);

#if !(BLE_QUALIF)
/**
 ****************************************************************************************
 * @brief Sends the ping request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_ping_req_pdu_send(uint16_t conhdl);
/**
 ****************************************************************************************
 * @brief Sends the ping response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_ping_rsp_pdu_send(uint16_t conhdl);
/**
 ****************************************************************************************
 * @brief Sends the length request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_length_req_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the length response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_llcp_length_rsp_pdu_send(uint16_t conhdl);


/**
 ******************************************************************************************
 * @brief GENERAL COMMENT FOR llcp_..._pdu_unpk : LMP PDU param extraction function
 *
 * @param[in] pdu      Pointer to PDU buffer, without the 1 or two opcode bytes.
 * @param[in] parlen   Length of left over pdu params.
 * @param[in] param    Pointer to kernel message param position for direct copy of pdu params
 *
 ******************************************************************************************
 */
#endif

#if (BLE_TESTER)
/**
 ****************************************************************************************
 * @brief Sends through tester the  LLCP PDU on the given connection handle.
 *
 * @param[in] conhdl  The connection handle on which the LLCP has to be transmitted
 * @param[in] length  The length of the LLCP packet (required to build the header)
 * @param[in] dara    The LLCP PDU Data
 *
 ****************************************************************************************
 */
void llc_llcp_tester_send(uint8_t conhdl, uint8_t length, uint8_t *data);
#endif // (BLE_TESTER)

#if (BLE_2MBPS)
/**
 ****************************************************************************************
 * @brief Sends the PHY request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         RX and TX phys requested
 ****************************************************************************************
 */
void llc_llcp_phy_req_pdu_send(uint16_t conhdl, struct  llcp_phy_req *param);

/**
 ****************************************************************************************
 * @brief Sends the PHY response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         RX and TX phys response
 ****************************************************************************************
 */
void llc_llcp_phy_rsp_pdu_send(uint16_t conhdl, struct  llcp_phy_rsp *param);

/**
 ****************************************************************************************
 * @brief Sends the PHY update request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         RX and TX phys chosen and the instant
 ****************************************************************************************
 */
void llc_llcp_phy_upd_ind_pdu_send(uint16_t conhdl,struct  llcp_phy_upd_req *param);

#endif // (BLE_2MBPS)

/**
 ****************************************************************************************
 * @brief Default handler for LLCP packet received.
 *
 * @param[in] dest_id    ID of the receiving task instance.
 * @param[in] status     status of the PDU rx unpack.
 * @param[in] pdu        Pointer to the received PDU.
 * @param[in] int_ctx    True if executing within interrupt context, False else
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int llc_llcp_recv_handler(kernel_task_id_t dest_id, uint8_t status, union llcp_pdu* pdu, bool int_ctx);

/**
 ****************************************************************************************
 * @brief Get the authorization value for a dedicated LLCP
 *
 * @param[in] opcode  Opcode of the LLCP to be checked
 *
 * @return Authorization value see enum llc_llcp_authorize
 ****************************************************************************************
 */
uint8_t llc_llcp_get_autorize(uint8_t opcode);

#endif // #if (BLE_PERIPHERAL || BLE_CENTRAL)



/// @} LLCLLCP

#endif // LLC_CNTL_H_
