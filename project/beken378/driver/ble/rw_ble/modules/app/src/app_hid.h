/**
 ****************************************************************************************
 *
 * @file app_hid.h
 *
 * @brief HID Application Module entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef APP_HID_H_
#define APP_HID_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief HID Application Module entry point
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_HID)

#include <stdint.h>          // Standard Integer Definition
#include "kernel_task.h"         // Kernel Task Definition

#if (PS2_SUPPORT)
#include "ps2.h"             // PS2 Mouse Driver
#endif //(PS2_SUPPORT)

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

///// Length of the keyboard use Report len max
#define APP_HID_KEYBOARD_REPORT_LEN       (8)
/// Length of the HID Mouse Report
#define APP_HID_MOUSE_REPORT_LEN          (4)
///// Length of the media use Report len max
#define APP_HID_MEDIA_REPORT_LEN          (2)
///// Length of the power use Report len max
#define APP_HID_POWER_REPORT_LEN          (1)
/// Length of the Voice use Report len max
#define APP_HID_VOICE_REPORT_LEN		  (20)
/// Length of the HID Sensor Report
#define APP_HID_SENSOR_REPORT_LEN         (18)


#define	APP_HID_KEYBOARD_IN_ENDPORT			0
#define	APP_HID_KEYBOARD_OUT_ENDPORT		1
#define APP_HID_MOUSE_IN_ENDPORT			2
#define APP_HID_RMC1_IN_ENDPORT				3
#define APP_HID_RMC2_IN_ENDPORT				4
#define APP_HID_RMC2_OUT_ENDPORT			5
#define	APP_HID_MEDIA_IN_ENDPORT			6
#define APP_HID_SENSOR_IN_ENDPOINT			7
#define	APP_HID_POWER_IN_ENDPORT			8


/// HID Application Module Environment Structure
struct app_hid_env_tag
{
    /// Connection handle
    uint8_t conidx;
    /// Mouse timeout value
    uint16_t timeout;
    /// Internal state of the module
    uint8_t state;
    /// Timer enabled
    bool timer_enabled;
    /// Number of report that can be sent
    uint8_t nb_report;
	/// operation sequence number
	uint16_t seq_num;
	/// encode cnt
	uint32_t  encode_cnt;
	///encode_send_cnt
    uint32_t  encode_send_cnt;
	///encode_no_send_cnt
    uint32_t  encode_no_send_cnt;
	//audio start
   	uint8_t audio_start;
	///encode data send cnt
	uint8_t encode_tx_cnt;
};


struct mouse_msg   //use test report interface
{
	uint8_t b;
	int8_t x;
	int8_t y;
	int8_t w;
};


extern struct app_hid_env_tag app_hid_env;

extern bool hid_key_send_status;

/*
 * DEFINE DEFINITION
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// Table of message handlers
extern const struct kernel_state_handler app_hid_table_handler;

/*
 * GLOBAL FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 * Health Thermometer Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize HID Application Module
 ****************************************************************************************
 */
void app_hid_init(void);


/**
 ****************************************************************************************
 * @brief Add a HID Service instance in the DB
 ****************************************************************************************
 */
void app_hid_add_hids(void);

/**
 ****************************************************************************************
 * @brief Enable the HID Over GATT Profile device role
 *
 * @param[in]:  conhdl - Connection handle for the connection
 ****************************************************************************************
 */
void app_hid_enable_prf(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Send a mouse report to the peer device
 *
 * @param[in]:  report - Mouse report sent by the PS2 driver
 ****************************************************************************************
 */


void app_hid_send_report(uint8_t *data, uint8_t len);


void app_hid_send_mouse_report( struct mouse_msg report );


void app_hid_send_sensor_report(uint8_t *sensor_data);


void app_hid_send_audio(void);

#endif //(BLE_APP_HID)

/// @} APP

#endif // APP_HID_H_
