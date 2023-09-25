/**
 ****************************************************************************************
 *
 * @file ais_sdk.c
 *
 * @brief  Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2017.11.08
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */
#ifndef APP_AIS_H_
#define APP_AIS_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief  Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if 1//(BLE_APP_FEB3)

#include <stdint.h>          // Standard Integer Definition
#include "kernel_task.h"         // Kernel Task Definition
#include "AIS.h"

#include "ble_pub.h"

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */




///  ais sdk Module Environment Structure
struct ais_sdk_env_tag
{
    /// Connection handle
    uint8_t conidx;
	
    uint16_t fed6_ind_cfg;
	
	uint16_t fed8_ntf_cfg;

    uint8_t fed6_ind_send_allow;
	
	uint8_t fed8_ntf_send_allow;

};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

///  Application environment
extern struct ais_sdk_env_tag ais_sdk_env;


extern struct gapm_start_advertise_cmd g_param;
/// Table of message handlers
extern const struct kernel_state_handler app_feb3_table_handler;
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
 * @brief Initialize fff0s Application Module
 ****************************************************************************************
 */
void app_feb3_init(void);
/**
 ****************************************************************************************
 * @brief Add a FEB3 Service instance in the DB
 ****************************************************************************************
 */
void app_ais_add_feb3s(void);

void feb3_ais_init(ais_bt_init_t *ais_init);

ble_err_t feb3_send_fed6_ind_value(uint32_t len,uint8_t *buf,uint16_t seq_num);

ble_err_t feb3_send_fed8_ntf_value(uint32_t len,uint8_t *buf,uint16_t seq_num);


/**
 ****************************************************************************************
 * @brief Send a Battery level value
 ****************************************************************************************
*/

#endif //(BLE_APP_FEB3)

#endif // APP_AIS_H_
