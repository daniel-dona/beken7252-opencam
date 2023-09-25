/**
 ****************************************************************************************
 *
 * @file ancsc_task.h
 *
 * @brief Header file - ANCSCTASK.
 *
 * Copyright (C) BeKen 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef ANCSC_TASK_H_
#define ANCSC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup ANCSCTASK ANCS  Client Task
 * @ingroup ANCSC
 * @brief ANCS Client Task
 *
 * The ANCSCTASK is responsible for handling the API messages coming from the application
 * or internal tasks.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#if (BLE_ANCS_CLIENT)
#include "kernel_task.h"
#include "common_error.h"
#include "ancsc.h"
#include "ancs_common.h"
#include "prf_types.h"
#include "prf_utils.h"
#include "ble_compiler.h"      // compiler definitions





/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of Ancs Client task instances
#define ANCSC_IDX_MAX    (BLE_CONNECTION_MAX)

/// Possible states of the ANCSC task
enum ancsc_state
{
    /// Not Connected State
    ANCSC_FREE,
    /// Idle state
    ANCSC_IDLE,
	
	  ANCSC_DISCOVER,
	
	  ANCSC_REGISTER,
	
		ANCSC_ENABLE_NTF,
    /// Busy State
    ANCSC_BUSY,

    /// Number of defined states.
    ANCSC_STATE_MAX
};

///ANCS Client API messages
enum ancsc_msg_id
{
    /// Start the find me locator profile - at connection
    ANCSC_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_ANCSC),
    /// Enable confirm message, containing ANCS attribute details if discovery connection type
    ANCSC_ENABLE_RSP,

	    /// Write  Notification Source Notification Configuration Value request
    ANCSC_CFG_NTF_SRC_INDNTF_REQ,
    /// Write   Notification Source  Notification Configuration Value response
    ANCSC_CFG_NTF_SRC_INDNTF_RSP,
	
		/// Write  Data Source Notification Configuration Value request
    ANCSC_CFG_DATA_SRC_INDNTF_REQ,
    /// Write   Data Source  Notification Configuration Value response
    ANCSC_CFG_DATA_SRC_INDNTF_RSP,
	
    /// Write control point req
    ANCSC_WR_CNTL_POINT_REQ,
	 /// Write control point rsp
	  ANCSC_WR_CNTL_POINT_RSP,
	
	    /// Notification source ind
    ANCSC_NOTIFICATION_IND,
    ///  data source ind
    ANCSC_DATA_SOURCE_IND,
};


/// Parameters of the @ref ANCSC_ENABLE_REQ message
struct ancsc_enable_req
{
		uint16_t conhdl;
    ///Connection type
    uint8_t con_type;
    ///Discovered ANCS details if any
    struct ancsc_content ancsc;
};

/// Parameters of the @ref ANCSC_ENABLE_RSP message
struct ancsc_enable_rsp
{
    ///Status
    uint8_t status;
    ///Discovered ANCS details if any
    struct ancsc_content ancsc;
};



///Parameters of the @ref ANCSC_CFG_NTF_SRC_INDNTF_REQ message
struct ancsc_ntf_src_ntf_cfg_req
{
    ///Notification Configuration
    uint16_t ntf_cfg;
};


///Parameters of the @ref ANCSC_CFG_NTF_SRC_INDNTF_RSP message
struct ancsc_ntf_src_ntf_cfg_rsp
{
    /// Status
    uint8_t  status;
};

///Parameters of the @ref ANCSC_CFG_DATA_SRC_INDNTF_REQ message
struct ancsc_data_src_ntf_cfg_req
{
    ///Notification Configuration
    uint16_t ntf_cfg;
};


///Parameters of the @ref ANCSC_CFG_DATA_SRC_INDNTF_RSP message
struct ancsc_data_src_ntf_cfg_rsp
{
    /// Status
    uint8_t  status;
};



///Parameters of the @ref ANCSC_WR_CNTL_POINT_REQ message




/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern const struct kernel_state_handler ancsc_default_handler;

#endif //BLE_ANCS_CLIENT

/// @} ANCSCTASK

#endif // ANCSC_TASK_H_
