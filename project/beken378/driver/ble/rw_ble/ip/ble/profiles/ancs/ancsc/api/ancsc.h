/**
 ****************************************************************************************
 *
 * @file ancsc.h
 *
 * @brief ancs  file  .
 *
 * Copyright (C) BEKEN 2016 - 2017
 *
 *
 ****************************************************************************************
 */

#ifndef ANCSC_H_
#define ANCSC_H_

/**
 ****************************************************************************************
 * @addtogroup ANCS Client
 * @ingroup ANCS
 * @brief ANCS Profile Client
 *
 * The ANCS is responsible for providing ANCS profile Client functionalities to
 * upper layer module or application. The device using this profile takes the role
 * of ANCS Client
 *
 *  ANCS Profile Client (LOC): A LOC (e.g. PC, phone, etc)
 * is the term used by this profile to describe a device that can get notic
 * in the peer Device (IPHONE), causing the to start a info notic
 * or other type of signal allowing it to be located.
 *
 * The interface of this role to the Application is:
 *  - Enable the profile role (from Iphone)
 *  - Disable the profile role (from Iphone)
 *  - Set info notic enable/disable in iphone (from Iphone)

 *
 *
 *  The enable/disable of the profile and devices disconnection is handled in the application,
 *  depending on its User Input.
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
#include "prf_types.h"
#include "prf_utils.h"

/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of Ancs Client task instances
#define ANCSC_IDX_MAX    (BLE_CONNECTION_MAX)


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

enum ancsc_char_type
{
   
		ANCSC_CNTL_POINT_CHAR,
	
		ANCSC_NTF_SOURCE_CHAR,
	
		ANCSC_DATA_SOURCE_CHAR,
	
    ANCS_SOURCE_CHAR_MAX,
};


enum ancsc_desc_type
{
    /// ANCSC Client NTF Characteristic Configuration
    ANCSC_DESC_NTF_SRC_CFG,
	
		ANCSC_DESC_DATA_SRC_CFG,

    ANCSC_DESC_MAX,
};


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// ancs notif service details container
struct ancsc_content
{
    /// Service info
    struct prf_svc svc;

    /// Characteristic info:
    /// 
    struct prf_char_inf chars[ANCS_SOURCE_CHAR_MAX];	
	    /// Descriptor handles:
   
    struct prf_char_desc_inf descs[ANCSC_DESC_MAX];
};

/// Environment variable for each Connections
struct ancsc_cnx_env
{
	  /// on-going operation
    struct kernel_msg * operation;
    /// Found ANCSC details
    struct ancsc_content ancsc;
    /// Last char. code requested to read.
    uint8_t  last_char_code;
    /// counter used to check service uniqueness
    uint8_t  nb_svc;
};

/// ANCS Client environment variable
struct ancsc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connections
    struct ancsc_cnx_env* env[ANCSC_IDX_MAX];
    /// State of different task instances
    kernel_state_t state[ANCSC_IDX_MAX];
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve ancs service profile interface
 *
 * @return ANCS service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* ancsc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send Enable Confirm message to the application.
 * @param status  Status to send.
 ****************************************************************************************
 */
void ancsc_enable_rsp_send(struct ancsc_env_tag *ancsc_env, uint8_t conidx, uint8_t status);

#endif //BLE_FINDME_LOCATOR

/// @} ANCSC
#endif // ANCSC_H_
