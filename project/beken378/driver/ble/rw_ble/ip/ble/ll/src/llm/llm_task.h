/**
 ****************************************************************************************
 *
 * @file llm_task.h
 *
 * @brief LLM task header file
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */

#ifndef LLM_TASK_H_
#define LLM_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup LLMTASK LLMTASK
 * @ingroup LLM
 * @brief Link Layer Manager Task
 *
 * The LLM task is responsible for managing link layer actions not related to a
 * specific connection with a peer (e.g. scanning, advertising, etc.). It implements the
 * state machine related to these actions.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h" // stack configuration
#include <stdint.h>

#include "common_bt.h"
#include "kernel_task.h"     // Task informations
#include "rwip_task.h"   // Task definitions

/*
 * INSTANCES
 ****************************************************************************************
 */
/// Maximum number of instances of the LLM task
#define LLM_IDX_MAX 1

/// LE events subcodes
#define LLM_LE_EVT_ADV_REPORT       0x02
/*
 * STATES
 ****************************************************************************************
 */
/// Possible states of the LLM task
enum llm_state_id
{
    /// IDLE state
    LLM_IDLE,
    /// ADVERTISING state
    LLM_ADVERTISING,
    /// SCANNING state
    LLM_SCANNING,
    /// INITIATING state
    LLM_INITIATING,
    /// STOPPING state
    LLM_STOPPING,
    /// TEST states.
    LLM_TEST,
    /// Number of states.
    LLM_STATE_MAX
};

/*
 * MESSAGES
 ****************************************************************************************
 */
/// Message API of the LLM task
enum llm_msg_id
{
    /*
     * ************** Msg LLM->LLM****************
     */
    /// Time out message
    /// to authorize the reception of the cmd
    LLM_LE_SET_HOST_CH_CLASS_CMD_STO = TASK_FIRST_MSG(TASK_ID_LLM),
    /// Request to initialize LLM
    LLM_STOP_IND,

    /*
     * ************** Msg LLM->LLC****************
     */
    /// request changing map
    LLM_LE_SET_HOST_CH_CLASS_REQ,
    /// channel map req indication
    LLM_LE_SET_HOST_CH_CLASS_REQ_IND,
    /*
     * ************** Msg LLC->LLM****************
     */
    /// link layer disconnection indication
    LLM_ENC_REQ,
    LLM_ENC_IND,

    #if SECURE_CONNECTIONS
    #if (BLE_CENTRAL || BLE_PERIPHERAL)
    /*
     * ************** Msg ECC->LLM****************
     */
    LLM_ECC_RESULT_IND,
    #endif //(BLE_CENTRAL || BLE_PERIPHERAL)
    #endif //SECURE_CONNECTIONS

    /*
     * ************** Channel Assessment ********
     */
    /// Channel Assessment Timer
    LLM_LE_CHNL_ASSESS_TIMER,
    /// Channel Assessment - Generate Channel classification
    LLM_GEN_CHNL_CLS_CMD,

    /*
     * **** Privacy 1.2 - Controller Privacy ****
     */
    /// Enhanced privacy timer used to renew Local Resolving Private Addresses
    LLM_LE_ENH_PRIV_ADDR_RENEW_TIMER,
};

///LLC Encrypt Request parameters structure
struct llm_enc_req
{
    ///Long term key structure
    struct ltk     key;
    ///Pointer to buffer with plain data to encrypt - 16 bytes
    uint8_t        plain_data[16];
};

///LLM LE Encrypt indication structure
struct llm_enc_ind
{
    /// Status of the command reception
    uint8_t status;
    ///Encrypted data to return to command source.
    uint8_t encrypted_data[ENC_DATA_LEN];
};

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */
extern const struct kernel_state_handler llm_default_handler;
extern kernel_state_t llm_state[LLM_IDX_MAX];

/// @} LLMTASK

#endif // LLM_TASK_H_
