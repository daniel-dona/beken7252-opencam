/**
 ****************************************************************************************
 *
 * @file app_ancsc.h
 *
 * @brief ANCS Application Module entry point
 *
 * Copyright (C) BeKen 2016-2017
 *
 *
 ****************************************************************************************
 */

#ifndef APP_ANCSC_H_
#define APP_ANCSC_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup 
 *
 * @brief ANCS Application Module entry point
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_ANCSC)

#include <stdint.h>          // Standard Integer Definition
#include "kernel_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

/// ANCS Application Module Environment Structure
struct app_ancsc_env_tag
{
    /// Connection handle
    uint16_t conhdl;
    /// Current Battery Level
   // uint8_t batt_lvl;
};

/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// ANCS Application environment
extern struct app_ancsc_env_tag app_ancsc_env;

/// Table of message handlers
extern const struct kernel_state_handler app_ancsc_table_handler;

/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 * ANCS Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize ANCS Application Module
 ****************************************************************************************
 */
void app_ancsc_init(void);

/**
 ****************************************************************************************
 * @brief Add a ANCS Service instance in the DB
 ****************************************************************************************
 */
void app_ancs_add_ancsc(void);

/**
 ****************************************************************************************
 * @brief Enable the ANCS Service
 ****************************************************************************************
 */
void app_ancsc_enable_prf(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Send a ANCS cfg value
 ****************************************************************************************
 */
void app_ancsc_send_ntf_cfg(uint16_t cfg);

void app_ancsc_send_data_cfg(uint16_t cfg);

#endif //(BLE_APP_ANCSC)

/// @} APP

#endif // APP_ANCSC_H_
