/**
 ****************************************************************************************
 *
 * @file application.h
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_PRESENT)

#include <stdint.h>          // Standard Integer Definition
#include "common_bt.h"           // Common BT Definitions
#include "architect.h"            // Platform Definitions
#include "gapc.h"            // GAPC Definitions

#if (NVDS_SUPPORT)
#include "nvds.h"
#endif // (NVDS_SUPPORT)

#include "ble_pub.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximal length of the Device Name value
#define APP_DEVICE_NAME_MAX_LEN      (18)

#define  TOTAL_BLOCK_NUM  			 (80)

/// Default Advertising duration - 30s (in multiple of 10ms)
#define APP_DFLT_ADV_DURATION        (3000)



#define BASE3 0x00003C60
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

#if (NVDS_SUPPORT)
/// List of Application NVDS TAG identifiers
enum app_nvds_tag
{
    /// BLE Application Advertising data
    NVDS_TAG_APP_BLE_ADV_DATA           = 0x0B,
    NVDS_LEN_APP_BLE_ADV_DATA           = 32,

    /// BLE Application Scan response data
    NVDS_TAG_APP_BLE_SCAN_RESP_DATA     = 0x0C,
    NVDS_LEN_APP_BLE_SCAN_RESP_DATA     = 32,

    /// Mouse Sample Rate
    NVDS_TAG_MOUSE_SAMPLE_RATE          = 0x38,
    NVDS_LEN_MOUSE_SAMPLE_RATE          = 1,
    /// Peripheral Bonded
    NVDS_TAG_PERIPH_BONDED              = 0x39,
    NVDS_LEN_PERIPH_BONDED              = 1,
    /// Mouse NTF Cfg
    NVDS_TAG_MOUSE_NTF_CFG              = 0x3A,
    NVDS_LEN_MOUSE_NTF_CFG              = 2,
    /// Mouse Timeout value
    NVDS_TAG_MOUSE_TIMEOUT              = 0x3B,
    NVDS_LEN_MOUSE_TIMEOUT              = 2,
    /// Peer Device BD Address
    NVDS_TAG_PEER_BD_ADDRESS            = 0x3C,
    NVDS_LEN_PEER_BD_ADDRESS            = 7,
    /// Mouse Energy Safe
    NVDS_TAG_MOUSE_ENERGY_SAFE          = 0x3D,
    NVDS_LEN_MOUSE_SAFE_ENERGY          = 2,
    /// EDIV (2bytes), RAND NB (8bytes),  LTK (16 bytes), Key Size (1 byte)
    NVDS_TAG_LTK                        = 0x3E,
    NVDS_LEN_LTK                        = 28,
    /// PAIRING
    NVDS_TAG_PAIRING                    = 0x3F,
    NVDS_LEN_PAIRING                    = 54,
};

enum app_loc_nvds_tag
{
    /// Audio mode 0 task
    NVDS_TAG_AM0_FIRST                  = NVDS_TAG_APP_SPECIFIC_FIRST, // 0x90
    NVDS_TAG_AM0_LAST                   = NVDS_TAG_APP_SPECIFIC_FIRST+16, // 0xa0

    /// Local device Identity resolving key
    NVDS_TAG_LOC_IRK,
    /// Peer device Resolving identity key (+identity address)
    NVDS_TAG_PEER_IRK,

    /// size of local identity resolving key
    NVDS_LEN_LOC_IRK                    = KEY_LEN,
    /// size of Peer device identity resolving key (+identity address)
    NVDS_LEN_PEER_IRK                   = sizeof(struct gapc_irk),
};
#endif // (NVDS_SUPPORT)

/// Application environment structure
struct app_env_tag
{
    /// Connection handle
    uint16_t conhdl;
    /// Connection Index
    uint8_t  conidx;

    /// Last initialized profile
    uint8_t next_svc;

    /// Bonding status
    bool bonded;

    /// Device Name length
    uint8_t dev_name_len;
    /// Device Name
    uint8_t dev_name[APP_DEVICE_NAME_MAX_LEN];

    /// Local device IRK
    uint8_t loc_irk[KEY_LEN];
	
			 /// Peer device IRK
    struct gapc_irk peer_irk;
	
};

//This struct use to remote ctrl device.
typedef enum 
{
	CONN_MODE_LONG,
	CONN_MODE_SHORT,
	CONN_MODE_ERROR,
}remote_conn_mode;


typedef struct 
{
	remote_conn_mode target_mode;
	uint32_t target_timeout;
}remote_ctrl_struct;


/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */

/// Application environment
extern struct app_env_tag app_env;

//Voice buffer
extern uint8_t encode_voice_data[TOTAL_BLOCK_NUM][20];

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize the BLE demo application.
 ****************************************************************************************
 */
void appm_init(void);

/**
 ****************************************************************************************
 * @brief Add a required service in the database
 ****************************************************************************************
 */
bool appm_add_svc(void);

/**
 ****************************************************************************************
 * @brief Put the device in general discoverable and connectable mode
 ****************************************************************************************
 */
#if (BLE_APP_MS)
msm_result_t ms_start_advertising(msm_ble_adv_content_t *adv_info);
#else
ble_err_t appm_start_advertising(void);
#endif //(BLE_APP_MS)
/**
 ****************************************************************************************
 * @brief Put the device in general discoverable and connectable mode
 ****************************************************************************************
 */
void appm_start_direct_dvertising(void);

/**
 ****************************************************************************************
 * @brief Put the device in non discoverable and non connectable mode
 ****************************************************************************************
 */
ble_err_t appm_stop_advertising(void);

/**
 ****************************************************************************************
 * @brief Send to request to update the connection parameters
 ****************************************************************************************
 */
void appm_update_param(struct gapc_conn_param *conn_param);

/**
 ****************************************************************************************
 * @brief Send a disconnection request
 ****************************************************************************************
 */
void appm_disconnect(uint8_t reason);


/**
 ****************************************************************************************
 * @brief appm_switch_general_adv
 ****************************************************************************************
 */
void appm_switch_general_adv(void);

/**
 ****************************************************************************************
 * @brief Retrieve device name
 *
 * @param[out] device name
 *
 * @return name length
 ****************************************************************************************
 */
uint8_t appm_get_dev_name(uint8_t* name);


/**
 ****************************************************************************************
 * @brief Return if the device is currently bonded
 ****************************************************************************************
 */
bool app_sec_get_bond_status(void);


/**
 ****************************************************************************************
 * @brief Send to request to security
 ****************************************************************************************
 */
void appm_send_seurity_req(void);


/**
 ****************************************************************************************
 * @brief app_send_encode_evt
 ****************************************************************************************
 */
void app_send_encode_evt(void);

/**
 ****************************************************************************************
 * @brief read_encode_data
 ****************************************************************************************
 */
uint8_t  read_encode_data(uint8_t *buf);

/**
 ****************************************************************************************
 * @brief store_encode_data
 ****************************************************************************************
 */
uint8_t  store_encode_data(uint8_t *buf);

/**
 ****************************************************************************************
 * @brief encode_voice_init
 ****************************************************************************************
 */
void encode_voice_init(void);

/**
 ****************************************************************************************
 * @brief print_debug_info
 ****************************************************************************************
 */
void print_debug_info(void);


/**
 ****************************************************************************************
 * @brief print_debug_status
 ****************************************************************************************
 */
void print_debug_status(void);


/**
 ****************************************************************************************
 * @brief app_set_mode
 ****************************************************************************************
 */
void app_set_mode(remote_conn_mode mode, uint32_t timeout);


/**
 ****************************************************************************************
 * @brief app_get_mode
 ****************************************************************************************
 */
remote_ctrl_struct app_get_mode(void);


/// @} APP

#endif //(BLE_APP_PRESENT)

#endif // APP_H_
