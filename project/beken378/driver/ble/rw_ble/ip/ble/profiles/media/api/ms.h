/**
 ****************************************************************************************
 *
 * @file ms.h
 *
 * @brief Header file - media Service Server Role
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */
#ifndef _MS_H_
#define _MS_H_

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

#if (BLE_MS_SERVER)

#include "ms_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

#define MS_CFG_FLAG_MANDATORY_MASK       (0x7F)
#define MS_CFG_FLAG_IND_SUP_MASK         (0x08)

#define MS_FLAG_IND_CFG_BIT             (0x03)

#define REVICE_USER_DESC             ("revice")
#define SEND_USER_DESC             ("ind")

#define REVICE_USER_DESC_LEN          sizeof(REVICE_USER_DESC)
#define SEND_USER_DESC_LEN          sizeof(SEND_USER_DESC)

enum
{		
	ATT_SVC_MS 			   = ATT_UUID_16(0xFF80),

	ATT_CHAR_MS_REVICE         =  ATT_UUID_16(0xFF81),

	ATT_CHAR_MS_SEND		   = ATT_UUID_16(0xFF82),
		
};

///  Service Attributes Indexes
enum
{
	MS_IDX_SVC,	 
	MS_IDX_REVICE_VAL_CHAR,
	MS_IDX_REVICE_VAL_VALUE,
	MS_IDX_REVICE_VAL_USER_DESC,
	MS_IDX_SEND_VAL_CHAR,
	MS_IDX_SEND_VAL_VALUE,
	MS_IDX_SEND_VAL_IND_CFG,
	MS_IDX_SEND_VAL_USER_DESC,
	MS_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


///  'Profile' Server environment variable
struct ms_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct kernel_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
   
    /// BASS task state
    kernel_state_t state[MS_IDX_MAX];
    /// 
    uint16_t ind_cfg[BLE_CONNECTION_MAX];
    /// Database features
    uint16_t features;	
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
const struct prf_task_cbs* ms_prf_itf_get(void);

uint16_t ms_get_att_handle(uint8_t att_idx);

uint8_t  ms_get_att_idx(uint16_t handle, uint8_t *att_idx);

void ms_exe_operation(void);

#endif /* #if (BLE_MS_SERVER) */



#endif /*  _MS_H_ */




