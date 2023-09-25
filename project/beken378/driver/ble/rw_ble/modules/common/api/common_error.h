/**
 ****************************************************************************************
 *
 * @file common_error.h
 *
 * @brief List of codes for error in RW Software.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef COMMON_ERROR_H_
#define COMMON_ERROR_H_

/**
 ****************************************************************************************
 * @addtogroup COMMON_ERROR Error Codes
 * @ingroup COMMON
 * @brief Defines error codes in messages.
 *
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
enum common_error
{
/*****************************************************
 ***              ERROR CODES                      ***
 *****************************************************/

    COMMON_ERROR_NO_ERROR                        = 0x00,
    COMMON_ERROR_UNKNOWN_HCI_COMMAND             = 0x01,
    COMMON_ERROR_UNKNOWN_CONNECTION_ID           = 0x02,
    COMMON_ERROR_HARDWARE_FAILURE                = 0x03,
    COMMON_ERROR_PAGE_TIMEOUT                    = 0x04,
    COMMON_ERROR_AUTH_FAILURE                    = 0x05,
    COMMON_ERROR_PIN_MISSING                     = 0x06,
    COMMON_ERROR_MEMORY_CAPA_EXCEED              = 0x07,
    COMMON_ERROR_CON_TIMEOUT                     = 0x08,
    COMMON_ERROR_CON_LIMIT_EXCEED                = 0x09,
    COMMON_ERROR_SYNC_CON_LIMIT_DEV_EXCEED       = 0x0A,
    COMMON_ERROR_ACL_CON_EXISTS                  = 0x0B,
    COMMON_ERROR_COMMAND_DISALLOWED              = 0x0C,
    COMMON_ERROR_CONN_REJ_LIMITED_RESOURCES      = 0x0D,
    COMMON_ERROR_CONN_REJ_SECURITY_REASONS       = 0x0E,
    COMMON_ERROR_CONN_REJ_UNACCEPTABLE_BDADDR    = 0x0F,
    COMMON_ERROR_CONN_ACCEPT_TIMEOUT_EXCEED      = 0x10,
    COMMON_ERROR_UNSUPPORTED                     = 0x11,
    COMMON_ERROR_INVALID_HCI_PARAM               = 0x12,
    COMMON_ERROR_REMOTE_USER_TERM_CON            = 0x13,
    COMMON_ERROR_REMOTE_DEV_TERM_LOW_RESOURCES   = 0x14,
    COMMON_ERROR_REMOTE_DEV_POWER_OFF            = 0x15,
    COMMON_ERROR_CON_TERM_BY_LOCAL_HOST          = 0x16,
    COMMON_ERROR_REPEATED_ATTEMPTS               = 0x17,
    COMMON_ERROR_PAIRING_NOT_ALLOWED             = 0x18,
    COMMON_ERROR_UNKNOWN_LMP_PDU                 = 0x19,
    COMMON_ERROR_UNSUPPORTED_REMOTE_FEATURE      = 0x1A,
    COMMON_ERROR_SCOMMON_OFFSET_REJECTED             = 0x1B,
    COMMON_ERROR_SCOMMON_INTERVAL_REJECTED           = 0x1C,
    COMMON_ERROR_SCOMMON_AIR_MODE_REJECTED           = 0x1D,
    COMMON_ERROR_INVALID_LMP_PARAM               = 0x1E,
    COMMON_ERROR_UNSPECIFIED_ERROR               = 0x1F,
    COMMON_ERROR_UNSUPPORTED_LMP_PARAM_VALUE     = 0x20,
    COMMON_ERROR_ROLE_CHANGE_NOT_ALLOWED         = 0x21,
    COMMON_ERROR_LMP_RSP_TIMEOUT                 = 0x22,
    COMMON_ERROR_LMP_COLLISION                   = 0x23,
    COMMON_ERROR_LMP_PDU_NOT_ALLOWED             = 0x24,
    COMMON_ERROR_ENC_MODE_NOT_ACCEPT             = 0x25,
    COMMON_ERROR_LINK_KEY_CANT_CHANGE            = 0x26,
    COMMON_ERROR_QOS_NOT_SUPPORTED               = 0x27,
    COMMON_ERROR_INSTANT_PASSED                  = 0x28,
    COMMON_ERROR_PAIRING_WITH_UNIT_KEY_NOT_SUP   = 0x29,
    COMMON_ERROR_DIFF_TRANSACTION_COLLISION      = 0x2A,
    COMMON_ERROR_QOS_UNACCEPTABLE_PARAM          = 0x2C,
    COMMON_ERROR_QOS_REJECTED                    = 0x2D,
    COMMON_ERROR_CHANNEL_CLASS_NOT_SUP           = 0x2E,
    COMMON_ERROR_INSUFFICIENT_SECURITY           = 0x2F,
    COMMON_ERROR_PARAM_OUT_OF_MAND_RANGE         = 0x30,
    COMMON_ERROR_ROLE_SWITCH_PEND                = 0x32, /* LM_ROLE_SWITCH_PENDING               */
    COMMON_ERROR_RESERVED_SLOT_VIOLATION         = 0x34, /* LM_RESERVED_SLOT_VIOLATION           */
    COMMON_ERROR_ROLE_SWITCH_FAIL                = 0x35, /* LM_ROLE_SWITCH_FAILED                */
    COMMON_ERROR_EIR_TOO_LARGE                   = 0x36, /* LM_EXTENDED_INQUIRY_RESPONSE_TOO_LARGE */
    COMMON_ERROR_SP_NOT_SUPPORTED_HOST           = 0x37,
    COMMON_ERROR_HOST_BUSY_PAIRING               = 0x38,
    COMMON_ERROR_CONTROLLER_BUSY                 = 0x3A,
    COMMON_ERROR_UNACCEPTABLE_CONN_INT           = 0x3B,
    COMMON_ERROR_DIRECT_ADV_TO                   = 0x3C,
    COMMON_ERROR_TERMINATED_MIC_FAILURE          = 0x3D,
    COMMON_ERROR_CONN_FAILED_TO_BE_EST           = 0x3E,
    COMMON_ERROR_CCA_REJ_USE_CLOCK_DRAG          = 0x40,
    COMMON_ERROR_UNDEFINED                       = 0xFF,


/*****************************************************
 ***              HW ERROR CODES                   ***
 *****************************************************/

    COMMON_ERROR_HW_UART_OUT_OF_SYNC            = 0x00,
    COMMON_ERROR_HW_MEM_ALLOC_FAIL              = 0x01,
};

/// @} COMMON_ERROR

#endif // COMMON_ERROR_H_
