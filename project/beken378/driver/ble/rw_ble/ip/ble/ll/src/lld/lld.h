/**
 ****************************************************************************************
 *
 * @file lld.h
 *
 * @brief Main API file for the Link Layer Driver
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef LLD_H_
#define LLD_H_

/**
 ****************************************************************************************
 * @addtogroup LLD LLD
 * @ingroup CONTROLLER
 * @brief Link Layer Driver
 *
 * The LLD is responsible for programming the HW for event scheduling. It provides an API
 * to the @ref LLD and the @ref LLC for event programming (@ref LLDEVT)and data
 * transmission/reception (@ref LLDDATA). This API takes two forms depending on the
 * direction:
 *      - LLM/LLC->LLD: Functions
 *      - LLD->LLM/LLC: Messages
 *
 * A typical use case of the LLD API:

 * @msc
 * a [label=LLC],
 * b [label=LLD];
 *
 * ---   [label="Data transmission example"];
 * a=>b  [label="lld_data_tx_push(evt, txdesc)", URL="\ref lld_data_tx_push()"];
 * b->b  [label="Data transmission to peer device"];
 * b->a  [label="lld_pdu_ind", URL="\ref lld_pdu_ind"];
 * @endmsc
 *
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

#include "common_list.h"
#include "common_llcp.h"
#include "kernel_task.h"
#include "rwip_config.h"
#include "rwip_task.h"
#include "llm.h"
#include "llc_llcp.h"
#include "reg_blecore.h"
#include "lld_evt.h"

// Forward declarations
struct llm_le_create_con_cmd;

/*
 * DEFINES
 ****************************************************************************************
 */
/// Reserved Handle for advertising traffic
#define LLD_ADV_HDL         BLE_CONNECTION_MAX

/// RX interrupt threshold
#define LLD_RX_IRQ_THRES   (BLE_RX_BUFFER_CNT/4) //(BLE_RX_BUFFER_CNT/4)  //modify 2017/08/31 alen

/// Basic rate and Enhanced Data Rate
#define MOD_GFSK            0x01

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message identifiers
enum
{
    /// Message confirming the event deletion requested by the upper layers
    LLD_STOP_IND = TASK_FIRST_MSG(TASK_ID_LLD),
};

/*
 * MESSAGE PARAMETER STRUCTURES
 ****************************************************************************************
 */



/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the BLE HW and event scheduler
 *
 * This function initializes the BLE HW core, as well as the environment of the LLD
 *
 * @param[in] reset  true if it's requested by a reset; false if it's boot initialization
 ****************************************************************************************
 */
void lld_init(bool reset);

/**
 ****************************************************************************************
 * @brief Reset the BLE HW
 *
 * This function stops any pending activity of the BLE core, resets it and disables it. It
 * is called upon reception of a HCI reset command.
 *
 ****************************************************************************************
 */
void lld_core_reset(void);

/**
 ****************************************************************************************
 * @brief Start advertising
 *
 * This function creates an event for advertising, set the control structure according
 * to the parameters, and program the required data for transmission.
 *
 * @param[in] adv_par        Pointer to the structure containing the advertising
 *                           parameters
 * @param[in] adv_pdu        Pointer to the buffer containing the advertising data
 * @param[in] scan_rsp_pdu   Pointer to the buffer containing the scan response data
 *                           (should be set to NULL is no scan response has to be
 *                            transmitted)
 * @param[in] adv_pwr        Transmission power for the advertising data
 *
 * @return The event pointer associated to the advertising
 *
 ****************************************************************************************
 */
struct ea_elt_tag *lld_adv_start(struct advertising_pdu_params *adv_par,
                                 struct em_desc_node *adv_pdu,
                                 struct em_desc_node *scan_rsp_pdu,
                                 uint8_t adv_pwr);
/**
 ****************************************************************************************
 * @brief Start scanning
 *
 * This function creates an event for scanning, set the control structure according
 * to the parameters, and program the required data for transmission.
 *
 * @param[in] param          Pointer to the structure containing the scanning parameters
 * @param[in] scan_req_pdu   Pointer to the buffer containing the scan request data
 *                           (should be set to NULL if passive scanning is required)
 *
 * @return The event pointer associated to the scanning
 *
 ****************************************************************************************
 */
struct ea_elt_tag *lld_scan_start(struct scanning_pdu_params const *scan_par,
                                  struct em_desc_node *scan_req_pdu);

#if (BLE_CENTRAL)
/**
 ****************************************************************************************
 * @brief Initiates a connection
 *
 * @param[in] con_par      Connection parameters as requested by the host
 * @param[in] con_req_pdu  Pointer to the data buffer containing the connection request
 * @param[in] conhdl       Connection handle on which connection is initiated
 *
 * @return The event pointer associated to the started connection
 *
 ****************************************************************************************
 */
struct ea_elt_tag* lld_con_start(struct hci_le_create_con_cmd const *con_par,
                                 struct em_desc_node *con_req_pdu,
                                 uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Indicates to LLD that it can program the first connection event for this handle
 *
 * This function is called by the @ref LLM once it receives the confirmation that the
 * connection request has been transmitted to the future slave. The LLD then programs the
 * first connection event for this slave.
 * @note The @ref lld_con_start function has to be called previously to start the
 * initiating process, otherwise lld_move_to_master has unpredictable behavior.
 *
 * @param[in] evt     The pointer to the event on which the connection will occur
 * @param[in] conhdl  Connection handle on which connection is initiated
 * @param[in] rx_hdl  Handle of the rx descriptor where the ADV has been detected
 *
 ****************************************************************************************
 */
struct ea_elt_tag* lld_move_to_master(struct ea_elt_tag *elt, uint16_t conhdl, struct llc_create_con_req_ind const *param, uint8_t rx_hdl);
#endif //(BLE_CENTRAL)

#if (BLE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief Indicates to LLD that it can program the first slave connection event
 *
 * This function is called by the @ref LLM once it receives a CONNECT_REQ. The LLD then
 * programs the first connection event according to the parameters received in the
 * CONNECT_REQ frame.
 *
 * @param[in] con_par       Parameters of the connection
 * @param[in] con_req_pdu   Buffer containing the CONNECT_REQ frame
 * @param[in] evt_adv       Event that was used for the advertising
 * @param[in] conhdl        Connection handle on which connection is started
 *
 ****************************************************************************************
 */
struct ea_elt_tag* lld_move_to_slave(struct llc_create_con_req_ind *con_par,
                                     struct llm_pdu_con_req_rx *con_req_pdu,
                                     struct ea_elt_tag *elt_adv,
                                     uint16_t conhdl);
#endif //(BLE_PERIPHERAL)

/**
 ****************************************************************************************
 * @brief Stop the activity of a scanning event
 *
 * This function flushes all the TX/RX data for this event, and unprogram the
 * event
 *
 * @param[in] evt  The pointer to the event to be stopped
 *
 ****************************************************************************************
 */
void lld_scan_stop(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Stop the activity of an advertising event
 *
 * This function flushes all the TX/RX data for this event, and unprogram the
 * event
 *
 * @param[in] evt  The pointer to the event to be stopped
 *
 ****************************************************************************************
 */
void lld_adv_stop(struct ea_elt_tag *evt);

/**
 ****************************************************************************************
 * @brief Stop the activity of a connection event
 *
 * This function flushes all the TX/RX data for this event, and unprogram the
 * event
 *
 * @param[in] evt  The pointer to the event to be stopped
 *
 ****************************************************************************************
 */
void lld_con_stop(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Stop the activity of a RF test mode event
 *
 * This function flushes all the TX/RX data for this event, and unprogram the
 * event
 *
 * @param[in] evt  The pointer to the event to be stopped
 *
 ****************************************************************************************
 */
void lld_test_stop(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Return the type of a provided connection (Master, Slave, Broadcaster, Advertiser)
 *
 * @param[in] conhdl  Connection Handle
 *
 * @return A mode contained in enum lld_evt_mode; if an error occurs mode is LLD_EVT_MODE_MAX
 *
 ****************************************************************************************
 */
uint8_t lld_get_mode(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Interrupt service routine for the end of encryption IRQ
 *
 ****************************************************************************************
 */
void lld_crypt_isr(void);

#if (BLE_CENTRAL || BLE_PERIPHERAL)

/**
 ****************************************************************************************
 * @brief Change map indication
 *
 * This function indicates to the slave that to channel map update request has been
 * received from the master, and at which time it will occur.
 *
 * @param[in] evt     The pointer to the event on which the channel map has to be changed
 * @param[in] instant The connection event count on which the update has to be applied
 *
 ****************************************************************************************
 */
void lld_ch_map_ind(struct ea_elt_tag *elt, uint16_t instant);

/**
 ****************************************************************************************
 * @brief Update connection parameters (in Master mode)
 *
 * This function request to change the current connection parameters.
 *
 * @param[in] evt_old    The event used prior to connection update
 * @param[in] param      Pointer on the structure which contains the latency,
 *                       interval, window offset and size, timeout and the instant
 * @param[out] param_pdu Pointer on the structure which contains the latency,
 *                       interval, window offset and size, timeout and the instant
 *                       dedicated to create the pdu
 *
 ****************************************************************************************
 */
void lld_con_update_req(struct ea_elt_tag *elt_old,
                        struct llc_con_upd_req_ind *param,
                        struct llcp_con_upd_ind *param_pdu);

/**
 ****************************************************************************************
 * @brief Update connection parameters after a param_req from peer(in Master mode)
 *
 * This function request to change the current connection parameters.
 *
 * @param[in] evt_old    The event used prior to connection update
 * @param[in] param      Pointer on the structure which contains the latency,
 *                       interval, window offset and size, timeout and the instant
 * @param[out] param_pdu Pointer on the structure which contains the latency,
 *                       interval, window offset and size, timeout and the instant
 *                       dedicated to create the pdu
 * @param[in] bypass_offchk  Bypass the check of the offset in case
 *
 ****************************************************************************************
 */
uint8_t lld_con_update_after_param_req(uint16_t conhdl,
                                       struct ea_elt_tag *elt_old,
                                       struct llc_con_upd_req_ind *param,
                                       struct llcp_con_upd_ind *param_pdu,
                                       bool bypass_offchk);

/**
 ****************************************************************************************
 * @brief Respond to a param_req from peer(in Slave mode)
 *
 * This function checks if the parameters proposed by the peer (master) can be accepted.
 *
 * @param[in] conhdl     Connection handle
 * @param[in] evt        The event used
 * @param[in|out] param  Pointer to the param_rsp to be sent to the peer
 *
 ****************************************************************************************
 */
uint8_t lld_con_param_rsp(uint16_t conhdl,
                          struct ea_elt_tag *elt,
                          struct llc_con_upd_req_ind *param);

/**
 ****************************************************************************************
 * @brief Send a param_req to peer(in Slave mode)
 *
 * This function checks if the parameters proposed by the host can be accepted.
 *
 * @param[in] evt        The event used
 * @param[in|out] param  Pointer to the parameters to update for the connection
 *
 ****************************************************************************************
 */
void lld_con_param_req(uint16_t conhdl, struct ea_elt_tag *elt, struct llc_con_upd_req_ind *param);

/**
 ****************************************************************************************
 * @brief Update connection parameters (in Slave mode)
 *
 * This function request to change the current connection parameters.
 *
 * @param[in] evt_old    The event used prior to connection update
 * @param[in] param_pdu  Pointer on the structure which contains the latency,
 *                       interval, window offset and size, timeout and the instant
 *                       received from the connection update pdu
 ****************************************************************************************
 */
void lld_con_update_ind(struct ea_elt_tag *elt_old,
                        struct llcp_con_upd_ind const *param_pdu);

#endif //(BLE_CENTRAL || BLE_PERIPHERAL)

/**
 ****************************************************************************************
 * @brief Start the transmit test mode
 *
 * This function request to the HW to start the transmit test mode, by setting the format
 * synchronization word (0x94826E8E), CRC(0x555555) and the transmit frequency in the CS
 * and the address of the tx descriptor.
 *
 * @param[in] enh_cmd       True if it is the enhanced command
 * @param[in] txdesc        Pointer on the TX descriptor containing the length, type and payload.
 * @param[in] tx_freq       Value of the Tx frequency
 * @param[in] phy           Value of the phy
 *
 * @return The event pointer associated to the tx test mode
 *
 ****************************************************************************************
 */
struct ea_elt_tag *lld_test_mode_tx(bool enh_cmd, struct em_desc_node *txdesc, uint8_t tx_freq, uint8_t phy);

/**
 ****************************************************************************************
 * @brief Start the receive test mode
 *
 * This function request to the HW to start the transmit test mode, by setting the format
 * and the receive frequency in the CS.
 *
 * @param[in] enh_cmd       True if it is the enhanced command
 * @param[in] rx_freq       Value of the rx frequency
 * @param[in] phy           Value of the phy
 * @param[in] modul_idx     Value of the modulation index
 *
 * @return The event pointer associated to the rx test mode
 *
 ****************************************************************************************
 */
struct ea_elt_tag *lld_test_mode_rx(bool enh_cmd, uint8_t rx_freq, uint8_t phy, uint8_t modul_idx);

/**
 ****************************************************************************************
 * @brief Request renewal of RPA addresses
 *
 * @return The event pointer associated to the ADV/SCAN/INIT mode
 *
 ****************************************************************************************
 */
void lld_ral_renew_req(struct ea_elt_tag *elt);

/// @} LLD

#endif // LLD_H_
