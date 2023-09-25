/**
 ****************************************************************************************
 *
 * @file fff0s.h
 *
 * @brief Header file - FFF0 Service Server Role
 *
 * Copyright (C) beken 2009-2015
 *
 *
 ****************************************************************************************
 */
#ifndef _FFF0S_H_
#define _FFF0S_H_

/**
 ****************************************************************************************
 * @addtogroup  FFF0 'Profile' Server
 * @ingroup FFF0
 * @brief FFF0 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_FFF0_SERVER)

#include "fff0s_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define FFF0S_CFG_FLAG_MANDATORY_MASK       (0x3F)
#define FFF0_CFG_FLAG_NTF_SUP_MASK         (0x08)
#define FFF0_CFG_FLAG_MTP_FFF1_MASK         (0x40)

#define FFF1_LVL_MAX               			(100)

#define FFF1_FLAG_NTF_CFG_BIT             (0x02)



enum
{		
		ATT_USER_SERVER_FFF0 												= ATT_UUID_16(0xFFF0),
	  
		ATT_USER_SERVER_CHAR_FFF1                   = ATT_UUID_16(0xFFF1),
		
		ATT_USER_SERVER_CHAR_FFF2										= ATT_UUID_16(0xFFF2),
		
};

/// Battery Service Attributes Indexes
enum
{
    FFF0S_IDX_SVC,

	
	  FFF0S_IDX_FFF2_LVL_CHAR,
		FFF0S_IDX_FFF2_LVL_VAL,
	
	  FFF0S_IDX_FFF1_LVL_CHAR,
    FFF0S_IDX_FFF1_LVL_VAL,
	  FFF0S_IDX_FFF1_LVL_NTF_CFG,
	
    FFF0S_IDX_NB   ,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// FFF0 'Profile' Server environment variable
struct fff0s_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct kernel_msg * operation;
    /// FFF0 Services Start Handle
    uint16_t start_hdl;
    /// Level of the FFF1
    uint8_t fff1_lvl[FFF0_FFF1_DATA_LEN];
	
	uint8_t fff2_value[FFF0_FFF2_DATA_LEN];
    /// BASS task state
    kernel_state_t state[FFF0S_IDX_MAX];
    /// Notification configuration of peer devices.
    uint8_t ntf_cfg[BLE_CONNECTION_MAX];
    /// Database features
    uint8_t features;

};



/**
 ****************************************************************************************
 * @brief Retrieve fff0 service profile interface
 *
 * @return fff0 service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* fff0s_prf_itf_get(void);

uint16_t fff0s_get_att_handle(uint8_t att_idx);

uint8_t  fff0s_get_att_idx(uint16_t handle, uint8_t *att_idx);

void fff0s_exe_operation(void);

#endif /* #if (BLE_FFF0_SERVER) */



#endif /*  _FFF0_H_ */



