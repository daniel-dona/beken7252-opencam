/**
 ****************************************************************************************
 *
 * @file fff0s_task.h
 *
 * @brief Header file - Battery Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */


#ifndef _FFE0S_TASK_H_
#define _FFE0S_TASK_H_


#include "rwprf_config.h"
#if  (BLE_FFE0_SERVER)
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of FFF0 Server task instances
#define FFE0S_IDX_MAX     0x01
///Maximal number of FFF0 that can be added in the DB

#define  FFE0_FFE1_DATA_LEN  20
#define  FFE0_FFE2_DATA_LEN  20
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Possible states of the FFF0S task
enum ffe0s_state
{
    /// Idle state
    FFE0S_IDLE,
    /// busy state
    FFE0S_BUSY,
    /// Number of defined states.
    FFE0S_STATE_MAX
};

/// Messages for FFF0 Server
enum ffe0s_msg_id
{
	/// Start the FFE0 Server - at connection used to restore bond data
	FFE0S_ENABLE_REQ  =  TASK_FIRST_MSG(TASK_ID_FFE0S),

	/// Confirmation of the FFF0 Server start
	FFE0S_ENABLE_RSP,
	/// FFF1 Level Value Update Request
	FFE0S_FFE1_VALUE_UPD_REQ,
	/// Inform APP if FFE1 Level value has been notified or not
	FFE0S_FFE1_VALUE_UPD_RSP,
	/// Inform APP that FFE1 Level Notification Configuration has been changed - use to update bond data
	FFE0S_FFE1_VALUE_NTF_CFG_IND,

	FFE0S_FFE1_WRITER_REQ_IND,

	FFE0S_FFE1_READ_REQ_IND,
	FFE0S_FFE1_READ_RSP,

	FFE0S_FFE2_WRITER_REQ_IND,

	FFE0S_GATTC_CMP_EVT
};

/// Features Flag Masks
enum ffe0s_features
{
    /// FFE1 Level Characteristic doesn't support notifications
    FFE0_FFE1_LVL_NTF_NOT_SUP,
    /// FFE1 Level Characteristic support notifications
    FFE0_FFE1_LVL_NTF_SUP,
};
/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct ffe0s_db_cfg
{
    /// Number of FFF0 to add
	uint8_t ffe0_nb;

	uint8_t cfg_flag;
	/// Features of each FFF0 instance
	uint8_t features;
};

/// Parameters of the @ref FFE0S_ENABLE_REQ message
struct ffe0s_enable_req
{
    /// connection index
    uint8_t  conidx;
    /// Notification Configuration
    uint8_t  ntf_cfg;
    
};


/// Parameters of the @ref FFE0S_ENABLE_RSP message
struct ffe0s_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref FFE0S_FFE1_VALUE_UPD_REQ message
struct ffe0s_ffe1_value_upd_req
{
    /// BAS instance
    uint8_t conidx;
	
	uint8_t length;
	
	uint16_t seq_num;
    /// ffe1 Level
    uint8_t value[__ARRAY_EMPTY];
};

///Parameters of the @ref FFE0S_FFE1_VALUE_UPD_RSP message
struct ffe0s_ffe1_value_upd_rsp
{
	 uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref FFE0S_FFE1_VALUE_NTF_CFG_IND message
struct ffe0s_ffe1_value_ntf_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint8_t  ntf_cfg;
};


/// Parameters of the @ref FFF0S_FFF2_WRITER_REQ_IND message
struct ffe0s_ffe2_writer_ind
{
   
    uint8_t conidx; /// Connection index
		uint8_t length;
	  uint8_t value[__ARRAY_EMPTY];
};


struct ffe0s_ffe1_writer_ind
{
   
    uint8_t conidx; /// Connection index
	uint8_t length;
	uint8_t value[__ARRAY_EMPTY];
};

struct ffe0s_ffe1_read_req_ind
{
   
    uint8_t conidx; /// Connection index
	uint16_t handle;
	uint16_t rsp_id;	
};

struct ffe0s_ffe1_read_rsp
{
   
    uint8_t conidx; /// Connection index
	uint8_t length;
	uint16_t handle;
	uint16_t rsp_id;
	uint8_t value[__ARRAY_EMPTY];
};
	

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct kernel_state_handler ffe0s_default_handler;
#endif // BLE_FFF0_SERVER


#endif /* _FFE0S_TASK_H_ */

