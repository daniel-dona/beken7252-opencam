/**
 ****************************************************************************************
 *
 * @file kernel_event.h
 *
 * @brief This file contains the definition related to kernel events.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _KERNEL_EVENT_H_
#define _KERNEL_EVENT_H_

/**
 ****************************************************************************************
 * @addtogroup EVT Events and Schedule
 * @ingroup KERNEL
 * @brief Event scheduling module.
 *
 * The KERNEL_EVT module implements event scheduling functions. It can be used to
 * implement deferred actions.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"          // stack configuration

#include <stdint.h>       // standard integer definition


/*
 * CONSTANTS
 ****************************************************************************************
 */


/// Status of kernel_task API functions
enum KERNEL_EVENT_STATUS
{
    KERNEL_EVENT_OK = 0,
    KERNEL_EVENT_FAIL,
    KERNEL_EVENT_UNKNOWN,
    KERNEL_EVENT_CAPA_EXCEEDED,
    KERNEL_EVENT_ALREADY_EXISTS,
};


/*
 * TYPE DEFINITION
 ****************************************************************************************
 */




/*
 * FUNCTION PROTOTYPES
 ****************************************************************************************
 */



/**
 ****************************************************************************************
 * @brief Initialize Kernel event module.
 ****************************************************************************************
 */
void kernel_event_init(void);

/**
 ****************************************************************************************
 * @brief Register an event callback.
 *
 * @param[in]  event_type       Event type.
 * @param[in]  p_callback       Pointer to callback function.
 *
 * @return                      Status
 ****************************************************************************************
 */
uint8_t kernel_event_callback_set(uint8_t event_type, void (*p_callback)(void));

/**
 ****************************************************************************************
 * @brief Set an event
 *
 * This primitive sets one event. It will trigger the call to the corresponding event
 * handler in the next scheduling call.
 *
 * @param[in]  event_type      Event to be set.
 ****************************************************************************************
 */
void kernel_event_set(uint8_t event_type);

/**
 ****************************************************************************************
 * @brief Clear an event
 *
 * @param[in]  event_type      Event to be cleared.
 ****************************************************************************************
 */
void kernel_event_clear(uint8_t event_type);

/**
 ****************************************************************************************
 * @brief Get the status of an event
 *
 * @param[in]  event_type      Event to get.
 *
 * @return                     Event status (0: not set / 1: set)
 ****************************************************************************************
 */
uint8_t kernel_event_get(uint8_t event_type);

/**
 ****************************************************************************************
 * @brief Get all event status
 *
 * @return                     Events bit field
 ****************************************************************************************
 */
uint32_t kernel_event_get_all(void);

/**
 ****************************************************************************************
 * @brief Flush all pending events.
 ****************************************************************************************
 */
void kernel_event_flush(void);

/**
 ****************************************************************************************
 * @brief Event scheduler entry point.
 *
 * This primitive is the entry point of Kernel event scheduling.
 ****************************************************************************************
 */
void kernel_event_schedule(void);



/// @} EVT

#endif //_KERNEL_EVENT_H_
