/**
****************************************************************************************
*
* @file EA.h
*
* @brief EA main module
*
* Copyright (C) RivieraWaves 2009-2015
*
*
****************************************************************************************
*/

#ifndef _EA_H_
#define _EA_H_

/**
 ****************************************************************************************
 * @addtogroup EA
 * @brief Entry points of the Event Arbiter module
 *
 * This module contains the primitives that allow stacks to schedule an event or frame.
 *
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (EA_PRESENT)

#include <stdint.h>               // Standard integer definitions
#include <stdbool.h>              // Standard boolean definitions
#include "common_list.h"              // List management functions definitions


#if (BT_EMB_PRESENT)
#define EA_ALARM_SUPPORT      1
#else
#define EA_ALARM_SUPPORT      0
#endif //(BT_EMB_PRESENT)

/*
 * MACROS
 ****************************************************************************************
 */



/*
 * DEFINES
 ****************************************************************************************
 */

/// Default BW 2 slots
#define EA_BW_USED_DFT_SLOT         (2)
#define EA_BW_USED_DFT_US           (EA_BW_USED_DFT_SLOT*SLOT_SIZE)
/// Clock correction delay
#define EA_CLOCK_CORR_LAT           (1)

/// Set ASAP settings
#define EA_ASAP_STG_SET(evt, type, parity, to_protect, resched_att, prio_inc)  evt->asap_settings = ( (((type) << 14) & 0xC000) | (((parity) << 12) & 0x3000) | (((to_protect) << 11) & 0x0800) | (((resched_att) << 4) & 0x03F0) | (((prio_inc) << 0) & 0x000F) );
/// Get/Set type from ASAP settings
#define EA_ASAP_STG_TYPE_GET(evt)                      ((uint8_t)((evt->asap_settings & 0xC000) >> 14))
#define EA_ASAP_STG_TYPE_SET(evt, type)                ( evt->asap_settings = ((evt->asap_settings & ~(0xC000)) | (((type) << 14) & 0xC000)) )
/// Get parity from ASAP settings
#define EA_ASAP_STG_PARITY_GET(evt)                    ((uint8_t)((evt->asap_settings & 0x3000) >> 12))
/// Get to protect from ASAP settings
#define EA_ASAP_STG_TO_PROTECT_GET(evt)                    ((uint8_t)((evt->asap_settings & 0x0800) >> 11))
/// Get/Set rescheduling attempts from ASAP settings
#define EA_ASAP_STG_RESCHED_ATT_GET(evt)               ((uint8_t)((evt->asap_settings & 0x03F0) >> 4))
#define EA_ASAP_STG_RESCHED_ATT_SET(evt, resched_att)  ( evt->asap_settings = ((evt->asap_settings & ~(0x03F0)) | (((resched_att) << 4) & 0x03F0)) )
/// Get priority increment from ASAP settings
#define EA_ASAP_STG_PRIO_INC_GET(evt)                  ((uint8_t)((evt->asap_settings & 0x000F) >> 0))


/*
 * DEFINITIONS
 ****************************************************************************************
 */

/// EA error codes
enum ea_error
{
    /// EA request succeed
    EA_ERROR_OK                 = 0,
    /// EA request rejected error
    EA_ERROR_REJECTED,
    /// EA element not found error
    EA_ERROR_NOT_FOUND,
    /// EA request rejected due to bandwidth full error
    EA_ERROR_BW_FULL
};

/// Action for the parameters request API
enum ea_param_req_action
{
    EA_PARAM_REQ_GET,
    EA_PARAM_REQ_CHECK,
};

/// ASAP type definition
enum ea_elt_asap_type
{
    /// 00: No ASAP
    EA_FLAG_NO_ASAP                = 0,
    /// 01: ASAP no limit
    EA_FLAG_ASAP_NO_LIMIT,
    /// 10: ASAP with limit
    EA_FLAG_ASAP_LIMIT,
    EA_FLAG_MAX
};

/// ASAP slot parity definition
enum ea_elt_asap_parity
{
    EA_EVEN_SLOT,
    EA_ODD_SLOT,
    EA_NO_PARITY,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */



/// Event Arbiter Element
struct ea_elt_tag
{
    /// List element for chaining in the Even Arbiter lists
    struct common_list_hdr hdr;

    /// Pointer on the next element linked to the current action
    struct ea_elt_tag *linked_element;

    /// Programming time in basetime (CLOCKN) absolute value
    uint32_t timestamp;

    /// Scheduling time limit in base time (CLOCKN) (in slots) (only for ASAP LIMIT requests)
    uint32_t asap_limit;

    /**
     * ASAP settings field
     * bit |15  14| 13  12 |     11     | 10|    9..4     |   3..0   |
     * def | TYPE | Parity | To protect |RFU| Resched att | Prio inc |
     *
     * Type:
     *  - 00: No ASAP
     *  - 01: ASAP no limit
     *  - 10: ASAP with limit
     *  - 11: ASAP with limit, no parity check
     *
     * Parity: (only for ASAP requests)
     *  - 0: even slots
     *  - 1: odd slots
     *  - 2: don't care
     *
     * To protect:
     *  - 0: Event does NOT need to be protected by the bigger stop notification delay
     *  - 1: Event needs to be protected by the bigger stop notification delay (when priority is higher)
     *
     * Number of rescheduling attempts:
     *  - The remaining number of rescheduling attempts.
     *  - Rescheduling happens when the event is overlapped by a higher priority event
     *  - Only used for ASAP requests
     *
     * Priority increment:
     *  - The current priority value is incremented each time the event is overlapped by a new insertion and postponed
     *  - Only used for ASAP requests
     */
    uint16_t asap_settings;

    /// Minimum duration of the event or frame (in us)
    uint16_t duration_min;

    /// Delay of the start of the activity from the local slot boundary (in us)
    uint16_t delay;

    /// Current priority
    uint8_t current_prio;
    /// Latency to stop the activity (when next activity is lower priority)
    uint8_t stop_latency1;
    /**
     * Latency to stop the activity (when next activity is higher priority, and next activity needs protection)
     * Note: threshold 2 is always higher than threshold 1, so that any following higher priority activity is protected
     *  from being overlapped by this activity
     */
    uint8_t stop_latency2;
    /// Latency to start the activity
    uint8_t start_latency;

    /************************************************************************************
     * ISR CALLBACKS
     ************************************************************************************/

    /// Start notification call back function
    void (*ea_cb_start)(struct ea_elt_tag*);
    /// Stop notification call back function
    void (*ea_cb_stop)(struct ea_elt_tag*);
    /// Cancel notification call back function
    void (*ea_cb_cancel)(struct ea_elt_tag*);

    /// BT/BLE Specific environment variable
    void *env;
};

/// Interval element strcuture
struct ea_interval_tag
{
    /// List element for chaining in the Interval list
    struct common_list_hdr hdr;
    /// Interval used
    uint16_t interval_used;
    /// Offset used
    uint16_t offset_used;
    /// Bandwidth used
    uint16_t bandwidth_used;
    /// Connection handle used
    uint16_t conhdl_used;
    /// Role used
    uint16_t role_used;
    /// Odd offset or even offset
    bool odd_offset;
    /// Link id
    uint16_t linkid;
};

/// API get/check parameters input
struct ea_param_input
{
    /// Interval minimum requested
    uint16_t interval_min;
    /// Interval maximum requested
    uint16_t interval_max;
    /// Duration minimum requested
    uint32_t duration_min;
    /// Duration maximum requested
    uint16_t duration_max;
    /// Preferred periodicity
    uint8_t pref_period;
    /// Offset requested
    uint16_t offset;
    /// action
    uint8_t action;
    /// Connection handle
    uint16_t conhdl;
    /// Role
    uint16_t role;
    /// Odd offset or even offset
    bool odd_offset;
    /// Link id
    uint16_t linkid;
};

/// API get/check parameters output
struct ea_param_output
{
    /// Interval returned
    uint16_t interval;
    /// Duration returned
    uint32_t duration;
    /// Offset returned
    uint16_t offset;
};

#if (EA_ALARM_SUPPORT)
/// Alarm element structure
struct ea_alarm_tag
{
    /// List element for chaining in the Even Arbiter lists
    struct common_list_hdr hdr;

    /// Timestamp of alarm expiry
    uint32_t timestamp;

    /// Call back function invoked upon alarm expiry
    void (*ea_cb_alarm)(struct ea_alarm_tag*);
};
#endif //(EA_ALARM_SUPPORT)

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
 * @brief Initialize the Event Arbiter.
 ****************************************************************************************
 */
void ea_init(bool reset);

/**
 ****************************************************************************************
 * @brief Return the current time half slot boundary rounded
 *
 * @return Current time (in BT slots)
 ****************************************************************************************
 */
uint32_t ea_time_get_halfslot_rounded(void);

/**
 ****************************************************************************************
 * @brief Return the current time slot boundary rounded
 *
 * @return Current time (in BT slots)
 ****************************************************************************************
 */
uint32_t ea_time_get_slot_rounded(void);

/**
 ****************************************************************************************
 * @brief Create a new event element
 *
 * @param[in] size_of_env          Size of the dedicated environment structure
 *
 * @return Pointer on the allocated element
 ****************************************************************************************
 */
struct ea_elt_tag *ea_elt_create(uint16_t size_of_env);

/**
 ****************************************************************************************
 * @brief API to try to insert a new element in the scheduler queue
 *
 * @param[in] elt           Pointer to the element to be inserted
 *
 * @return No error if element is inserted.
 ****************************************************************************************
 */
uint8_t ea_elt_insert(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief API to try to remove the current element programmed
 *
 * @param[in] elt           Pointer to the element to be removed from current ptr
 *
 ****************************************************************************************
 */
uint8_t ea_elt_remove(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Start of event/frame interrupt handler
 *
 * This function is called under interrupt when a start of event/frame interrupt is
 * generated by the BLE/BT core.
 *
 ****************************************************************************************
 */
void ea_finetimer_isr(void);

/**
 ****************************************************************************************
 * @brief Software interrupt handler
 *
 * This function is called under interrupt when a SW interrupt is generated by the BLE/BT
 * core.
 *
 ****************************************************************************************
 */
void ea_sw_isr(void);

/**
 ****************************************************************************************
 * @brief Create a new interval element
 *
 * @return Pointer on the allocated element
 ****************************************************************************************
 */
struct ea_interval_tag *ea_interval_create(void);

/**
 ****************************************************************************************
 * @brief API to try to remove an interval from the queue
 *
 * @param[in] interval_to_remove     Interval to be removed
 *
 ****************************************************************************************
 */
void ea_interval_delete(struct ea_interval_tag *interval_to_remove);

/**
 ****************************************************************************************
 * @brief Insert the interval in the common interval queue
 ****************************************************************************************
 */
void ea_interval_insert(struct ea_interval_tag *interval_to_add);

/**
 ****************************************************************************************
 * @brief Remove the interval from the common interval queue
 ****************************************************************************************
 */
void ea_interval_remove(struct ea_interval_tag *interval_to_remove);


/**
 ****************************************************************************************
 * @brief Get the first timer target (in Slot) used for deep sleep decision
 *
 * @param[in] current_time Current time value used in case of HW activity programmed
 *
 * @return Invalid time if nothing programmed; target time else.
 ****************************************************************************************
 */
 
 uint32_t ea_timer_target_get(uint32_t current_time);

bool ea_sleep_check(uint32_t *sleep_duration, uint32_t wakeup_delay);

/**
 ****************************************************************************************
 * @brief Choose an appropriate offset according to the interval
 *
 * @param[in] input_param  parameters used to compute the offset
 *
 * @param[in] output_param offset used for the interval
 *
 * @return true if an offset has been found or not
 ****************************************************************************************
 */
uint8_t ea_offset_req(struct ea_param_input* input_param, struct ea_param_output* output_param);
/**
 ****************************************************************************************
 * @brief Choose an appropriate interval and duration for the event
 *
 * @param[in] input_param  parameter used to compute the interval and duration of the event
 *
 * @param[in] output_param interval and duration chosen for the event
 *
 ****************************************************************************************
 */
void ea_interval_duration_req(struct ea_param_input* input_param, struct ea_param_output* output_param);

#if (EA_ALARM_SUPPORT)
/**
 ****************************************************************************************
 * @brief Set an alarm
 *
 * @param[in] elt      Pointer to the alarm element to be programmed
 *
 * @return 0 - no error / 1:255 - error
 ****************************************************************************************
 */
uint8_t ea_alarm_set(struct ea_alarm_tag* elt);

/**
 ****************************************************************************************
 * @brief Clear an alarm
 *
 * @param[in] elt      Pointer to the alarm element to be cleared
 *
 * @return 0 - no error / 1:255 - error
 ****************************************************************************************
 */
uint8_t ea_alarm_clear(struct ea_alarm_tag* elt);
#endif //(EA_ALARM_SUPPORT)

//struct ea_elt_tag *ea_elt_get(void);
#endif //(EA_PRESENT)

///@} EA

#endif // _EA_H_
