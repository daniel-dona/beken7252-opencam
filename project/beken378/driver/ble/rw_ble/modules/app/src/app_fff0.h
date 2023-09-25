/**
 ****************************************************************************************
 *
 * @file app_fff0.c
 *
 * @brief findt Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2016.05.31
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */
#ifndef APP_FFF0_H_
#define APP_FFF0_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief fff0 Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_FFF0)

#include <stdint.h>          // Standard Integer Definition
#include "kernel_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

/// fff0s Application Module Environment Structure
struct app_fff0_env_tag
{
    /// Connection handle
    uint8_t conidx;
    /// Current Battery Level
    uint8_t fff1_lvl;
};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// fff0s Application environment
extern struct app_fff0_env_tag app_fff0_env;

/// Table of message handlers
extern const struct kernel_state_handler app_fff0_table_handler;
/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 * fff0s Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize fff0s Application Module
 ****************************************************************************************
 */
void app_fff0_init(void);
/**
 ****************************************************************************************
 * @brief Add a fff0 Service instance in the DB
 ****************************************************************************************
 */
void app_fff0_add_fff0s(void);
/**
 ****************************************************************************************
 * @brief Enable the fff0 Service
 ****************************************************************************************
 */
void app_fff0_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a Battery level value
 ****************************************************************************************
 */
void app_fff0_send_lvl(uint8_t batt_lvl);

#endif //(BLE_APP_FFF0)

#endif // APP_BATT_H_
