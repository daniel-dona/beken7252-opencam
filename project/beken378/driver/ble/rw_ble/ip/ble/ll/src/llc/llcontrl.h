/**
 ****************************************************************************************
 *
 * @file llcontrl.h
 *
 * @brief Main API file for the Link Layer controller
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */

#ifndef LLCONTRL_H_
#define LLCONTRL_H_

/**
 ****************************************************************************************
 * @addtogroup LLC LLC
 * @ingroup CONTROLLER
 * @brief Link Layer Controller
 *
 * Declaration of the functions used by the logical link controller
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "common_bt.h"
#include "common_error.h"
#include "common_llcp.h"
#include "kernel_task.h"
#include "llc_llcp.h"
#include "llc_task.h"
#include "llm.h"
#include "lld_evt.h"
#if (BLE_CHNL_ASSESS)
#include "llc_ch_asses.h"
#endif //(BLE_CHNL_ASSESS)
#if (BLE_PERIPHERAL || BLE_CENTRAL)

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * MACROS
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// LLC Flags (Status)
///  [15-13]  12     11     10      9     8      7      6      5      4      3      2      1      0
/// +......+------+------+------+------+------+------+------+------+------+------+------+------+------+
/// |  RFU |PHY EN|IN_PRO|LLCP_E|DISC_R|S_FND | U_EVT| U_HST| U_PE | TO_PE|LLCP_D|W_T_PA| P_VER| F_EXC|
/// +......+------+------+------+------+------+------+------+------+------+------+------+------+------+
enum llc_status_flag
{
    #if (BLE_2MBPS)
    /// - [12]:  Flag to indicate if the phy upd procedure is enabled
    LLC_STAT_PHY_ENABLED_MASK           = 0x1000,
    LLC_STAT_PHY_ENABLED_LSB            = 12,
    #endif // (BLE_2MBPS)
    /// - [11]:  Instant finished and proceed
    LLC_STAT_INSTANT_PROCEED_MASK       = 0x800,
    LLC_STAT_INSTANT_PROCEED_LSB        = 11,

    /// - [10]:  LLCP PDU Instant process in defer context
    LLC_STAT_LLCP_INSTANT_EXTRACTED_MASK= 0x400,
    LLC_STAT_LLCP_INSTANT_EXTRACTED_LSB = 10,

    /// - [9]:    Flag indicating if disconnection is requested by remote device
    LLC_STAT_DISC_REM_REQ_MASK          = 0x200,
    LLC_STAT_DISC_REM_REQ_LSB           = 9,

    /// - [8]:    Flag indicating if synchronization found
    LLC_STAT_SYNC_FOUND_MASK            = 0x100,
    LLC_STAT_SYNC_FOUND_LSB             = 8,

    /// - [7]:    Flag indicating if connection update event should be sent
    LLC_STAT_UPDATE_EVT_SENT_MASK       = 0x80,
    LLC_STAT_UPDATE_EVT_SENT_LSB        = 7,

    /// - [6]:    Flag indicating if connection update requested by host
    LLC_STAT_UPDATE_HOST_REQ_MASK       = 0x40,
    LLC_STAT_UPDATE_HOST_REQ_LSB        = 6,

    /// - [5]:    Flag indicating if connection update ongoing flag
    LLC_STAT_UPDATE_PENDING_MASK        = 0x20,
    LLC_STAT_UPDATE_PENDING_LSB         = 5,

    /// - [4]:    Flag indicating if connection TO is pending
    LLC_STAT_TO_PENDING_MASK            = 0x10,
    LLC_STAT_TO_PENDING_LSB             = 4,

    /// - [3]:    Flag indicating if LLC messages received have to be discarded
    LLC_STAT_LLCP_DISCARD_MASK          = 0x08,
    LLC_STAT_LLCP_DISCARD_LSB           = 3,

    /// - [2]:    Flag indicating if traffic has to be paused
    LLC_STAT_WAIT_TRAFFIC_PAUSED_MASK   = 0x04,
    LLC_STAT_WAIT_TRAFFIC_PAUSED_LSB    = 2,

    /// - [1]:    Flag indicating if the peer version is already known
    LLC_STAT_PEER_VERS_KNOWN_MASK       = 0x02,
    LLC_STAT_PEER_VERS_KNOWN_LSB        = 1,

    /// - [0]:    Flag indicating whether features have been exchanged or not
    LLC_STAT_FEAT_EXCH_MASK             = 0x01,
    LLC_STAT_FEAT_EXCH_LSB              = 0
};


/// Data Length Extension info
///   7  6  5  4  3  2  1   0
/// +--+--+--+--+--+--+---+---+
/// |      RFU        |EVT|REQ|
/// +--+--+--+--+--+--+---+---+
enum llc_dle_flag
{
    /// - [7:2]: RFU

    /// - [1]:    SE Private Address Renewal timer started
    LLC_DLE_EVT_SENT_MASK  = 0x02,
    LLC_DLE_EVT_SENT_LSB   = 1,

    /// - [0]:    RCVD/non RCVD LL_LENGTH_REQ
    LLC_DLE_REQ_RCVD_MASK  = 0x01,
    LLC_DLE_REQ_RCVD_LSB   = 0
};


#if (BLE_TESTER)
/// Tester flags
enum llc_tester_flag
{
    /// Ignore LLCP Feature request
    LLC_TESTER_IGNORE_FEAT_REQ     = 0x01,
    /// Force connection update parameters
    LLC_TESTER_FORCE_UP_PARAM      = 0x02,
    /// Surcharge LLCP connection parameter request
    LLC_TESTER_SURCHARGE_PARAM_REQ = 0x04,
};
#endif // (BLE_TESTER)





/// Remote version information structure
struct rem_version
{
    /// LMP version
    uint8_t vers;
    /// Manufacturer ID
    uint16_t compid;
    /// LMP subversion
    uint16_t subvers;
};

/// Encryption structure
struct encrypt
{
    /// Session key diversifier
    struct sess_k_div   skd;
    /// Long term key
    struct ltk          ltk;
    /// Random value
    uint8_t             randn[KEY_LEN];
};

/// Data Length Extension structure
struct data_len_ext_tag
{
    ///the maximum number of octets that the local device will send to the remote device
    uint16_t conn_max_tx_octets;
    ///the maximum number of octets that the local device is able to receive from the remote device
    uint16_t conn_max_rx_octets;

    ///the lesser of conn_max_tx_octets and conn_rem_max_rx_octets
    uint16_t conn_eff_max_tx_octets;
    ///the lesser of conn_max_rx_octets and conn_rem_max_tx_octets
    uint16_t conn_eff_max_rx_octets;

    ///the maximum number of microsecond that the local device will take to transmit to the remote
    uint16_t conn_max_tx_time;
    ///the maximum number of microsecond that the local device can take to receive from the remote device
    uint16_t conn_max_rx_time;

    ///the lesser of conn_max_tx_time and conn_rem_max_rx_time
    uint16_t conn_eff_max_tx_time;
    ///the lesser of conn_max_rx_time and conn_rem_max_tx_time
    uint16_t conn_eff_max_rx_time;

    ///Flag to check if the request is allowed or not TRUE = not allowed
    bool send_req_not_allowed;

    ///Flag to check if the request has been already done and the event sent
    uint8_t data_len_ext_flag;
};


/// LLC environment structure
struct llc_env_tag
{
    /// Request operation Kernel message
    void* operation[LLC_OP_MAX];
    /// Pointer to the associated @ref LLD event
    struct ea_elt_tag *elt;
    #if (BLE_CHNL_ASSESS)
    /// Channel Assessment structure environment
    struct llc_ch_asses_tag chnl_assess;
    #endif //(BLE_CHNL_ASSESS)
    /// Peer version obtained using the LL_VERSION_IND LLCP message
    struct rem_version  peer_version;
    /// Length extension
    struct data_len_ext_tag data_len_ext_info;
    /// Link supervision time out
    uint16_t            sup_to;
    /// New link supervision time out to be applied
    uint16_t            n_sup_to;
    /// Authenticated payload time out (expressed in units of 10 ms)
    uint16_t            auth_payl_to;
    /// Authenticated payload time out margin (expressed in units of 10 ms)
    uint16_t            auth_payl_to_margin;
    /// LLC status
    uint16_t            llc_status;
    ///Current channel map
    struct le_chnl_map  ch_map;
    ///New channel map - Will be applied at instant when a channel map update is pending
    struct le_chnl_map  n_ch_map;
    /// Received signal strength indication
    int8_t              rssi;
    /// Features used by the stack
    struct le_features  feats_used;
    /// Structure dedicated for the encryption
    struct encrypt      encrypt;
    /// Disconnection reason
    uint8_t             disc_reason;
    /// Disconnection event sent
    bool               disc_event_sent;
    /// Local procedure state
    uint8_t             loc_proc_state;
    /// Remote procedure state
    uint8_t             rem_proc_state;
    /// encryption state
    uint8_t             encryption_state;

    #if (BLE_TESTER)
    struct hci_tester_set_le_params_cmd tester_params;
    /// Use to know if LLCP pass through mechanism is enabled or not
    bool llcp_pass_through_enable;
    #endif
};

/*
 * DEFINES
 ****************************************************************************************
 */
/// Default values
#define LLC_DFT_INTERV          3200
#define LLC_DFT_LATENCY         500
#define LLC_DFT_SUP_TO          3200
#define LLC_DFT_RSP_TO          4000
#define LLC_DFT_AUTH_PAYL_TO    3000 // expressed in units of 10 ms, so 30 seconds
#define LLC_DFT_CE_LEN          65535
#define LLC_DFT_WIN_OFF         0
#define LLC_DFT_WIN_SIZE        8
#define LLC_DFT_HOP_INC         1
#define LLC_DFT_INST_MAP_UPD    6

/// Connection interval min (N*1.250ms)
#define LLC_CNX_INTERVAL_MIN            6       //(0x06)
/// Connection interval Max (N*1.250ms)
#define LLC_CNX_INTERVAL_MAX            3200    //(0xC80)
/// Connection latency min (N*cnx evt)
#define LLC_CNX_LATENCY_MIN             0       //(0x00)
/// Connection latency Max (N*cnx evt)
// "The connSlaveLatency parameter shall be less than 500" LL:4.5.1
#define LLC_CNX_LATENCY_MAX             499     //(0x1F3)
/// Supervision TO min (N*10ms)
#define LLC_CNX_SUP_TO_MIN              10      //(0x0A)
/// Supervision TO Max (N*10ms)
#define LLC_CNX_SUP_TO_MAX              3200    //(0xC80)
/// Connection event length min (N*0.625ms)
#define LLC_CNX_CE_LGTH_MIN             0       //(0x00)
/// Connection event length  Max (N*0.625ms)
#define LLC_CNX_CE_LGTH_MAX             65535   //(0xFFFF)

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct llc_env_tag* llc_env[BLE_CONNECTION_MAX];

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialization of the BLE LLC task
 *
 * This function initializes the LLC task, as well as assessment statistics
 *
 ****************************************************************************************
 */
void llc_init(void);

/**
 ****************************************************************************************
 * @brief Reset of the BLE LLC task
 *
 * This function reset the LLC task
 *
 ****************************************************************************************
 */
void llc_reset(void);

/**
 ****************************************************************************************
 * @brief Start the BLE LLC task
 *
 * This function set the state of the task, the initiating link supervision time out, the
 * feature used as well as the environment of the LLC and send the connection completed
 * event
 *
 * @param[in] param         Pointer on the structure which contains all the parameters
 *                          needed to create and maintain the link.
 * @param[in] conhdl        Connection handle on which the connection is created
 * @param[in] evt           Pointer to the event associated with this connection
 *
 ****************************************************************************************
 */
void llc_start(struct llc_create_con_req_ind *param, struct ea_elt_tag *elt);
/**
 ****************************************************************************************
 * @brief Stop the BLE LLC task
 *
 * This function clears the state of the task, the environment of the LLC and send the connection completed
 * event
 *
 * @param[in] conhdl        Connection handle on which the connection is created
 ****************************************************************************************
 */
void llc_stop(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the disconnection complete event
 *
 * This function notify the host that the disconnection happened
 *
 * @param[in] src_id        Source of the disconnection
 * @param[in] status        Status on the completion of the disconnection
 * @param[in] conhdl        Connection handle on which the disconnection happened
 * @param[in] reason        Why the disconnection happened
 ****************************************************************************************
 */
void llc_discon_event_complete_send(kernel_task_id_t src_id, uint8_t status, uint8_t conhdl, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Sends the LE connection complete event
 *
 * This function notify the host that the connection happened
 *
 * @param[in] status        Status on the completion of the connection
 * @param[in] conhdl        Connection handle on which the connection happened
 * @param[in] param         Pointer on the structure which contains all the parameters
 *                          needed to create and maintain the link.
 ****************************************************************************************
 */
void llc_le_con_cmp_evt_send(uint8_t status, uint16_t conhdl, struct llc_create_con_req_ind *param);

/**
 ****************************************************************************************
 * @brief Sends the update connection complete event.
 *
 * This function notify the host that the update of the connection's parameters happened.
 *
 * @param[in] status        Status on the completion of the update connection.
 * @param[in] conhdl        Connection handle on which the update connection happened.
 * @param[in] evt           Pointer to the event structure linked to the connection.
 ****************************************************************************************
 */
void llc_con_update_complete_send(uint8_t status, uint16_t conhdl, struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief Sends the command complete event.
 *
 * This function notify the host that the command is completed.
 *
 * @param[in] opcode        Command opcode
 * @param[in] status        Status on the completion of the command.
 * @param[in] conhdl        Connection handle on which the command has been processed.
 ****************************************************************************************
 */
void llc_common_cmd_complete_send(uint16_t opcode, uint8_t status, uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the command status event.
 *
 * This function notify the host that the command is understood.
 *
 * @param[in] opcode        Command opcode
 * @param[in] status        Status on the understanding of the command.
 * @param[in] conhdl        Connection handle on which the command has been processed.
 ****************************************************************************************
 */
void llc_common_cmd_status_send(uint16_t opcode, uint8_t status, uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the number of completed packet event.
 *
 * This function notify the host of the number of packets acknowledged
 *
 * @param[in] conhdl        Connection handle on which the packet has been acknowledged
 * @param[in] nb_of_pkt     Number of acknowledged packets
 ****************************************************************************************
 */
void llc_common_nb_of_pkt_comp_evt_send(uint16_t conhdl, uint8_t nb_of_pkt);

/**
 ****************************************************************************************
 * @brief Sends the read remote used features meta-event.
 *
 * @param[in] status        Status of the event
 * @param[in] conhdl        Connection handle on which the remote features have been read
 * @param[in] feats         Read remote features
 *
 ****************************************************************************************
 */
void llc_feats_rd_event_send(uint8_t status,
                              uint16_t conhdl,
                              struct le_features const *feats);

/**
 ****************************************************************************************
 * @brief Sends the remote version indication event.
 *
 * @param[in] status        Status of the event
 * @param[in] conhdl        Connection handle on which the remote version have been read
 *
 ****************************************************************************************
 */
void llc_version_rd_event_send(uint8_t status, uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the encryption change event.
 *
 * This function notify the host of the new encryption status.
 *
 * @param[in] conhdl        Connection handle on which the status of the encryption has
 *                          been changed
 * @param[in] enc_status    Status of the encryption (ON or OFF)
 * @param[in] status        Inform if the change is successfully done or not.
 ****************************************************************************************
 */
void llc_common_enc_change_evt_send(uint16_t conhdl, uint8_t enc_status, uint8_t status);

/**
 ****************************************************************************************
 * @brief Sends the flush occurred event.
 *
 * This function notify the host that a flush of packet occurred.
 *
 * @param[in] conhdl        Connection handle on which the flush occurred.
 ****************************************************************************************
 */
void llc_common_flush_occurred_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the encryption key refresh completed event.
 *
 * This function notify the host that the resume is done for the encryption.
 *
 * @param[in] conhdl        Connection handle on which the key has been refreshed.
 ****************************************************************************************
 */
void llc_common_enc_key_ref_comp_evt_send(uint16_t conhdl, uint8_t status);

/**
 ****************************************************************************************
 * @brief Sends the long term key request.
 *
 * This function request to the host for a LTK.
 *
 * @param[in] conhdl        Connection handle on which the LTK is requested.
 * @param[in] param         Pointer to the parameters of the LL_ENC_REQ
 ****************************************************************************************
 */
void llc_ltk_req_send(uint16_t conhdl, struct llcp_enc_req const *param);

/**
 ****************************************************************************************
 * @brief Indicates that the parameter update has occurred
 *
 * @param[in] conhdl        Connection handle on which the update occurred.
 * @param[in] evt_new       Pointer to the new LLD event that is used for this connection.
 ****************************************************************************************
 */
void llc_con_update_ind(uint16_t conhdl, struct ea_elt_tag *elt_new);

/**
 ****************************************************************************************
 * @brief Indicates that the channel map update should be done.
 *
 * @param[in] conhdl        Connection handle on which the update occurred.
 ****************************************************************************************
 */
void llc_map_update_ind(uint16_t conhdl);


/**
 ****************************************************************************************
 * @brief Finish the connection update procedure
 *
 * @param[in] conhdl        Connection handle
 ****************************************************************************************
 */
void llc_con_update_finished(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Finish the channel map update procedure
 *
 * @param[in] conhdl        Connection handle
 ****************************************************************************************
 */
void llc_map_update_finished(uint16_t conhdl);


#if(BLE_2MBPS)
/**
 ****************************************************************************************
 * @brief Finish the phys update procedure and check if the host event should be sent
 *
 * @param[in] conhdl        Connection handle
 * @param[in] tx_phy        Transmitter phy selected
 * @param[in] rx_phy        Received phy selected
 * @param[in] status        reason of the end
 * @param[in] operation     current operation: Local, hci or peer request
 *
 ****************************************************************************************
 */
void llc_phy_update_finished(uint16_t conhdl, uint8_t tx_phy, uint8_t rx_phy, uint8_t status, uint8_t operation);


/**
 ****************************************************************************************
 * @brief send the HCI message to inform that PHY update is completed
 *
 * @param[in] status        reason of the end
 * @param[in] conhdl        Connection handle
 * @param[in] tx_phy        Transmitter phy selected
 * @param[in] rx_phy        Received phy selected
 *
 ****************************************************************************************
 */
void llc_phy_update_complete_send(uint8_t status, uint16_t conhdl , uint8_t tx_phy, uint8_t rx_phy);
#endif // (BLE_2MBPS)
/**
 ****************************************************************************************
 * @brief Before the connection update instant used the greater LSTO
 *
 * @param[in] conhdl        Connection handle
 ****************************************************************************************
 */
void llc_lsto_con_update(uint16_t conhdl);

#endif // #if (BLE_PERIPHERAL || BLE_CENTRAL)
/// @} LLC

#endif // LLC_H_
