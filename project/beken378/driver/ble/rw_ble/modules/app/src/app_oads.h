/**
 ****************************************************************************************
 *
 * @file app_oads.h
 *
 * @brief OAD Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2016.10.13
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */
#ifndef APP_OADS_H_
#define APP_OADS_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup BEKEN
 *
 * @brief OADS Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_OADS)

#include <stdint.h>          // Standard Integer Definition
#include "kernel_task.h"         // Kernel Task Definition
#include "oads.h"
/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */
 

/// bracess Application Module Environment Structure
struct app_oads_env_tag
{
    /// Connection handle
    uint8_t conidx;
	
	
};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// fff0s Application environment
extern struct app_oads_env_tag app_oads_env;

/// Table of message handlers
extern const struct kernel_state_handler app_oads_table_handler;
/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 * braces Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize braces Application Module
 ****************************************************************************************
 */
void app_oads_init(void);
/**
 ****************************************************************************************
 * @brief Add a oad Service instance in the DB
 ****************************************************************************************
 */
void app_oad_add_oads(void);
/**
 ****************************************************************************************
 * @brief Enable the oad Service
 ****************************************************************************************
 */
void app_oads_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a step_info
 ****************************************************************************************
 */
 
 /*********************************************************************
 * LOCAL FUNCTIONS
 */



#endif //(BLE_APP_OADS)

#endif // APP_OADS_H_
