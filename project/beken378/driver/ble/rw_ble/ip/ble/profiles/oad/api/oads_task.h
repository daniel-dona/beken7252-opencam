/**
 ****************************************************************************************
 *
 * @file otas_task.h
 *
 * @brief Header file - ota profile Server Role Task.
 *
 * Copyright (C) BeKen 2009-2015
 *
 *
 ****************************************************************************************
 */


#ifndef _OTAS_TASK_H_
#define _OTAS_TASK_H_


#include "rwprf_config.h"
#if (BLE_OADS_SERVER)
#include <stdint.h>
#include "prf_types.h"

#include "rwip_task.h" // Task definitions
//#include "kernel_task.h"


/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of  Server task instances
#define OADS_IDX_MAX     0x01
///Maximal number of BRACES that can be added in the DB



#define  OADS_FFC1_DATA_LEN  25
#define  OADS_FFC2_DATA_LEN  25



/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
 

/// Possible states of the braces task
enum oads_state
{
    /// Idle state
    OADS_IDLE,
    /// busy state
    OADS_BUSY,
    /// Number of defined states.
    OADS_STATE_MAX
};


/// Messages for BRACELET Server
enum oads_msg_id
{
	/// Start the Bracelet profile - at connection used 
	OADS_ENABLE_REQ   = TASK_FIRST_MSG(TASK_ID_OADS),

	/// Confirmation of the Bracelet Server start
	OADS_ENABLE_RSP,

	OADS_FFC1_WRITER_REQ_IND,

	OADS_FFC1_NTF_CFG_IND,

	OADS_FFC1_UPD_REQ,
	OADS_FFC1_UPD_RSP,

	OADS_FFC2_WRITER_REQ_IND,

	OADS_FFC2_NTF_CFG_IND,

	OADS_FFC2_UPD_REQ,

	OADS_FFC2_UPD_RSP,
	
};



/// Features Flag Masks
enum oads_features
{
    ///  Characteristic doesn't support notifications
    OADS_NTF_NOT_SUP,
    
    OADS_NTF_SUP,
};

/*
 * APIs Structures
 ****************************************************************************************
 */
/// Parameters for the database creation
struct oads_db_cfg
{
    /// Number of  to add
    uint8_t oad_nb;
    /// Features of each  instance
    uint8_t features;
   };

/// Parameters of the @ref BRACES_ENABLE_REQ message
struct oads_enable_req
{
	/// connection index
	uint8_t  conidx;
	/// Notification Configuration
	uint16_t  ffc1_ntf_cfg;

	uint16_t  ffc2_ntf_cfg;
};



/// Parameters of the @ref OADS_ENABLE_RSP message
struct oads_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref OADS_FFC1_WRITER_REQ_IND message
struct oads_ffc1_writer_ind
{
    /// instance
    uint8_t conidx;
	
	uint8_t length;
    /// data
    uint8_t data[OADS_FFC1_DATA_LEN];
};

///Parameters of the @ref OADS_FFC1_WRITER_REQ_IND message
struct oads_ffc2_writer_ind
{
    /// instance
    uint8_t conidx;
	
	uint8_t length;
    /// data
    uint8_t data[OADS_FFC2_DATA_LEN];
};

///Parameters of the @ref OADS_FFC1_UPD_REQ message
struct oads_ffc1_upd_req
{
    /// instance
    uint8_t conidx;
	
	uint8_t length;
    /// data
    uint8_t data[OADS_FFC1_DATA_LEN];
};

///Parameters of the @ref OADS_FFC1_UPD_REQ message
struct oads_ffc2_upd_req
{
    /// instance
    uint8_t conidx;
	
	uint8_t length;
    /// data
    uint8_t data[OADS_FFC2_DATA_LEN];
};


///Parameters of the @ref OADS_FFC1_UPD_RSP message
struct oads_ffc1_upd_rsp
{
	uint8_t  conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref OADS_FFC2_UPD_RSP message
struct oads_ffc2_upd_rsp
{
	uint8_t  conidx;
    ///status
    uint8_t status;
};



/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct kernel_state_handler braces_default_handler;
#endif // BLE_BRACELET_REPORTER


#endif /* _BRACES_TASK_H_ */

