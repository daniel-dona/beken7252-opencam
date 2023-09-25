/**
 ****************************************************************************************
 *
 * @file ms_task.h
 *
 * @brief Header file - MS Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2018
 *
 *
 ****************************************************************************************
 */


#ifndef _MS_TASK_H_
#define _MS_TASK_H_


#include "rwprf_config.h"
#if (BLE_MS_SERVER)
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of MS Server task instances
#define MS_IDX_MAX     0x01
///Maximal number of MS that can be added in the DB

#define  MS_REVICE_DATA_LEN  20
#define  MS_SEND_DATA_LEN  20
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Possible states of the MS task
enum ms_state
{
    /// Idle state
    MS_IDLE,
    /// busy state
    MS_BUSY,
    /// Number of defined states.
    MS_STATE_MAX
};

/// Messages for FFF0 Server
enum ms_msg_id
{
	/// Start the FFE0 Server - at connection used to restore bond data
	MS_ENABLE_REQ  =  TASK_FIRST_MSG(TASK_ID_MS),

	/// Confirmation of the MS Server start
	MS_ENABLE_RSP,

	MS_REVICE_REQ_IND,
	///  Level Value Update Request
	MS_SEND_VALUE_UPD_REQ,
	/// Inform APP if  Level value has been notified or not
	MS_SEND_VALUE_UPD_RSP,
	/// Inform APP that  send l Notification Configuration has been changed - use to update bond data
	MS_SEND_VALUE_IND_CFG_IND,

	MS_GATTC_CMP_EVT
};

/// Features Flag Masks
enum ms_features
{
    /// FFE1 Level Characteristic doesn't support notifications
    MS_SEND_LVL_IND_NOT_SUP,
    /// FFE1 Level Characteristic support notifications
    MS_SEND_LVL_IND_SUP = 2,
};
/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct ms_db_cfg
{
    /// Number of FFF0 to add
	uint8_t ms_nb;

	uint16_t cfg_flag;
	/// Features of each FFF0 instance
	uint16_t features;
};

/// Parameters of the @ref  message
struct ms_enable_req
{
    /// connection index
    uint8_t  conidx;
    ///  Configuration
    uint16_t  ind_cfg;
    
};


/// Parameters of the @ref  message
struct ms_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref  message
struct ms_send_value_upd_req
{
    /// BAS instance
    uint8_t conidx;
	
	uint8_t length;
	
	uint16_t seq_num;
    ///  Level
    uint8_t value[__ARRAY_EMPTY];
};

///Parameters of the @ref  message
struct ms_send_value_upd_rsp
{
	 uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref  message
struct ms_send_value_ind_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint16_t  ind_cfg;
};


/// Parameters of the @ref FFF0S_FFF2_WRITER_REQ_IND message
struct ms_revice_ind
{
   
    uint8_t conidx; /// Connection index
		uint8_t length;
	  uint8_t value[__ARRAY_EMPTY];
};






/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct kernel_state_handler ms_default_handler;
#endif // BLE_MS_SERVER


#endif /* _MS_TASK_H_ */

