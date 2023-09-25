/**
 ****************************************************************************************
 *
 * @file llc_util.h
 *
 * @brief Link layer controller utilities definitions
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */
#ifndef LLC_UTIL_H_
#define LLC_UTIL_H_
/**
 ****************************************************************************************
 * @addtogroup LLCUTIL
 * @ingroup LLC
 * @brief Link layer controller utilities definitions
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

#include "ble_compiler.h"
#include "em_buf.h"
#include "llc_task.h"
#include "reg_ble_em_rx_desc.h"
#include "reg_ble_em_cs.h"
#include "llcontrl.h"
#if (BLE_PERIPHERAL || BLE_CENTRAL)

/*
 * MARCROS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Get PDU Encryption state value
 *
 * @param[in] conhdl  handle of the link where the flow should be get
 *
 * @return Encryption state
 *
 ****************************************************************************************
 */
#define LLC_UTIL_ENC_STATE_GET(conhdl)\
    (llc_env[(conhdl)]->encryption_state)

/**
 ****************************************************************************************
 * @brief Check if the Encryption state value is set
 *
 * @param[in] conhdl   handle of the link where the flow should be on/off
 * @param[in] field    State field
 *
 * @return True if state is enabled ; False else
 ****************************************************************************************
 */
#define LLC_UTIL_ENC_STATE_IS_SET(conhdl, field) \
    (((llc_env[(conhdl)]->encryption_state) & (field)) == (field))

/**
 ****************************************************************************************
 * @brief Set the Encryption state value
 *
 * @param[in] conhdl   handle of the link where the flow should be on/off
 * @param[in] value    value of the encryption state
 *
 ****************************************************************************************
 */
#define LLC_UTIL_ENC_STATE_SET(conhdl, value)\
    (llc_env[(conhdl)]->encryption_state = (value))

/**
 ****************************************************************************************
 * @brief Update the Encryption state value
 *
 * @param[in] conhdl   handle of the link where the flow should be on/off
 * @param[in] field    Field value to update
 * @param[in] enable   Enable or not the state field
 *
 ****************************************************************************************
 */
#define LLC_UTIL_ENC_STATE_UP(conhdl, field, enable) \
    llc_env[(conhdl)]->encryption_state = (enable)\
                                        ? ((llc_env[(conhdl)]->encryption_state) |  (field)) \
                                        : ((llc_env[(conhdl)]->encryption_state) & ~(field))



/*
 * DEFINES
 ****************************************************************************************
 */

// Encryption state
enum llc_util_enc_state
{
    /// Encryption is disabled
    LLC_ENC_DISABLED       = 0x00,

    /// Encryption enabled in TX
    LLC_ENC_TX              = 0x01,
    /// Encryption enabled in RX
    LLC_ENC_RX              = 0x02,
    /// Encryption enabled for TX and RX
    LLC_ENC_ENABLE          = 0x03,

    ///TX flow is now allowed
    LLC_ENC_TX_FLOW_OFF     = 0x04,
    ///RX flow is now allowed
    LLC_ENC_RX_FLOW_OFF     = 0x08,
    /// Both TX and RX Flow are off
    LLC_ENC_FLOW_OFF        = 0x0C,

    /// Encryption key refresh procedure is pending
    LLC_ENC_REFRESH_PENDING = 0x10,
    /// Encryption pause procedure is pending
    LLC_ENC_PAUSE_PENDING   = 0x20,
};


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Gets a free connection handle.
 *
 * This function allocates a new connection handle if possible.
 *
 * @param[in/out] conhdl         Pointer on the connection handle allocated.
 *
 * @return If the connection handle has been correctly allocated.
 *
 ****************************************************************************************
 */
uint8_t llc_util_get_free_conhdl(uint16_t *conhdl);


/**
 ****************************************************************************************
 * @brief Process the disconnection
 *
 * This function request to the Link Layer to stop sending any further packets, frees the
 * LLC conhdl and LLC task associated, notifies the host of the loss of connection.
 *
 * @param[in] conhdl         Connection handle disconnected.
 * @param[in] reason         reason of the disconnection.
 *
 ****************************************************************************************
 */
void llc_util_dicon_procedure(uint16_t conhdl, uint8_t reason);

/**
 ****************************************************************************************
 * @brief  Checks the number of active(s) link(s).
 *
 * This function returns the number of active(s) link(s).
 *
 * @return The number of active(s) link(s).
 *
 ****************************************************************************************
 */
__INLINE uint8_t llc_util_get_active_conhdl(void)
{
    uint8_t idx;
    uint8_t cpt_active=0;
    kernel_task_id_t llc_free;
    for(idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        // build a task ID dedicated to the conhdl
        llc_free = KERNEL_BUILD_ID(TASK_LLC, idx);
        // gets the state
        if(kernel_state_get(llc_free)!= LLC_FREE)
        {
            cpt_active++;
        }
    }
    return cpt_active;
}

/**
 ****************************************************************************************
 * @brief  Checks if the meta event is enabled.
 *
 * This function checks if the meta event is authorized to be sent to the host.
 *
 * @param[in] meta_event         Event to be checked.
 *
 * @return The is the event is authorized.
 *
 ****************************************************************************************
 */
__INLINE bool llc_util_event_enabled(uint8_t meta_event)
{
    uint8_t idx;
    uint8_t cpt_active=0;
    kernel_task_id_t llc_free;
    for(idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        // build a task ID dedicated to the conhdl
        llc_free = KERNEL_BUILD_ID(TASK_LLC, idx);
        // gets the state
        if(kernel_state_get(llc_free)!= LLC_FREE)
        {
            cpt_active++;
        }
    }
    return cpt_active;
}

/**
 ****************************************************************************************
 * @brief  Gets the L2CAP length.
 *
 * This function gets in the data buffer the L2CAP length.
 *
 * @param[in] data         Pointer on the data buffer .
 *
 * @return The L2CAP data length.
 *
 ****************************************************************************************
 */
__INLINE uint16_t llc_util_rxl2clen_getf(uint8_t *data)
{
    uint16_t localVal =  *data++ & 0x00FF; //gets the lsb first
    localVal |= (uint16_t) (*data << 8) & 0xFF00;
    return (localVal);
}

/**
 ****************************************************************************************
 * @brief  Set the IVm in the control structure
 *
 * @param[in] conhdl    Handle of the connection for which the IVm is set
 * @param[in] ivm       Pointer to the IVm to set for this connection
 *
 ****************************************************************************************
 */
__INLINE void llc_util_ivm_set(uint16_t conhdl, uint8_t const *ivm)
{
	int i = 0;
    for ( i = 0; i < 2; i++)
    {
        // Sets the lower part of the IV with the IVm
        ble_iv_setf(conhdl, i , common_read16p(&ivm[2*i]));
    }
}

/**
 ****************************************************************************************
 * @brief  Set the IVs in the control structure
 *
 * @param[in] conhdl    Handle of the connection for which the IVs is set
 * @param[in] ivs       Pointer to the IVs to set for this connection
 *
 ****************************************************************************************
 */
__INLINE void llc_util_ivs_set(uint16_t conhdl, uint8_t const *ivs)
{
	int i = 0;
    for ( i = 0; i < 2; i++)
    {
        // Sets the upper part of the IV with the IVs
        ble_iv_setf(conhdl, i + 2 , common_read16p(&ivs[2*i]));
    }
}

/**
 ****************************************************************************************
 * @brief  Update the channel map of a specific link
 *
 * @param[in] conhdl    Handle of the connection for which the IVs is set
 * @param[in] map       Pointer to the channel map
 ****************************************************************************************
 */
void llc_util_update_channel_map(uint16_t conhdl, struct le_chnl_map *map);

/**
 ****************************************************************************************
 * @brief  Enable/disable LLCP discard
 *
 * @param[in] conhdl    Connection handle
 ****************************************************************************************
 */
void llc_util_set_llcp_discard_enable(uint16_t conhdl, bool enable);

/**
 ****************************************************************************************
 * @brief  Calculates and sets an appropriate margin for the authenticated payload timeout
 *
 * @param[in] conhdl    Handle of the connection for which the IVs is set
 * @param[in] enable    True/false
 ****************************************************************************************
 */
void llc_util_set_auth_payl_to_margin(struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief  Check disc command param
 *
 * @param[in] reason    Reason for disconnection
 *
 * @return reason accepted or rejected
 ****************************************************************************************
 */
__INLINE bool llc_util_disc_reason_ok(uint8_t reason)
{
    // disconnection reason range
    uint8_t valid_r[7] = {
            COMMON_ERROR_AUTH_FAILURE,
            COMMON_ERROR_REMOTE_USER_TERM_CON,
            COMMON_ERROR_REMOTE_DEV_TERM_LOW_RESOURCES,
            COMMON_ERROR_REMOTE_DEV_POWER_OFF,
            COMMON_ERROR_UNSUPPORTED_REMOTE_FEATURE,
            COMMON_ERROR_PAIRING_WITH_UNIT_KEY_NOT_SUP,
            COMMON_ERROR_UNACCEPTABLE_CONN_INT
    };
    uint8_t i = 0;
    for ( i = 0; i < 7; i++)
        if (reason == valid_r[i]) return (true);

    return (false);
}

/**
 ****************************************************************************************
 * @brief Set operation pointer
 *
 * @param[in] conhdl        Connection handle.
 * @param[in] op_type       Operation type.
 * @param[in] op            Operation pointer.
 *
 ****************************************************************************************
 */
__INLINE void llc_util_set_operation_ptr(uint16_t conhdl, uint8_t op_type, void* op)
{
    ASSERT_ERR(op_type < LLC_OP_MAX);
    struct llc_env_tag *llc_env_ptr = llc_env[conhdl];
    // update operation pointer
    llc_env_ptr->operation[op_type] = op;
}

/**
 ****************************************************************************************
 * @brief Get operation pointer
 *
 * @param[in] conhdl        Connection handle.
 * @param[in] op_type       Operation type.
 *
 * @return operation pointer on going
 ****************************************************************************************
 */
__INLINE void* llc_util_get_operation_ptr(uint16_t conhdl, uint8_t op_type)
{
    ASSERT_ERR(op_type < LLC_OP_MAX);
    struct llc_env_tag *llc_env_ptr = llc_env[conhdl];
    // return operation pointer
    return llc_env_ptr->operation[op_type];
}
/**
 ****************************************************************************************
 * @brief Get element pointer
 *
 * @param[in] conhdl        Connection handle.
 *
 * @return element pointer on going
 ****************************************************************************************
 */
__INLINE struct ea_elt_tag * llc_util_get_element_ptr(uint16_t conhdl)
{
    ASSERT_ERR(conhdl <= BLE_CONNECTION_MAX);
    // return element pointer
    return ((void*)(llc_env[conhdl]->elt));
}
/**
 ****************************************************************************************
 * @brief Clear  operation and allocated area
 *
 * @param[in] conhdl        Connection handle.
 * @param[in] op_type       Operation type.
 *
 ****************************************************************************************
 */
void llc_util_clear_operation_ptr(uint16_t conhdl, uint8_t op_type);

/**
 ****************************************************************************************
 * @brief Check if the bandwidth after data length extension procedure has changed
 *
 * @param[in] conhdl        Connection handle.
 *
 ****************************************************************************************
 */
void llc_util_bw_mgt(uint16_t conhdl);


/**
 ****************************************************************************************
 * @brief Check if procedure state is on-going
 *
 * @param state     Current task state
 * @param procedure Procedure to check
 *
 * @return true if procedure is on-going, false else
 ****************************************************************************************
 */
__INLINE bool llc_state_chk(kernel_state_t state, uint8_t procedure)
{
    return (((state) & (procedure)) == (procedure));
}

/**
 ****************************************************************************************
 * @brief Update task state according to on-going procedure
 *
 * @param[in] task    Task where state has to be updated
 * @param[in|out] state     task state
 * @param[in] procedure Procedure bit to update
 * @param[in] enable    Enable or disable procedure
 ****************************************************************************************
 */
__INLINE void llc_state_update(kernel_task_id_t task, kernel_state_t *state, uint8_t procedure, bool enable)
{
    *state = enable ? ((*state) | (procedure)) : ((*state) & ~(procedure));

    kernel_state_set(task, *state);
}


/**
 ****************************************************************************************
 * @brief Handling defer of End of event
 *
 * @param[in] conhdl   Connection handle
 ****************************************************************************************
 */
void llc_end_evt_defer(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Handling defer of TX ACL acknowledgment
 *
 * @param[in] conhdl   Connection handle
 * @param[in] tx_cnt   Number of packet acknowledged
 *
 ****************************************************************************************
 */
void llc_pdu_acl_tx_ack_defer(uint16_t conhdl, uint8_t tx_cnt);

/**
 ****************************************************************************************
 * @brief Handling defer of TX LLCP acknowledgment
 *
 * @param[in] conhdl   Connection handle
 * @param[in] opcode   LLCP Operation code
 *
 ****************************************************************************************
 */
void llc_pdu_llcp_tx_ack_defer(uint16_t conhdl, uint8_t opcode);


/**
 ****************************************************************************************
 * @brief Handling of pdu information in a deferred event
 *
 * @param[in] conhdl   Connection handle
 * @param[in] status   RX status
 * @param[in] rssi     RX detected RSSI raw value
 * @param[in] channel  Channel of packet received
 * @param[in] length   Length of packet received
 ****************************************************************************************
 */
void llc_pdu_defer(uint16_t conhdl, uint16_t status, uint8_t rssi, uint8_t channel, uint8_t length);

#endif // #if (BLE_PERIPHERAL || BLE_CENTRAL)
/// @} LLCUTIL
#endif // LLC_UTIL_H_
