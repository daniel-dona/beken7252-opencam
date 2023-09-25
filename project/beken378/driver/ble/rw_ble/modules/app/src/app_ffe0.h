/**
 ****************************************************************************************
 *
 * @file app_ffe0.c
 *
 * @brief findt Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2017.03.29
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */
#ifndef APP_FFE0_H_
#define APP_FFE0_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief ffe0 Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_FFE0)

#include <stdint.h>          // Standard Integer Definition
#include "kernel_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

/// fff0s Application Module Environment Structure
struct app_ffe0_env_tag
{
    /// Connection handle
    uint8_t conidx;
    /// Current Ba
    uint8_t ffe1_ntf_cfg;
    uint8_t first_send_ffe1_flag;
	uint8_t key_status;
};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// fff0s Application environment
extern struct app_ffe0_env_tag app_ffe0_env;

/// Table of message handlers
extern const struct kernel_state_handler app_ffe0_table_handler;
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
void app_ffe0_init(void);
/**
 ****************************************************************************************
 * @brief Add a fff0 Service instance in the DB
 ****************************************************************************************
 */
void app_ffe0_add_ffe0s(void);
/**
 ****************************************************************************************
 * @brief Enable the fff0 Service
 ****************************************************************************************
 */
void app_ffe0_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a Battery level value
 ****************************************************************************************
 */
void app_ffe1_send_val(uint8_t len,uint8_t *buf,uint16_t seq_num);

#endif //(BLE_APP_FFF0)

#endif // APP_BATT_H_
