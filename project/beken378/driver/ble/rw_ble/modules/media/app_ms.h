/**
 ****************************************************************************************
 *
 * @file app_ms.c
 *
 * @brief findt Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2018.09.17
 *
 * Copyright (C) Beken 2009-2018
 *
 *
 ****************************************************************************************
 */
#ifndef APP_MS_H_
#define APP_MS_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief MS Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_MS)
#include <stdint.h>          // Standard Integer Definition
#include "kernel_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

///  Application Module Environment Structure
struct app_ms_env_tag
{
    /// Connection handle
    uint8_t conidx;
    /// Current Ba
    uint16_t send_ind_cfg;
    uint8_t allow_send_flag;

};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

///  Application environment
extern struct app_ms_env_tag app_ms_env;

/// Table of message handlers
extern const struct kernel_state_handler app_ms_table_handler;
/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 *  Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize  Application Module
 ****************************************************************************************
 */
void app_ms_init(void);
/**
 ****************************************************************************************
 * @brief Add a Service instance in the DB
 ****************************************************************************************
 */
void app_ms_add_ms(void);
/**
 ****************************************************************************************
 * @brief Enable the  Service
 ****************************************************************************************
 */
void app_ms_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a Battery level value
 ****************************************************************************************
 */
void app_ms_send_val(uint8_t len,uint8_t *buf,uint16_t seq_num);

#endif //(BLE_APP_MS)

#endif // APP_MS_H_
