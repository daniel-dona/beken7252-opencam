/**
 ****************************************************************************************
 *
 * @file ffe0s.h
 *
 * @brief Header file - FFE0 Service Server Role
 *
 * Copyright (C) beken 2009-2015
 *
 *
 ****************************************************************************************
 */
#ifndef _FFE0S_H_
#define _FFE0S_H_

/**
 ****************************************************************************************
 * @addtogroup  FFE0 'Profile' Server
 * @ingroup FFE0
 * @brief FFE0 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_FFE0_SERVER)

#include "ffe0s_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

#define FFE0S_CFG_FLAG_MANDATORY_MASK       (0x7F)
#define FFE0_CFG_FLAG_NTF_SUP_MASK         (0x08)

#define FFE1_FLAG_NTF_CFG_BIT             (0x03)

#define FFE1_USER_DESC             ("voice")
#define FFE1_USER_DESC_LEN          sizeof(FFE1_USER_DESC)
enum
{		
	ATT_SVC_FFE0 			   = ATT_UUID_16(0xFFE0),

	ATT_CHAR_FFE0_FFE1         = ATT_UUID_16(0xFFE1),

	ATT_CHAR_FFE0_FFE2		   = ATT_UUID_16(0xFFE2),
		
};

/// FFE0 Service Attributes Indexes
enum
{
	FFE0S_IDX_SVC,	 
	FFE0S_IDX_FFE1_VAL_CHAR,
	FFE0S_IDX_FFE1_VAL_VALUE,
	FFE0S_IDX_FFE1_VAL_NTF_CFG,
	FFE0S_IDX_FFE1_VAL_USER_DESC,
	FFE0S_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// FFF0 'Profile' Server environment variable
struct ffe0s_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct kernel_msg * operation;
    /// FFF0 Services Start Handle
    uint16_t start_hdl;
   
    /// BASS task state
    kernel_state_t state[FFE0S_IDX_MAX];
    /// Notification configuration of peer devices.
    uint8_t ntf_cfg[BLE_CONNECTION_MAX];
    /// Database features
    uint8_t features;
	
	uint8_t ffe1_len;
	uint8_t *ffe1_val;
};



/**
 ****************************************************************************************
 * @brief Retrieve fff0 service profile interface
 *
 * @return fff0 service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* ffe0s_prf_itf_get(void);

uint16_t ffe0s_get_att_handle(uint8_t att_idx);

uint8_t  ffe0s_get_att_idx(uint16_t handle, uint8_t *att_idx);

void ffe0s_exe_operation(void);

#endif /* #if (BLE_FFE0_SERVER) */



#endif /*  _FFF0_H_ */




