/**
 ****************************************************************************************
 *
 * @file kernel_timer.h
 *
 * @brief This file contains the definitions used for timer management
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _KERNEL_TIMER_H_
#define _KERNEL_TIMER_H_

/**
 ****************************************************************************************
 * @defgroup TIMER BT Time
 * @ingroup KERNEL
 * @brief Timer management module.
 *
 * This module implements the functions used for managing kernel timers.
 *
 ****************************************************************************************
 */

#include "kernel_msg.h"               // messaging definition
#include "rwip_config.h"          // stack configuration


/*
 * DEFINITIONS
 ****************************************************************************************
 */

///  Convert timer from second to timer accuracy (10ms)
#define KERNEL_TIME_IN_SEC(_time) (_time * 100)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Timer Object
struct kernel_timer
{
    /// next ke timer
    struct kernel_timer *next;
    /// message identifier
    kernel_msg_id_t     id;
    /// task identifier
    kernel_task_id_t    task;
    /// time value
    uint32_t        time;
};


/*
 * FUNCTION PROTOTYPES
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Initialize Kernel timer module.
 ****************************************************************************************
 */
void kernel_timer_init(void);

/**
 ****************************************************************************************
 * @brief Set a timer.
 *
 * The function first cancel the timer if it is already existing, then
 * it creates a new one. The timer can be one-shot or periodic, i.e. it
 * will be automatically set again after each trigger.
 *
 * When the timer expires, a message is sent to the task provided as
 * argument, with the timer id as message id.
 *
 * The timer is programmed in time units (TU is 10ms).
 *
 * @param[in] timer_id      Timer identifier (message identifier type).
 * @param[in] task_id       Task identifier which will be notified
 * @param[in] delay         Delay in time units.
 ****************************************************************************************
 */
void kernel_timer_set(kernel_msg_id_t const timer_id, kernel_task_id_t const task, uint32_t delay);

/**
 ****************************************************************************************
 * @brief Remove an registered timer.
 *
 * This function search for the timer identified by its id and its task id.
 * If found it is stopped and freed, otherwise an error message is returned.
 *
 * @param[in] timer_id  Timer identifier.
 * @param[in] task      Task identifier.
 ****************************************************************************************
 */
void kernel_timer_clear(kernel_msg_id_t const timerid, kernel_task_id_t const task);

/**
 ****************************************************************************************
 * @brief Checks if a requested timer is active.
 *
 * This function pops the first timer from the timer queue and notifies the appropriate
 * task by sending a kernel message. If the timer is periodic, it is set again;
 * if it is one-shot, the timer is freed. The function checks also the next timers
 * and process them if they have expired or are about to expire.
 ****************************************************************************************
 */
bool kernel_timer_active(kernel_msg_id_t const timer_id, kernel_task_id_t const task_id);

/**
 ****************************************************************************************
 * @brief Adjust all kernel timers by specified adjustment delay.
 *
 * This function updates all timers to align to a new SCLK after a system clock adjust.
 ****************************************************************************************
 */
void kernel_timer_adjust_all(uint32_t delay);


#if (DEEP_SLEEP)
/**
 ****************************************************************************************
 * @brief Get the first timer target (in Slot) used for deep sleep decision
 *
 * @return Invalid time if nothing programmed; target time else.
 ****************************************************************************************
 */
bool kernel_timer_sleep_check(uint32_t *sleep_duration, uint32_t wakeup_delay);
uint32_t kernel_timer_target_get(void);

#endif //DEEP_SLEEP

/// @} TIMER

#endif // _KERNEL_TIMER_H_
