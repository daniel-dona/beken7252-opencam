/**
****************************************************************************************
*
* @file lld_wlcoex.h
*
* @brief Wireless LAN Coexistence mailbox functions
*
* Copyright (C) RivieraWaves 2009-2015
*
*
****************************************************************************************
*/
#ifndef LLD_WLCOEX_H_
#define LLD_WLCOEX_H_
/**
****************************************************************************************
* @defgroup LDWLCOEX WIFI Coexistence
* @ingroup LD
* @brief Responsible for WIFI coexistence.
* @{
****************************************************************************************
*/

/*
 * CONSTANT DEFINTIONS
 ****************************************************************************************
 */
#if (RW_BLE_WLAN_COEX) || (RW_BLE_MWS_COEX)
/// Coexistence disabled
#define BLECOEX_DISABLED        0
/// Coexistence enabled
#define BLECOEX_ENABLED         1

/// Coexistence Definitions
#define BLEMPRIO_CONREQ     0
#define BLEMPRIO_LLCP       1
#define BLEMPRIO_DATA       2
#define BLEMPRIO_INITSC     3
#define BLEMPRIO_ACTSC      4
#define BLEMPRIO_CONADV     5
#define BLEMPRIO_NCONADV    6
#define BLEMPRIO_PASSC      7
#define BLEMPRIO_DFT        15
#endif // (RW_BLE_WLAN_COEX) || (RW_BLE_MWS_COEX)


/*
 * FUNCTION PROTOTYPES
 ****************************************************************************************
 */
#if (RW_BLE_WLAN_COEX)
/**
 ****************************************************************************************
 * @brief Enable/Disable the Wireless LAN coexistence interface.
 * 
 * @param[in]   CoexSetting     Coexistence value
 *
 ****************************************************************************************
 */
void lld_wlcoex_set(bool en);
#endif // RW_BLE_WLAN_COEX

#if (RW_BLE_MWS_COEX)
/**
 ****************************************************************************************
 * @brief Enable/Disable the MWS coexistence interface.
 * 
 * @param[in]  en    Enable=true/Disable=false
 *
 ****************************************************************************************
 */
void lld_mwscoex_set(bool en);
#endif // RW_BLE_MWS_COEX

///@} LLDWLCOEX

#endif // LLD_WLCOEX_H_
