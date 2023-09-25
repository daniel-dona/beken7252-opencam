/**
 ****************************************************************************************
 *
 * @file llm_util.h
 *
 * @brief  Link layer manager utilities definitions
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */

#ifndef LLM_UTIL_H_
#define LLM_UTIL_H_

/**
 ****************************************************************************************
 * @addtogroup LLMUTIL
 * @ingroup LLM
 * @brief Link layer manager utilities definitions
 *
 * full description
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <string.h>
#include "common_endian.h"
#include "common_error.h"
#include "kernel_mem.h"
#include "ble_compiler.h"
#include "em_buf.h"
#include "reg_ble_em_rx_desc.h"
#include "reg_ble_em_tx_desc.h"
#include "reg_ble_em_cs.h"
#include "reg_blecore.h"
#include "llm.h"
#include "lld.h"
#if (HCI_PRESENT)
#include "hci.h"
#endif //(HCI_PRESENT)


/*
 * DEFINES
 ****************************************************************************************
 */
#if (BLE_CENTRAL && BLE_CHNL_ASSESS)
/// Default Channel Assessment Timer duration (1s - Multiple of 10ms)
#define LLM_UTIL_CH_ASSES_DFLT_TIMER_DUR     (100)
/// When a command to change the channel map is sent we should process it within 10 sec (9sec)
#define LLM_UTIL_CH_ASSES_MAX_TIMER_DUR     (900)
/// Default Channel Reassessment Timer duration (Multiple of Channel Assessment Timer duration)
#define LLM_UTIL_CH_ASSES_DFLT_REASS_CNT     (8)
/// Default Minimal Threshold
#define LLM_UTIL_CH_ASSES_DFLT_MIN_THR       (-10)
/// Default Maximum  Threshold
#define LLM_UTIL_CH_ASSES_DFLT_MAX_THR       (+10)
/// Default noise RSSI Threshold
#define LLM_UTIL_CH_ASSES_DFLT_NOISE_THR     (-60)
#endif // (BLE_CENTRAL && BLE_CHNL_ASSESS)

///Constant nibble to use as top 4 MSBs, to have at least 2 transitions
extern const uint8_t LLM_AA_CT1[3];

///Constant nibble to use in AA to get more than 1 bit different from Advertising AA
extern const uint8_t LLM_AA_CT2[2];

enum bl_flag_wl
{
    LLM_UTIL_BL_NO_ACTION_WL,
    LLM_UTIL_BL_CLEAR_WL,
    LLM_UTIL_BL_SET_WL
};
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
#if (BLE_CENTRAL && BLE_CHNL_ASSESS)
/**
 ****************************************************************************************
 * @brief Return the upper limit for the received quality(value set by the NVDS)
 *
 ****************************************************************************************
 */
__INLINE int8_t llm_util_ch_assess_get_upper_limit(void)
{
    return (llm_le_env.ch_map_assess.upper_limit);
};
/**
 ****************************************************************************************
 * @brief Return the lower limit for the received quality (value set by the NVDS)
 *
 ****************************************************************************************
 */
__INLINE int8_t llm_util_ch_assess_get_lower_limit(void)
{
    return (llm_le_env.ch_map_assess.lower_limit);
};
/**
 ****************************************************************************************
 * @brief Return the reassessment counter (value set by the NVDS)
 *
 ****************************************************************************************
 */
__INLINE uint8_t llm_util_ch_assess_get_reass_cnt(void)
{
    return (llm_le_env.ch_map_assess.reassess_count);
};
/**
 ****************************************************************************************
 * @brief Return the RSSI threshold for a noiser (value set by the NVDS)
 *
 ****************************************************************************************
 */
__INLINE int8_t llm_util_ch_assess_get_rssi_noise(void)
{
    return (llm_le_env.ch_map_assess.rssi_noise_limit);
};
/**
 ****************************************************************************************
 * @brief Return the assessment time (in multiple of 10ms) (value set by the NVDS)
 *
 ****************************************************************************************
 */
__INLINE uint16_t llm_util_ch_assess_get_assess_timer(void)
{
    return (llm_le_env.ch_map_assess.assess_timer);
};
#endif //(BLE_CENTRAL && BLE_CHNL_ASSESS)

/**
 ****************************************************************************************
 * @brief Checks if the channel map contains at least 1 channel
 *
 * This function checks if advertising or data channel map contains at least 1 channel.
 *
 * @param[in] channel_map        Pointer on the channel map to be checked.
 * @param[in] nb_octet           number of octet(s) to be parsed.
 *
 * @return If the channel map is correct or not.
 *
 ****************************************************************************************
 */
uint8_t llm_util_check_map_validity(uint8_t *channel_map, uint8_t nb_octet);


/**
 ****************************************************************************************
 * Retrieve position of address in white list
 *
 * @param[in] bd_address   BLE Address
 * @param[in] bd_addr_type BLE Address type
 *
 * @return position of device or BLE_WHITELIST_MAX if not found
 ****************************************************************************************
 */
uint16_t llm_util_bd_addr_wl_position(struct bd_addr const *bd_address, uint8_t bd_addr_type);

/**
 ****************************************************************************************
 * @brief Checks if the device address is available in the white list.
 *
 * This function checks if the bd address if present in the WL and return the status
 *
 * @param[in] bd_address         Pointer on the device address to be checked.
 * @param[in] bd_address_type    Type of the address ..
 * @param[in/out] position       Position in the White List.
 * @param[out] in_black_list     check if device is in black list or not
 *
 * @return If the device address has been found or not.
 *
 ****************************************************************************************
 */
bool llm_util_bd_addr_in_wl(struct bd_addr const *bd_address, uint8_t bd_addr_type, bool* in_black_list);

/**
 ****************************************************************************************
 * @brief Checks the validity of the device address
 *
 * This function checks if the address type and value are available, no NULL, available
 * in the WL, not the local one.
 *
 * @param[in] bd_address           Pointer on the device address to be checked.
 * @param[in] addr_type            Type of the device address to be checked.
 *
 * @return If the device address is correct or not.
 *
 ****************************************************************************************
 */
uint8_t llm_util_check_address_validity( struct bd_addr *bd_address, uint8_t addr_type);

/**
 ****************************************************************************************
 * @brief Apply the BD address
 *
 * This function applies the BD address corresponding to the current address type on the
 * device.
 *
 * @param[in] addr_type           Type of the requested address
 *
 ****************************************************************************************
 */
void llm_util_apply_bd_addr(uint8_t addr_type);

/**
 ****************************************************************************************
 * @brief Set the public address
 *
 * @param[in]  bd_addr           Public address
 ****************************************************************************************
 */
void llm_util_set_public_addr(struct bd_addr *bd_addr);

#if (BLE_CENTRAL || BLE_OBSERVER)
/**
 ****************************************************************************************
 * @brief Check if the advertiser address has been already received.
 *
 * This function checks in case where the filtering is enabled if the address is already
 * listed.
 *
 * @param[in] adv_bd_addr         Pointer on the device address to be checked.
 * @param[in] adv_type            Type of the advertising report (ADV_IND, ADV., SCANRSP)
 *
 * @return If the device address has been found or not.
 *
 ****************************************************************************************
 */
__INLINE bool llm_util_check_adv_report_list(struct bd_addr *adv_bd_addr, uint8_t adv_type)
{
    // Get first ADV device in the list
    struct adv_device_list *adv_device = (struct adv_device_list *)common_list_pick(&llm_le_env.adv_list);
    // Returned status - Address not found by default
    bool found = false;

    while (adv_device)
    {
        // Check BD Address
        if (common_bdaddr_compare(adv_bd_addr, &adv_device->adv_addr))
        {
            // Check Address Type
            if (adv_device->adv_type == adv_type)
            {
                found = true;
                break;
            }
        }

        // Get next ADV device
        adv_device = (struct adv_device_list *)(adv_device->hdr.next);
    }

    // If the device has not been found, add it
    if (!found)
    {
        // Check number of element in the list
        if (common_list_size(&llm_le_env.adv_list) == BLE_DUPLICATE_FILTER_MAX)
        {
            // Consider the device as found
            found = true;
        }
        else
        {
            // Allocate an adv device element
            adv_device = (struct adv_device_list *)kernel_malloc(sizeof(struct adv_device_list),
                                                             KERNEL_MEM_ENV);

            // Saves the bd address in the adv filter list
            memcpy(&adv_device->adv_addr.addr[0], &adv_bd_addr->addr[0], BD_ADDR_LEN);
            // Save the type of the device address
            adv_device->adv_type = adv_type;

            // Add the element at the end of the list
            common_list_push_back(&llm_le_env.adv_list, &adv_device->hdr);
        }
    }

    return (found);
}

/**
 ****************************************************************************************
 * @brief Set the default scan parameters values
 *
 * This function set the default values for the scan parameters
 *
 ****************************************************************************************
 */
__INLINE void llm_util_set_param_scan_dft(void)
{
    // Allocate a structure containing scan parameters
    llm_le_env.scanning_params = (struct scanning_pdu_params *)kernel_malloc(sizeof(struct scanning_pdu_params),
                                                                         KERNEL_MEM_ENV);

    // Disable filtering
    llm_le_env.scanning_params->filterpolicy     = SCAN_ALLOW_ADV_ALL;
    // Set the scan interval to 10ms
    llm_le_env.scanning_params->interval         = LLM_LE_SCAN_INTERV_DFLT;
    // Set the scan interval to scan_interv/2
    llm_le_env.scanning_params->window           = LLM_LE_SCAN_INTERV_DFLT/2;
    // Set the scan mode to passive (no transmission)
    llm_le_env.scanning_params->type             = SCAN_BLE_PASSIVE;
    // Disable the duplicate filtering
    llm_le_env.scanning_params->filter_duplicate = SCAN_FILT_DUPLIC_DIS;
}
#endif //(BLE_CENTRAL || BLE_OBSERVER)

/**
****************************************************************************************
* @brief LLM function generating the Access Address for a new Link.
*
* The function will generate the following 4 bytes:
* |Byte 3           | Byte 2           |  Byte 1            | Byte 0            |
* |CT1 | PRAND_4MSB | CT2 | PRAND_4LSB | CT2 | INCRAND_4MSB | CT2 | INCRAND_4LSB|
*
* For each new generated AA,
*  - PRAND is a pseudo random 1 byte number.
*  - INCRAND is an initially generated random number, increased by 1 for each AA
*  - CT2 is toggled between the two values
*  - CT1 is again toggled between the 3 possible values
*
* Considering the formats of CT1 and CT2, the rules for AA generation are respected:
*  - since CT2 is put between bytes, and has two equal bits on each half,
*    it ensures no more than 6 consecutive bits inside the address. CT1 also.
*
*  - CT2 and CT1 ensure that the newly generated address is different and at least 1 bit
*    different from the advertising access address
*
*  - since CT1 != CT2 , the four bytes of the AA can never be equal
*
*  - Since in a 32 bit number there are 31 transitions, and the neighboring equal bits
*    in CT1 and CT2 represent 7 transitions, there can never be more than 31-7=24
*    transitions in the AA
*
*  - The format of CT1 ensures at least two transitions in the most significant 6 bits.
*
* @param acc_addr Pointer to the 4 byte array space for the AA.
*
*
*****************************************************************************************
*/
__INLINE void llm_util_aa_gen(uint8_t *acc_addr)
{
    //the pseudo random byte is generated every time - use a seed?
    uint8_t prand = common_rand_byte();

    //Create the AA - MSB to LSB

    acc_addr[0] = (LLM_AA_CT2[llm_le_env.aa.ct2_idx] << 4 ) | (llm_le_env.aa.intrand & 0x0F);
    acc_addr[1] = (LLM_AA_CT2[llm_le_env.aa.ct2_idx] << 4 ) | (llm_le_env.aa.intrand >> 4);
    acc_addr[2] = (LLM_AA_CT2[llm_le_env.aa.ct2_idx] << 4 ) | (prand & 0x0F);
    acc_addr[3] = (LLM_AA_CT1[llm_le_env.aa.ct1_idx] << 4 ) | (prand >> 4);

    llm_le_env.aa.ct1_idx = (llm_le_env.aa.ct1_idx + 1) % 3;

    //Increase random
    llm_le_env.aa.intrand ++;

}

/**
 ****************************************************************************************
 * @brief Fulfills the payload for the transmit test mode.
 *
 * This function fulfills the payload for the transmit test mode.
 *
 * @param[in] pattern_type         type of the pattern.
 * @param[in] payload_len          length of the payload.
 *
 ****************************************************************************************
 */
__INLINE void llm_util_gen_pattern(uint8_t pattern_type, uint8_t payload_len ,
        uint8_t *payload)
{
    uint8_t pattern = 0;
    // get the pattern
    switch(pattern_type)
    {
        case PAYL_11110000:
            pattern = 0xF0;
            break;
        case PAYL_10101010:
            pattern = 0xAA;
            break;
        case PAYL_ALL_1:
            pattern = 0xFF;
            break;
        case PAYL_ALL_0:
            pattern = 0x00;
            break;
        case PAYL_00001111:
            pattern = 0x0F;
            break;
        case PAYL_01010101:
            pattern = 0x55;
            break;
        default:
            ASSERT_ERR(pattern_type < PAYL_END);
            break;
    }
    // fulfill the payload
    memset(payload, pattern, payload_len);

}

/**
 ****************************************************************************************
 * @brief Set the default advertising parameters values
 *
 * This function set the default values for the advertising parameters
 *
 ****************************************************************************************
 */
#if (BLE_PERIPHERAL || BLE_BROADCASTER)
__INLINE void llm_util_set_param_adv_dft(void)
{
    // Allocate a structure containing scan parameters
    llm_le_env.advertising_params = (struct advertising_pdu_params *)kernel_malloc(sizeof(struct advertising_pdu_params),
                                                                               KERNEL_MEM_ENV);
    memset(llm_le_env.advertising_params, 0, sizeof(struct advertising_pdu_params));

    // Disable filtering
    llm_le_env.advertising_params->filterpolicy = ADV_ALLOW_SCAN_ANY_CON_ANY;
    // Set the advertising type to connectable undirected
    llm_le_env.advertising_params->type         = ADV_CONN_UNDIR;
    // Set the interval max to 1.28s
    llm_le_env.advertising_params->intervalmax  = LLM_LE_ADV_INTERV_DFLT;
    // Set the interval min to 1.28s
    llm_le_env.advertising_params->intervalmin  = LLM_LE_ADV_INTERV_DFLT;
    // Set all adv channels available
    llm_le_env.advertising_params->channelmap   = LLM_LE_ADV_CH_MAP_DFLT;
    // use low duty cycle ADV
    llm_le_env.advertising_params->adv_ldc_flag  = true;


    // Sets the type of advertising, the length(only AdvA len = 6 bytes)
    ble_txphadv_pack(LLM_LE_ADV_IDX,                // index for the ADVERTISING PDU descriptor
            6,                                      // Length of the connect req
            0,                                      // peer address type
            0,                                      // updated by HW
            llm_le_env.advertising_params->type );  // type of PDU

    // Sets the scan response
    ble_txphadv_pack(LLM_LE_SCAN_RSP_IDX,   // index for the SCAN RESPONSE PDU descriptor
            6,                              // Length of the connect req
            0,                              // peer address type
            0,                              // updated by HW
            LL_SCAN_RSP );                  // type of PDU
}
#endif //(BLE_PERIPHERAL || BLE_BROADCASTER)

/**
 ****************************************************************************************
 * @brief Chek the status of the test mode
 *
 * This function sends an event to the host when the TX or RX test mode is finished
 *
 ****************************************************************************************
 */
__INLINE void llm_util_chk_tst_mode(void)
{
    //if the current state is not IDLE
    if((llm_le_env.test_mode.directtesttype != TEST_END) && (llm_le_env.test_mode.end_of_tst == true))
    {
        // structure type for the complete command event
        struct hci_test_end_cmd_cmp_evt *event;

        // allocate the complete event message
        event = KERNEL_MSG_ALLOC(HCI_CMD_CMP_EVENT, 0, HCI_LE_TEST_END_CMD_OPCODE, hci_test_end_cmd_cmp_evt);

        llm_le_env.test_mode.end_of_tst = false;
        // enable the whitening
        ble_whit_dsb_setf(0);

        // Number_Of_Packets for a transmitter test is reported as NULL.
        if (llm_le_env.test_mode.directtesttype == TEST_TX)
        {
            event->nb_packet_received = 0;
        }
        else
        {
            event->nb_packet_received = ble_rxccmpktcnt0_get(LLD_ADV_HDL);
        }
        // set the env variable,
        llm_le_env.test_mode.directtesttype = TEST_END;
        // update the status
        event->status = COMMON_ERROR_NO_ERROR;
        // send the message
        hci_send_2_host(event);
        kernel_msg_send_basic(LLM_STOP_IND, TASK_LLM, TASK_LLM);
    }
}

#if (BLE_BROADCASTER || BLE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief Get Advertising type
 *
 * @return The type of the advertising: Low or High Duty Cycle
 *
 ****************************************************************************************
 */
__INLINE uint8_t llm_util_get_adv_type(void)
{
    if(llm_le_env.advertising_params->adv_ldc_flag)
    {
        return (LLD_LD_ADVERTISER);
    }
    else
    {
        return (LLD_HD_ADVERTISER);
    }
}
#endif
#if ((BLE_CENTRAL || BLE_PERIPHERAL) && (RW_DEBUG))
/**
 ****************************************************************************************
 * @brief Gets the size of the packet received.
 *
 * This function gets the maximum length of the packet to be transmitted.
 *
 *
 * @return The length of the data.
 *
 ****************************************************************************************
 */
__INLINE uint16_t llm_util_dle_get_tx_max_size(void)
{
      return (llm_le_env.data_len_val.suppted_max_tx_octets);
}
#endif

/**
 ****************************************************************************************
 * @brief Check the event mask
 *
 * @param[in]     event_id     event to check
 *
 * @return     True: event can be sent / False: event is masked
 ****************************************************************************************
 */
bool llm_util_check_evt_mask(uint8_t event_id);

/**
 ****************************************************************************************
 * @brief Get the channel map
 *
 * @param[out] map    Pointer to channel map buffer
 ****************************************************************************************
 */
void llm_util_get_channel_map(struct le_chnl_map *map);

/**
 ****************************************************************************************
 * @brief Get supported features
 *
 * @param[out] feats    Pointer to supported features buffer
 ****************************************************************************************
 */
void llm_util_get_supp_features(struct le_features *feats);

#if (BLE_BROADCASTER || BLE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief Check if advertising data need to be updated and update it if yes
 *
 ****************************************************************************************
 */
void llm_util_adv_data_update(void);
#endif//(BLE_BROADCASTER || BLE_PERIPHERAL)

#if (BLE_CENTRAL || BLE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief Checks if the device address is already in the black list.
 *
 * This function checks if the bd address if present in the connected list and return
 * the status.
 *
 * @param[in]  bd_address_to_add  Pointer on the device address to be checked.
 * @param[in]  bd_addr_type       Type of BD Address
 * @param[out] conhdl             Pointer on the Connection handle.
 * @param[in]  wl_flag_action     set/clear the flag.
 * @param[out] in_wl              Retrieve if device present in WL
 *
 * @return If the device address has been found or not.
 *
 ****************************************************************************************
 */
uint8_t llm_util_bl_check(const struct bd_addr *bd_addr_to_add, uint8_t bd_addr_type, uint16_t *conhdl, uint8_t wl_flag_action, bool* in_wl);
/**
 ****************************************************************************************
 * @brief Add the device address in the BLACK list.
 *
 * This function checks if the bd address if present in the connected list and return
 * the status.
 *
 * @param[in] bd_address_to_add  Pointer on the device address to be added.
 * @param[in] bd_addr_type       address type (public or random) on the device address to be added.
 * @param[in] conhdl             Connection handle linked to the bd address.

 *
 * @return If the device address has been added or not.
 *
 ****************************************************************************************
 */
uint8_t llm_util_bl_add(struct bd_addr *bd_addr_to_add, uint8_t bd_addr_type, uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Remove the bd address if already connected.
 *
 * This function checks if the bd address if present in the connected list, extract the
 * bd address from the list and free the memory.
 *
 * @param[in] conhdl  connection handle linked to the bd address to be removed.
 *
 * @return If the bd address has been removed or not.
 *
 ****************************************************************************************
 */
uint8_t llm_util_bl_rem(uint16_t conhdl);

#if (BLE_2MBPS)
/**
 ****************************************************************************************
 * @brief Get the default Phys values.
 *
 * This function gets the values of the default Phys set over the command hci_set_dft_phy
 *
 * @param[out] tx_phy            Retrieve phy used for TX
 * @param[out] rx_phy            Retrieve phy used for RX
 *
 ****************************************************************************************
 */
void llm_util_get_default_phy(uint8_t *tx_phy, uint8_t *rx_phy);
#endif

#endif // (BLE_CENTRAL || BLE_PERIPHERAL)

/**
 ****************************************************************************************
 * @brief check if the bd address if present in the RAL and return the status and position
 *
 * If not present, this function provide first available element in the list
 ****************************************************************************************
 */
bool llm_util_bd_addr_in_ral(struct bd_addr const *bd_address, uint8_t bd_addr_type, uint8_t *position);



/**
 ****************************************************************************************
 * @brief Handling defer of End of event
 *
 ****************************************************************************************
 */
void llm_end_evt_defer(void);

/**
 ****************************************************************************************
 * @brief Handling of pdu information in a deferred event
 *
 * @param[in] conhdl   Connection handle
 * @param[in] rx_hdl   RX Descriptor handle
 * @param[in] tx_cnt   Number of buffer acknowledged
 *
 * @param[out] elt_deleted   Return true if the element has been delested (scan evt for init)
 ****************************************************************************************
 */
bool llm_pdu_defer(uint16_t status, uint8_t rx_hdl, uint8_t tx_cnt);
/// @} LLM

#endif // LLM_UTIL_H_
