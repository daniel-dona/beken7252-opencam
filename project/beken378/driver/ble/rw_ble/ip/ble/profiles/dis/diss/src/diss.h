/**
 ****************************************************************************************
 *
 * @file diss.h
 *
 * @brief Header file - Device Information Service Server.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef DISS_H_
#define DISS_H_

/**
 ****************************************************************************************
 * @addtogroup DISS Device Information Service Server
 * @ingroup DIS
 * @brief Device Information Service Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#if (BLE_DIS_SERVER)
#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */


#define DISS_IDX_MAX        (1)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the DISS task
enum
{
    /// Idle state
    DISS_IDLE,
    /// Busy state
    DISS_BUSY,
    /// Number of defined states.
    DISS_STATE_MAX
};

/// DISS Attributes database handle list
enum diss_att_db_handles
{
    DIS_IDX_SVC,

    DIS_IDX_MANUFACTURER_NAME_CHAR,
    DIS_IDX_MANUFACTURER_NAME_VAL,

    DIS_IDX_MODEL_NB_STR_CHAR,
    DIS_IDX_MODEL_NB_STR_VAL,

    DIS_IDX_SERIAL_NB_STR_CHAR,
    DIS_IDX_SERIAL_NB_STR_VAL,

    DIS_IDX_HARD_REV_STR_CHAR,
    DIS_IDX_HARD_REV_STR_VAL,

    DIS_IDX_FIRM_REV_STR_CHAR,
    DIS_IDX_FIRM_REV_STR_VAL,

    DIS_IDX_SW_REV_STR_CHAR,
    DIS_IDX_SW_REV_STR_VAL,

    DIS_IDX_SYSTEM_ID_CHAR,
    DIS_IDX_SYSTEM_ID_VAL,

    DIS_IDX_IEEE_CHAR,
    DIS_IDX_IEEE_VAL,

    DIS_IDX_PNP_ID_CHAR,
    DIS_IDX_PNP_ID_VAL,

    DIS_IDX_NB,
};

///Attribute Table Indexes
enum diss_info
{
    /// Manufacturer Name
    DIS_MANUFACTURER_NAME_CHAR,
    /// Model Numberd
    DIS_MODEL_NB_STR_CHAR,
    /// Serial Number
    DIS_SERIAL_NB_STR_CHAR,
    /// HW Revision Number
    DIS_HARD_REV_STR_CHAR,
    /// FW Revision Number
    DIS_FIRM_REV_STR_CHAR,
    /// SW Revision Number
    DIS_SW_REV_STR_CHAR,
    /// System Identifier Name
    DIS_SYSTEM_ID_CHAR,
    /// IEEE Certificate
    DIS_IEEE_CHAR,
    /// Plug and Play Identifier
    DIS_PNP_ID_CHAR,

    DIS_CHAR_MAX,
};

///Database Configuration Flags
enum diss_features
{
    ///Indicate if Manufacturer Name String Char. is supported
    DIS_MANUFACTURER_NAME_CHAR_SUP       = 0x0001,
    ///Indicate if Model Number String Char. is supported
    DIS_MODEL_NB_STR_CHAR_SUP            = 0x0002,
    ///Indicate if Serial Number String Char. is supported
    DIS_SERIAL_NB_STR_CHAR_SUP           = 0x0004,
    ///Indicate if Hardware Revision String Char. supports indications
    DIS_HARD_REV_STR_CHAR_SUP            = 0x0008,
    ///Indicate if Firmware Revision String Char. is writable
    DIS_FIRM_REV_STR_CHAR_SUP            = 0x0010,
    ///Indicate if Software Revision String Char. is writable
    DIS_SW_REV_STR_CHAR_SUP              = 0x0020,
    ///Indicate if System ID Char. is writable
    DIS_SYSTEM_ID_CHAR_SUP               = 0x0040,
    ///Indicate if IEEE Char. is writable
    DIS_IEEE_CHAR_SUP                    = 0x0080,
    ///Indicate if PnP ID Char. is writable
    DIS_PNP_ID_CHAR_SUP                  = 0x0100,

    ///All features are supported
    DIS_ALL_FEAT_SUP                     = 0x01FF,
};


/// Value element
struct diss_val_elmt
{
    /// list element header
    struct common_list_hdr hdr;
    /// value identifier
    uint8_t value;
    /// value length
    uint8_t length;
    /// value data
    uint8_t data[__ARRAY_EMPTY];
};

///Device Information Service Server Environment Variable
struct diss_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// List of values set by application
    struct common_list values;
    /// Service Attribute Start Handle
    uint16_t start_hdl;
    /// Services features
    uint16_t features;
    /// Last requested value
    uint8_t  req_val;
    /// Last connection index which request value
    uint8_t  req_conidx;

    /// DISS task state
    kernel_state_t state[DISS_IDX_MAX];
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve DIS service profile interface
 *
 * @return DIS service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* diss_prf_itf_get(void);


/**
 ****************************************************************************************
 * @brief Check if an attribute shall be added or not in the database
 *
 * @param features DIS features
 *
 * @return Feature config flag
 ****************************************************************************************
 */
uint32_t diss_compute_cfg_flag(uint16_t features);

/**
 ****************************************************************************************
 * @brief Check if the provided value length matches characteristic requirements
 * @param char_code Characteristic Code
 * @param val_len   Length of the Characteristic value
 *
 * @return status if value length is ok or not
 ****************************************************************************************
 */
uint8_t diss_check_val_len(uint8_t char_code, uint8_t val_len);

/**
 ****************************************************************************************
 * @brief Retrieve handle attribute from value
 *
 * @param[in] env   Service environment variable
 * @param[in] value Value to search
 *
 * @return Handle attribute from value
 ****************************************************************************************
 */
uint16_t diss_value_to_handle(struct diss_env_tag* env, uint8_t value);

/**
 ****************************************************************************************
 * @brief Retrieve value from attribute handle
 *
 * @param[in] env    Service environment variable
 * @param[in] handle Attribute handle to search
 *
 * @return  Value from attribute handle
 ****************************************************************************************
 */
uint8_t diss_handle_to_value(struct diss_env_tag* env, uint16_t handle);

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */
extern const struct kernel_state_handler diss_default_handler;

#endif //BLE_DIS_SERVER

/// @} DISS

#endif // DISS_H_
