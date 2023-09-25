/**
 ****************************************************************************************
 *
 * @file btl.h
 *
 * @brief Header file - media Service Server Role
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */
#ifndef _BTL_H_
#define _BTL_H_

/**
 ****************************************************************************************
 * @addtogroup  BTL 'Profile' Server
 * @ingroup BTL
 * @brief BTL 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_BTL_SERVER)

#include "btl_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

#define BTL_CFG_FLAG_MANDATORY_MASK       (0x7F)
#define BTL_CFG_FLAG_IND_SUP_MASK         (0x08)

enum
{		
	ATT_SVC_BTL 			   = ATT_UUID_16(0xFFFF),

	ATT_CHAR_BTL_FF01         =  ATT_UUID_16(0xFF01),

	ATT_CHAR_BTL_FF02		   = ATT_UUID_16(0xFF02),
		
};

///  Service Attributes Indexes
enum
{
	BTL_IDX_SVC,	 
	BTL_IDX_FF01_VAL_CHAR,
	BTL_IDX_FF01_VAL_VALUE,
	BTL_IDX_FF02_VAL_CHAR,
	BTL_IDX_FF02_VAL_VALUE,
	BTL_IDX_FF02_VAL_IND_CFG,
	BTL_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


///  'Profile' Server environment variable
struct btl_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct kernel_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
   
    /// BASS task state
    kernel_state_t state[BTL_IDX_MAX];
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
const struct prf_task_cbs* btl_prf_itf_get(void);

uint16_t btl_get_att_handle(uint8_t att_idx);

uint8_t  btl_get_att_idx(uint16_t handle, uint8_t *att_idx);

void btl_exe_operation(void);

#endif /* #if (BLE_BTL_SERVER) */



#endif /*  _MS_H_ */




