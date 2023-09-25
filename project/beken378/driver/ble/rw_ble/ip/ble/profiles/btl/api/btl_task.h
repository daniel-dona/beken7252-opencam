/**
 ****************************************************************************************
 *
 * @file btl_task.h
 *
 * @brief Header file - BTL Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2018
 *
 *
 ****************************************************************************************
 */


#ifndef _BTL_TASK_H_
#define _BTL_TASK_H_


#include "rwprf_config.h"
#if (BLE_BTL_SERVER)
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of BTL Server task instances
#define BTL_IDX_MAX     0x01
///Maximal number of BTL that can be added in the DB

#define  BTL_CHAR_DATA_LEN  128

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Possible states of the MS task
enum btl_state
{
    /// Idle state
    BTL_IDLE,
    /// busy state
    BTL_BUSY,
    /// Number of defined states.
    BTL_STATE_MAX
};

/// Messages for FFF0 Server
enum btl_msg_id
{
	/// Start the FFE0 Server - at connection used to restore bond data
	BTL_ENABLE_REQ  =  TASK_FIRST_MSG(TASK_ID_BTL),

	/// Confirmation of the MS Server start
	BTL_ENABLE_RSP,

	BTL_FF01_REQ_IND,
	///  Level Value Update Request
	BTL_FF02_VALUE_UPD_REQ,
	/// Inform APP if  Level value has been notified or not
	BTL_FF02_VALUE_UPD_RSP,
	/// Inform APP that  send l Notification Configuration has been changed - use to update bond data
	BTL_FF02_VALUE_IND_CFG_IND,

	BTL_GATTC_CMP_EVT
};

/// Features Flag Masks
enum btl_features
{
    /// FF02 Level Characteristic doesn't support notifications
    BTL_FF02_LVL_IND_NOT_SUP,
    /// FF02 Level Characteristic support notifications
    BTL_FF02_LVL_IND_SUP = 2,
};
/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct btl_db_cfg
{
    /// Number of FFFF to add
	uint8_t btl_nb;

	uint16_t cfg_flag;
	/// Features of each FFFF instance
	uint16_t features;
};

/// Parameters of the @ref  message
struct btl_enable_req
{
    /// connection index
    uint8_t  conidx;
    ///  Configuration
    uint16_t  ind_cfg;
    
};


/// Parameters of the @ref  message
struct btl_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref  message
struct btl_ff02_value_upd_req
{
    /// BAS instance
    uint8_t conidx;
	
	uint8_t length;
	
	uint16_t seq_num;
    ///  Level
    uint8_t value[__ARRAY_EMPTY];
};

///Parameters of the @ref  message
struct btl_ff02_value_upd_rsp
{
	 uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref  message
struct btl_ff02_value_ind_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint16_t  ind_cfg;
};


/// Parameters of the @ref FFF0S_FFF2_WRITER_REQ_IND message
struct btl_ff01_ind
{
   
    uint8_t conidx; /// Connection index
		uint8_t length;
	  uint8_t value[__ARRAY_EMPTY];
};






/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct kernel_state_handler btl_default_handler;
#endif // BLE_BTL_SERVER


#endif /* _BTL_TASK_H_ */

