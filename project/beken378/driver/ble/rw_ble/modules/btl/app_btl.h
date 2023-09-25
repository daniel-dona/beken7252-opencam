/**
 ****************************************************************************************
 *
 * @file app_btl.c
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
#ifndef APP_BTL_H_
#define APP_BTL_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief BTL Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_BTL)
#include <stdint.h>          // Standard Integer Definition
#include "btl_pub.h"
#include "kernel_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

///  Application Module Environment Structure
struct app_btl_env_tag
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
extern struct app_btl_env_tag app_btl_env;

/// Table of message handlers
extern const struct kernel_state_handler app_btl_table_handler;
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
void app_btl_init(void);
/**
 ****************************************************************************************
 * @brief Add a Service instance in the DB
 ****************************************************************************************
 */
void app_btl_add_btl(void);
/**
 ****************************************************************************************
 * @brief Enable the  Service
 ****************************************************************************************
 */
void app_btl_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a Battery level value
 ****************************************************************************************
 */
void app_btl_ff02_val(uint8_t len,uint8_t *buf,uint16_t seq_num);

void app_btl_set_recive_cb(recieve_cb_t func);

#endif //(BLE_APP_BTL)

#endif // APP_BTL_H_
